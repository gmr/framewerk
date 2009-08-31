<?php // $Id: fIP2CountryDatabase.inc 919 2007-08-06 18:34:41Z dallas $
############################################################################
#    Copyright (C) 2005-2007 by the Framewerk Development Group            #
#    info@framewerk.org                                                    #
#                                                                          #
#    Permission is hereby granted, free of charge, to any person obtaining #
#    a copy of this software and associated documentation files (the       #
#    "Software"), to deal in the Software without restriction, including   #
#    without limitation the rights to use, copy, modify, merge, publish,   #
#    distribute, sublicense, and#or sell copies of the Software, and to    #
#    permit persons to whom the Software is furnished to do so, subject to #
#    the following conditions:                                             #
#                                                                          #
#    The above copyright notice and this permission notice shall be        #
#    included in all copies or substantial portions of the Software.       #
#                                                                          #
#    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       #
#    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    #
#    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.#
#    IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR     #
#    OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, #
#    ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR #
#    OTHER DEALINGS IN THE SOFTWARE.                                       #
############################################################################

/**
 * IP Address to Country Lookup Client Object.
 *
 * This function uses a Database table to lookup  country information
 * The original CSV Function is available at http://ip-to-country.webhosting.info
 * Load the data table via the commandline utilities in Utilities
 *
 * @author     Gavin M. Roy <gavinmroy@gmail.com>
 * @link       http://framewerk.org
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2005-2007 the Framewerk Development Group
 * @version    $Rev: 919 $
 * @package    Engine
 * @subpackage Core
 */

class fIP2CountryDatabase
{
  protected $fMain;

  /**
   * Constructor
   *
   * fIP2CountryLookup Constructor
   *
   * @param fIP2Country Object
   */
  function __construct()
  {
    $this->fMain = fMain::getInstance();
  }

  /**
   * GetCountry
   *
   * Get the Country Code for the Current IP Address via a database lookup
   *
   * @param string Return Type
   * @return string Return Data
   */
  function GetCountry($type = '3')
  {
    $address = sprintf("%u",ip2long($_SERVER['REMOTE_ADDR']));

    $query = $this->fMain->PDO->prepare("SELECT codetwo, codethree, name FROM iptocountry WHERE rangestart <= :ipAddressStart AND rangeend >= :ipAddressEnd;");
    if ( !is_object($query) ) throw new fHTTPException(501, 'IP2Country table not found.  Please run IP2Country installation script or turn off IP2Country in your configuration.');
    $query->bindParam(":ipAddressStart", $address);
    $query->bindParam(":ipAddressEnd", $address);
    $query->execute();
    if ( $query->rowCount() == 0 )
    {
      $data[0] = "UN";
      $data[1] = "UNK";
      $data[2] = "UNKNOWN";
    } else {
      $data = $query->Fetch(PDO::FETCH_ASSOC);
      unset($query);
    }

    switch ($type)
    {
      case 'name':
        return $data['name'];
      case '2':
        return $data['codetwo'];
      case '3':
      default:
        return $data['codethree'];
    }
  }
  
}
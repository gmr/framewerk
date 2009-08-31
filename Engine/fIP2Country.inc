<?php // $Id: fIP2Country.inc 837 2007-03-29 02:55:08Z gmr $
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
  * IP Address to Country Lookup Parent Object
  *
  * This object is responsible for defining how to use IP-To-Country lookups
  *
 * @author     Gavin M. Roy <gavinmroy@gmail.com>
 * @link       http://framewerk.org
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2005-2007 the Framewerk Development Group
 * @version    $Rev: 837 $
 * @package    Engine
 * @subpackage Core
 */

class fIP2Country
{
  protected $config;
  private   $country;
  private   $module;
  private   $enabled;

  /**
   * Constructor
   *
   * Object Constructor for fIP2Country
   *
   * @param fMain parent object
   */
  function __construct()
  {
    $this->config = fMain::getInstance()->configuration->session->ip2country;
    $this->country = "unk";
    $this->enabled = false;
    $classname = "fIP2Country" . strval($this->config->module);
    $this->module = new $classname($parent);
    $this->enabled = true;
  }

  /**
   * GetCountry
   *
   * Lookup the Country Code via the child object
   *
   * @param string Code type, name, 2, 3
   * @return string Country Result
   */
  function GetCountry($code = "2")
  {
    if ( $this->enabled == true )
    {
      return $this->module->GetCountry($code);
    } else {
      return "Unknown";
    }
  }

  function __destruct()
  {
    unset($this->module);
  }

}
<?php // $Id: fTrafficLog.inc 919 2007-08-06 18:34:41Z dallas $
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
 * fTrafficLog
 *
 * Contains the Traffic Logging class.
 */
/**
 * Traffic logging object
 *
 * @author     Gavin M. Roy <gavinmroy@gmail.com>
 * @link       http://framewerk.org
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2005-2007 the Framewerk Development Group
 * @version    $Rev: 919 $
 * @package    Engine
 * @subpackage Core
 * @uses       fMainObject
 * @since      2005-09-09
 */

class fTrafficLog extends fObject
{

  function __construct()
  {
    $this->fMain = fMain::getInstance();

    if ( $this->fMain->configuration->site->logTraffic['enabled'] == false )
    {
      return;
    }

    foreach ( $this->fMain->configuration->site->logTraffic->ignore AS $object )
    {
      if ( strval($object) == $this->fMain->lastInterface )
      {
        return;
      }
    }

    $query = $this->fMain->PDO->prepare('INSERT INTO traffic VALUES ( now(), :ipaddress, :url, :referrer, :agent );');

    $query->bindParam(':ipaddress', $_SERVER['REMOTE_ADDR']);
    $url = $this->fMain->lastInterface . ': ' . $_SERVER['REQUEST_URI'];
    $query->bindParam(':url', $url);
    $query->bindParam(':referrer', $_SERVER['HTTP_REFERER']);
    $query->bindParam(':agent', $_SERVER['HTTP_USER_AGENT']);
    $query->execute();
  }
}
?>

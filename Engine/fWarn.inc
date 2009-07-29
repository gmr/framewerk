<?php // $Id: fWarn.inc 8486 2007-12-20 15:22:22Z jhansche $
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
 * Engine Warning Information Object
 *
 * This object stores and outputs warning information
 *
 * @author     Gavin M. Roy <gavinmroy@gmail.com>
 * @link       http://framewerk.org
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2005-2007 the Framewerk Development Group
 * @version    $Rev: 8486 $
 * @package    Engine
 * @subpackage Messages
 * @uses       fEngineInformation
 */

class fWarn extends fEngineInformation
{
  protected static $instance = NULL;
  private $enabled = false;

  private $syslog, $logSeverity;

  /**
   * Constructor
   * Loads registry setting to see if Warnings are enabled
   */
  protected function __construct()
  {
    $registry = new fRegistry('fWarn');
    $this->enabled = $registry->enabled;

    $this->outputMethod = $registry->outputMethod;
    if ( $this->outputMethod == 'syslog' )
    {
      $this->logSeverity = $registry->logSeverity;
      $this->syslog = fSyslog::getInstance();
    }
  }

  function add($message)
  {
    switch ( $this->outputMethod )
    {
      case 'syslog':
        $dbg = debug_backtrace();
        $source = $dbg[1]['class'] . '->' . $dbg[1]['function'] . '()';
        $this->syslog->logMessage(sprintf('%s: [Line %d]: %s', $source, $dbg[0]['line'], $message), $this->logSeverity);
        unset($dbg);
        break;
      default:
        break;
    }
    parent::add($message);
  }

  /**
   * Get the current instance and create it if it's not there
   *
   * @return fWarn instance
   */
  static function getInstance()
  {
    if ( !isset(self::$instance) )
    {
      self::$instance = new fWarn();
    }
    return self::$instance;
  }

  /**
   * Output warning messages
   *
   * @return String Warning output
   */
  function output()
  {
    if ( $this->outputMethod != '' && $this->outputMethod != 'browser' )
    {
      return NULL;
    }
    $this->done = true;
    if ( !count($this->messages) || $this->enabled != true ) return;

    $dom = new DOMDocument('1.0', 'UTF-8');
    $messages = $dom->createElement('warnings');

    foreach ( $this->messages as $message)
    {
      $xml = $dom->createElement('warning');
      $xml->appendChild($dom->createElement('timestamp', $message['timestamp']));
      $xml->appendChild($dom->createElement('value', $message['value']));
      $messages->appendChild($xml);
    }
    $dom->appendChild($messages);

    $theme = fTheme::getInstance();
    return $theme->transformXML($theme->xslFile('fEngineInformation'), $dom);
  }

  function outputArray()
  {
    $this->done = true;
    if ( !count($this->messages) || $this->enabled != true )
    {
      return array();
    }

    $return = array();

    foreach ( $this->messages as $message )
    {
      $return[] = $message['value'];
    }

    return $return;
  }

  /**
   * Enable Warnings
   *
   * @param boolean $bool
   */
  function enable($bool)
  {
    $this->enabled = intval($bool);
  }

  /**
   * Return if we have output or not so fTheme knows if it should include the files it needs for our display
   *
   * @return bool
   */
  function hasMessages()
  {
    if ( $this->enabled != true || !count($this->messages) ) return false;
    return true;
  }

}
?>

<?php // $Id: fProfile.inc 8540 2007-12-21 15:02:50Z jhansche $
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
 * Engine Profiling Information Object
 *
 * This object stores and outputs profiling information
 *
 * @author     Joe Hansche <madCoder@gamesurge.net>
 * @link       http://framewerk.org
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2005-2007 the Framewerk Development Group
 * @version    $Rev: 8540 $
 * @package    Engine
 * @subpackage Messages
 * @uses       fInformation
 */

class fProfile extends fEngineInformation
{
  protected static $instance = NULL;
  protected $enabled = true;
  protected $precision = 4;
  protected $syslog, $logSeverity;
  protected $outputThreshold;
  protected $overThresholdLogSeverity;
  protected $fDebug;

  protected function __construct()
  {
    $this->profile('fProfile', 'fProfile Lifetime');
    $registry = new fRegistry('fProfile');
    $this->fDebug = fDebug::getInstance();
    $this->precision = $registry->precision;
    $this->enabled = $registry->enabled;
    $this->outputMethod = $registry->outputMethod;

    switch($this->outputMethod)
    {
      case 'syslog':
        $this->logSeverity = $registry->logSeverity;
        $this->outputThreshold = $registry->outputThreshold;
        $this->overThresholdLogSeverity = $registry->overThresholdLogSeverity;
        $this->syslog = fSyslog::getInstance();
        break;
    }
  }

  /**
   * Checks to see if the object is instanced in the internal static variable $instance and if not, does so
   *
   * @return fProfile instance
   */
  static function getInstance()
  {
    if ( !isset(self::$instance) )
    {
      self::$instance = new fProfile();
    }
    return self::$instance;
  }

  /**
   * No operation, use Profile()
   */
  function add($message) { return; }

  /**
   * Adds a message to the profiling information stack
   *
   * @param String $key - Unique profile key
   * @param String $message - Profiling Message
   */
  function profile($key, $message = NULL)
  {
    // Do nothing if disabled
    if ( ( $this->enabled != true ) || ( isset($this->done) && $this->done == true ) ) 
    {
      return;
    }

    if ( isset($this->messages[$key]) )
    {
      $item = $this->messages[$key];
      unset($this->messages[$key]);
      $this->fDebug->add("Reusing $key with $message; $item[message]", 8);
      if ( !isset($item['message']) || strlen($item['message']) == 0 || $item['message'] == NULL )
      {
        $item['message'] = $message;
      }
    } else {
      // Weird bug causes use of $this->fDebug to cause an exception
      fDebug::getInstance()->add("Adding $key; $message", 8);
      $item = array("message" => $message, "timestamps" => array());
    }
    $item['timestamps'][] = $this->timestamp();

    switch($this->outputMethod)
    {
      case 'syslog':
        if ( sizeof($item['timestamps']) % 2 == 0 )
        {
          $i = sizeof($item['timestamps']) - 2;
          $delta = number_format($item['timestamps'][$i + 1] - $item['timestamps'][$i], $this->precision);
          $d = debug_backtrace();
          $source = $d[1]['class'] . '->' . $d[1]['function'] . '()';
          $this->syslog->logMessage(sprintf('%ss %s [Line %d]: %s', $delta, $source, $d[0]['line'], $item['message']), $this->logSeverity);

          if ( $delta >= $this->outputThreshold && $this->logSeverity !== $this->overThresholdLogSeverity )
          {
            $this->syslog->logMessage(sprintf('%ss %s [Line %d]: %s', $delta, $source, $d[0]['line'], $item['message']), $this->overThresholdLogSeverity);
          }
        }
        break;
    }
    $this->messages[$key] = $item;
  }

  /**
   * returns formatted content via XSL transformation
   *
   * @return String Profiling Output
   */
  function output()
  {
    if ( $this->enabled != true || !count($this->messages) ) return;

    $this->profile('fProfile'); // End fProfile profile
    $this->done = true;
    
    switch($this->outputMethod)
    {
      case 'syslog':
        break;
      default:
        $dom = new DOMDocument('1.0', 'UTF-8');
        $messages = $dom->createElement('profile');

        foreach($this->messages as $key => $info)
        {
          // Only show it if we have at least 2 timestamps
          if ( ( $count = count($info['timestamps']) ) > 1 )
          {
            // Only show pairs of 2 (discard the last timestamp if it is not an even number)
            for($ii = 0; $ii < $count - ($count % 2); $ii += 2)
              $timestamp = number_format($info['timestamps'][$ii + 1] - $info['timestamps'][$ii], $this->precision);

            $xml = $dom->createElement('profile-message');
            $xml->appendChild($dom->createElement('object', $info['info']['class']));
            $xml->appendChild($dom->createElement('function', $info['info']['function']));
            $xml->appendChild($dom->createElement('timestamp', $timestamp));
            $xml->appendChild($dom->createElement('value', $info['message']));
            $messages->appendChild($xml);
          }
        }
        $dom->appendChild($messages);

        $theme = fTheme::getInstance();
        $out = $theme->transformXML($theme->xslFile("fEngineInformation"), $dom);
        return $out;
        break;
    }
  }

  function outputArray()
  {
    $this->profile('fProfile');
    $this->done = true;
    if ( $this->enabled != true || !count($this->messages) )
    {
      return array();
    }

    $return = array();
    foreach($this->messages as $info)
    {
      // Only show it if we have at least 2 timestamps
      if ( ( $count = count($info['timestamps']) ) > 1 )
      {
        // Only show pairs of 2 (discard the last timestamp if it is not an even number)
        for($ii = 0; $ii < $count - ($count % 2); $ii += 2)
        {
          $timestamp = number_format($info['timestamps'][$ii + 1] - $info['timestamps'][$ii], $this->precision);
        }
        $return[] = $timestamp . ' sec - ' . $info['message'];
      }
    }
    return $return;
  }

  /**
   * Return a more accurate float timestamp (not a UNIX timestamp, used for delta purposes only)
   * @return float timestamp
   */
  function timestamp()
  {
    return microtime(true);
  }

  /**
   * Toggle if profiling is enabled
   *
   * @param bool $bool
   * @param float $precision
   */
  function enable($bool, $precision)
  {
    $this->enabled = $bool;
    $this->precision = $precision;
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

<?php // $Id: fDebug.inc 8486 2007-12-20 15:22:22Z jhansche $
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
  * fDebug
  *
  * This file contains the fDebug object
  */
/**
 * Engine Debugging Information Object
 *
 * This object stores and outputs debugging information
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

class fDebug extends fEngineInformation
{
  protected static $instance = NULL;
  private $level = 0;
  private $done = false;

  private $syslog, $logSeverity;

  protected function __construct()
  {
    $registry = new fRegistry('fDebug');
    $this->level = $registry->level;
    $this->outputMethod = $registry->outputMethod;
    if ( $this->outputMethod == 'syslog' )
    {
      $this->logSeverity = $registry->logSeverity;
      $this->syslog = fSyslog::getInstance();
    }
  }


  /**
   * Checks to see if the object is instanced in the internal static variable $instance and if not, does so
   *
   * @return fDebug instance
   */
  static function getInstance()
  {
    if ( self::$instance == NULL )
    {
      self::$instance = new fDebug();
    }
    return self::$instance;
  }

  /**
   * Adds a message to the debugging information stack
   *
   * @param String $message - Debugging Message
   * @param int    $level   - Debugging Level
   * @param String $class   - Class Override
   */
  function add($message, $level = 8, $class = NULL)
  {
    // Fall through if our debugging level is set lower than the message level
    if ( ( $this->done ) || ( intval($level) > $this->level ) ) return;

    // If there is a message
    $temp = debug_backtrace();
    if ( strlen(trim($message)) )
    {
      // Get our calling object/file info
      if ( isset($class) )
      {
        if ( is_object($class) )
          $object = get_class($class);
        else
          $object = $class;
      } else {
        $info = $temp[1];
        if ( !isset($info['class']) )
        {
          $object = "(none)";
        } else {
          $object = $info['class'];
          if ( !isset($info['function']) )
          {
            $func = '(none)';
          } else {
            $func = $info['function'];
          }
        }
      }

      $info = $temp[0];
      $file = basename($info['file']);
      $line = $info['line'];
      if ( $object == "(none)" )
      {
        $object = $file;
      }

      // Construct the message array
      $message = array('timestamp'=>microtime(true), 'object'=>$object, 'value'=>$message, 'file'=>$file, 'line'=>$line, 'func'=>$func);

      switch($this->outputMethod)
      {
        case 'syslog':
          $source = $message['object'];
          if ( $message['func'] != '(none)' && strlen($message['func']) > 0 )
          {
            $source .= '->' . $message['func'] . '()';
          }
          $message['value'] = str_replace(array("\r","\n", "  "), " ", $message['value']);
          $this->syslog->logMessage(sprintf('%s: [Line %d]: %s', $source, $message['line'], $message['value']), $this->severity);
          break;

        default:
          break;
      }
      // Add the message
      $this->messages[] = $message;
    }
  }

  /**
   * Outputs formatted content in divs
   *
   * @return String Debugging Output
   */
  function output()
  {
    if ( $this->outputMethod != '' && $this->outputMethod != 'browser' )
    {
      return NULL;
    }

    $host = apc_fetch('host');
    if ( !$host )
    {
      list($host) = explode('.', shell_exec('hostname'));
      apc_store('host', trim($host), 86400);
    }
    $this->add('Handled by server: ' . trim($host), 1, 'fDebug');

    $this->done = true;

    if ( ( intval($this->level) == 0 ) || ( count($this->messages) == 0 ) )
      return null;

    $dom = new DOMDocument('1.0', 'UTF-8');
    $messages = $dom->createElement('debug');
    $messages->setAttribute("level", $this->level);

    foreach ( $this->messages as $message )
    {
      $xml = $dom->createElement('debug-message');
      $xml->appendChild($dom->createElement('timestamp', $message['timestamp']));
      $xml->appendChild($dom->createElement('file', $message['file']));
      $xml->appendChild($dom->createElement('line', $message['line']));
      $xml->appendChild($dom->createElement('object', $message['object']));
      $xml->appendChild($dom->createElement('value', strval($message['value'])));
      $xml->appendChild($dom->createElement('func', strval($message['func'])));
      $messages->appendChild($xml);
    }
    $dom->appendChild($messages);

    $theme = fTheme::getInstance();
    return $theme->transformXML($theme->xslFile('fEngineInformation'), $dom);
  }

  /**
   * Set the debugging level
   *
   * @param int $level
   */
  function setLevel($level)
  {
    $this->level = intval($level);
    $this->add('Setting Debug Level to: ' . $level, 1);
  }

  /**
   * Return the current debugging level
   *
   * @return int
   */
  function getLevel()
  {
    return $this->level;
  }

  /**
   * Return if we have output or not so fTheme knows if it should include the files it needs for our display
   *
   * @return bool
   */
  function hasMessages()
  {
    if ( !count($this->messages) ) return false;
    return true;
  }

  function outputArray()
  {
    $this->done = true;

    if ( ( intval($this->level) == 0 ) || ( count($this->messages) == 0 ) )
      return array();

    $return = array();

    foreach ( $this->messages as $message )
    {
      $return[] = $message['object'] . ' ('.$message['line'].'): ' . $message['value'];
    }
    return $return;
  }

}
?>

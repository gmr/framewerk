<?php // $Id: fMessages.inc 919 2007-08-06 18:34:41Z dallas $
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
 * Engine Messages Handling Object
 *
 * This class handles messages that are to be shown to the client
 *
 * @author     Rick Martinez <rick@digxworld.com>
 * @link       http://framewerk.org
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2005-2007 the Framewerk Development Group
 * @version    $Rev: 919 $
 * @package    Engine
 * @subpackage Messages
 * @uses       fSingletonObject
 */

class fMessages extends fSingletonObject
{
  static $instance = null;
  private $fSession, $fTheme, $messages = array();

  static function getInstance()
  {
    if ( self::$instance == null ) self::$instance = new fMessages();
    return self::$instance;
  }

  /**
   * Class Constructor
   *
   */
  function __construct()
  {
    parent::__construct();

    $this->fTheme       = fTheme::getInstance();
    $this->fSession     = fSession::getInstance();
    $this->messages     = unserialize($this->fSession->message);

    if ( !is_array($this->messages) ) $this->messages = array();
  }

  /**
   * Forcefully unset a singleton instance.
   */
  static function destroy()
  {
    self::$instance = NULL;
  }

  /**
   * Adds a new message to display for the user
   *
   * @param fMessage $message Message Object
   * @return bool Returns whether it was successfully added or not
   */
  function add($message)
  {
    if ( ! ($message instanceof fMessage) ) return false;
      $this->messages[] = $message;
        $this->save();
    $this->fTheme->addObjectCSSFile('fMessages');           
    return true;
  }

  /**
   * Gets messages from internal storage
   *
   * @param string $type
   * @return mixed Returns messages
   */
  function get($type = "xhtml")
  {
    $this->fDebug->add('Entering fMessages::get with ' . Count($this->messages) . ' waiting.', 3);

    if ( count($this->messages) < 1 ) return;

    $count = 0;
    $dom = new DOMDocument('1.0', 'UTF-8');
    $root = $dom->createElement('messages');
    foreach ( $this->messages as $key => $message )
    {
      /* @var $message fMessage */
      if ( $node = $message->xml() )
      {
        $root->appendChild($dom->importNode($node, true));
        $count++;
      }
      unset ($this->messages[$key], $node);
    }
    $this->save();
    if ( $count === 0 ) return;
      $dom->appendChild($root);

    switch ( $type )
    {
      case "dom":
        return $dom;
        break;
      case "xml":
        return $dom->saveXML();
        break;
      case "xhtml":
      default:
        $xslfile = $this->fTheme->xslFile('fMessages');
        return $this->fTheme->transformXML($xslfile, $dom->saveXML());
        break;
    }
  }

  /**
     * Save the message array to the fSession object
   */
  function save()
  {
    $this->fDebug->add('Saving messages to Session');
    $this->fSession->message = serialize($this->messages);
  }
  
}
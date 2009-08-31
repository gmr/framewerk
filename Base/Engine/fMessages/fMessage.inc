<?php // $Id: fMessage.inc 878 2007-06-14 00:38:08Z gmr $
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
 * fMessage Abstract Class
 *
 * Abstract class for a single engine message
 *
 * @author     Rick Martinez <rick@digxworld.com>
 * @link       http://framewerk.org
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2005-2007 the Framewerk Development Group
 * @version    $Rev: 878 $
 * @package    Engine
 * @subpackage Messages
 * @todo       Fix TMessage and children so ::createDom() works properly with ::__wakeup()
 * @abstract
 */

abstract class fMessage
{
  protected $message;
  public $dom;
  protected $domVersion = "1.0";
  protected $domEncoding = "UTF-8";

  /**
   * Sets initial message
   *
   * @param string $message
   */
  function __construct($message)
  {
    $this->message = array('time' => time(), 'message' => $message);
  }

  /**
   * Creates DOM object for use by child classes and checks for message expiration
   *
   */
  function createDom()
  {
    $config = fMain::getInstance()->configuration->site->messageExpiration;
    $time = time();

    $this->dom = new DOMDocument($this->domVersion, $this->domEncoding);
    $message = $this->dom->createElement('message');

    if ( $this->message['time'] < ($time - intval($config['expires'])) )
    {
      switch( strval($config['action']) )
      {
        case 'delete':
          unset($this->dom, $this->message);
          return false;
        case 'display':
          $difference = ($time - $this->message['time']);
          $display    = '';
          if ( $difference < 60 )
          {
            $display = $difference . ' seconds ago.';
          } else {
            $display = floor($difference / 60) . ' minutes ago.';
          }
          $age = $this->dom->createAttribute('age');
          $age->appendChild($this->dom->createTextNode($display));
          $message->appendChild($age);
          break;
        case '':
        case 'none':
          break;
        default:
          throw new Exception('Invalid Message Expiration Action');
      }
    }
    return $message;
  }

  /**
   * Returns DOM node containing message information
   *
   * @return DOMNode
   */
  abstract function xml();

}
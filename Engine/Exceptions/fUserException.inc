<?php // $Id: fUserException.inc 919 2007-08-06 18:34:41Z dallas $
############################################################################
#    Copyright (C) 2006-2007 by the Framewerk Development Group            #
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
 * fUserException
 *
 * Exception object for fUser and Backend Errors
 *
 * @since      2006-09-28
 * @author     Joe Hansche <madcoder@gamesurge.net>
 * @link       http://framewerk.org
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2006-2007 the Framewerk Development Group
 * @package    Engine
 * @subpackage Exceptions
 * @version    $Revision: 919 $
 */

class fUserException extends Exception
{
  // This is a flag that can be used to indicate to fUser that this exception may be ignored
  const E_FUSER_INFO = 0x10;

  // Error code set when the backend cannot find the specified user.  Should not interrupt page load (INFO)
  const E_FUSER_NOSUCHUSER = 0x11; // 0x01 | 0x10

  // Backend-specific error
  const E_FUSER_BACKEND = 0x08;

  function __construct($message, $code = 0)
  {
    if ( $message instanceOf Exception )
    {
      parent::__construct($message->getMessage(), $message->getCode());
      $this->file = $message->getFile();
      $this->line = $message->getLine();
      return;
    } else {
      parent::__construct($message, $code);
    }
  }
}
?>

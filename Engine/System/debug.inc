<?php // $Id: debug.inc 929 2007-08-14 23:47:33Z gmr $
############################################################################
#    Copyright (C) 2004-2007 by the Framewerk Development Group            #
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
 * Framewerk Error Handling
 *
 * Contains functions specifying how to handle PHP error messages and exceptions
 *
 * @since      2004-09-14
 * @author     Joe Hansche <madCoder@gamesurge.net>
 * @author     Rick Martinez <rick.martinez@gmail.com>
 * @author     Gavin M. Roy <gavinmroy@gmail.com>
 * @version    $Rev: 929 $
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2004-2007 the Framewerk Development Group
 * @package    Engine
 * @subpackage Library
 */

/**
 * Error Handler
 *
 * @param int $code
 * @param string $message
 * @param string $file
 * @param int $line
 */
function errorHandler($code, $message, $file, $line)
{
  /**
  // Store the errors data as the lastError in fMain
  fMain::getInstance()->lastError = array('code' => $code, 'message' => $message, 'file' => $file, 'line' => $line);

  switch ( $code )
  {
    case E_NOTICE:
      fDebug::getInstance()->add('PHP Notice: ' . $file . '(' . $line . '): ' . $message, 6);
      break;
    case E_USER_ERROR:
    case E_USER_NOTICE:
    case E_USER_WARNING:
      $temp = explode('/', $file);
      $temp = explode('.', array_pop($temp));
      $file = $temp[0];
      fWarn::getInstance()->add($file . '(' . $line . '): ' . $message);
      break;
    default:
      fDebug::getInstance()->add('Unknown Error #' . $code . ': ' . $file . '(' . $line . '): ' . $message);
      break;
  }**/
}

/**
 * Exception Handler
 *
 * @param Exception $exception
 */
function exceptionHandler($exception)
{
  try
  {
    $error = new fHTTPMessage($exception);
    $error->show();
  } catch ( Exception $e ) {
    echo "<pre><strong>\nA fatal Framewerk error occured that prevented it from handling an exception.</strong>\n\n<hr />\n";
    echo "Message: {$e->getMessage()}\n\n";
    echo "File: {$e->getFile()}\n";
    echo "Line: {$e->getLine()}\n\n";
    echo "{$e->getTraceAsString()}\n</pre>";
  }
  die();
}

// Set our error reporting levels and handlers
error_reporting(E_ALL ^ ( E_USER_ERROR | E_USER_NOTICE | E_USER_WARNING | E_NOTICE | E_WARNING ) );
set_error_handler('errorHandler', E_USER_ERROR | E_USER_NOTICE | E_USER_WARNING  | E_NOTICE | E_WARNING );
set_exception_handler('exceptionHandler');
?>
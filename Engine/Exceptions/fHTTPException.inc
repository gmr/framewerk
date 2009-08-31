<?php // $Id: fHTTPException.inc 842 2007-03-29 06:28:02Z gmr $
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
 * HTTP Exception
 *
 * Exception for HTTP Errors
 *
 * @since      2006-01-24
 * @author     Rick Martinez <rick.martinez@gmail.com>
 * @link       http://framewerk.org
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2006-2007 the Framewerk Development Group
 * @package    Engine
 * @subpackage Exceptions
 * @version    $Revision: 842 $
 */

class fHTTPException extends Exception
{
  /**
   * HTTP Exception Constructor
   *
   * @param int $code
   */
  public function __construct($code, $message = null)
  {
    // Make sure we have a valid HTTP Exception #
    if ( !is_numeric($code) )
    {
      throw new Exception("Invalid Argument, expecting integer");
    }

    switch ( $code )
    {
      case 404:
        $this->message = "Object or file not found: " . $_SERVER['REQUEST_URI'];
        $this->code = 404;
        break;

      case 403:
        $this->message = "Forbidden: " . $_SERVER['REQUEST_URI'];
        $this->code = 403;
        break;

      case 500:
        $this->message = "Internal Server Error";
        $this->code = 500;
        break;

      default:
        $this->message = "Unsupported error code";
        $this->code = $code;
        break;
    }
    
    if ( !empty($message) )
      $this->message = $message;
  }
}
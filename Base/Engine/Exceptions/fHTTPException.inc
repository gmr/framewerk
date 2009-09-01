<?php
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
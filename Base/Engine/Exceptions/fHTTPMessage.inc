<?php // $Id: fHTTPMessage.inc 931 2007-09-10 17:41:46Z gmr $
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
 * HTTP Message
 *
 * Generates HTTP Message for HTTP Exceptions
 *
 * @since      2006-01-22
 * @author     Rick Martinez <rick.martinez@gmail.com>
 * @link       http://framewerk.org
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2006-2007 the Framewerk Development Group
 * @package    Engine
 * @subpackage Exceptions
 * @version    $Revision: 931 $
 *
 * @todo Move errors messages to XSL for different styling/xhtml per error code?
 */

class fHTTPMessage extends fFatalMessage
{
  private $exception, $trace;

  /**
   * Constructor
   *
   * @param Exception $exception
   */
  public function __construct($exception, $trace = NULL)
  {
    // Call parent constructor
    parent::__construct();

    // Decide what to show the user
    switch ( $exception->getCode( ) )
    {
      case 404:
        $this->error = '404 Not Found';
        $this->description = 'The object or file requested could not be found.';
        header($protocol . ' 404 Not Found');
        break;

      case 403:
        // Check if logged in
        /**
         * Todo I don't this this belongs here.  At best, it could be in fSession?
        $registry = new fRegistry($this);

        if ( fSession::getInstance()->isAuthenticated() == 'false' )
        {
          // Should we redirect the client?
          $redir = $registry->redirectGuestOn403;
          if ( !empty($redir) )
          {
            $fMain = array_shift($this->registry->xpath('/registry/node[@name="fMain"]'));
            $redir = str_replace("{@baseURL}", $fMain->site->baseURL, $redir);
            $referer = ( strtolower($_SERVER['HTTPS']) == 'on' ? "https://" : "http://" );
            $referer .= $_SERVER['HTTP_HOST'] . $_SERVER['REQUEST_URI'];
            header( 'Location: ' . $redir . '?redirect=' . urlencode( $referer ) );
            exit();
          }
        } */

        $this->error = '403: Forbidden';
        $this->description = 'You do not have permission to access the requested content on this server.';
        header($protocol . ' 403 Forbidden');
        break;

      case 500:
        $this->error = '500: Internal Server Error';
        $this->description = 'The server encountered an internal error or misconfiguration and was unable to complete your request.';
        header($protocol . ' 500');
        break;

      default:
        $this->error = '500: Internal Server Error';
        $this->description = 'The server encountered an internal error or misconfiguration and was unable to complete your request.';
        header($protocol . ' 500');
        break;
    }

    $this->exception = $exception;
    $this->message = $this->exception->getMessage();
    $this->line = $this->exception->getLine();
    $this->file = $this->exception->getFile();
    
    $fDebug = array_shift($this->registry->xpath('/registry/node[@name="fDebug"]'));
    if ( $fDebug->level > 0 )
    {
      $this->trace = $this->exception->getTrace();
    }
    
    $dom = $this->createDOM();
    $this->content = $this->createPage( $dom );
  }

  /**
   * Creates DOM Document for use with error page generation
   *
   * @return DOMDocument
   */
  protected function createDOM()
  {
    $exception = $this->exception;

    // Create XML for exception
    $dom = new DOMDocument('1.0', 'UTF-8');
    $root = $dom->createElement("exception");
    $root->appendChild( $dom->createElement("error", $this->error) );
    $root->appendChild( $dom->createElement("description", $this->description) );
    $root->appendChild( $dom->createElement("message", $this->message) );

    // If trace is on, show the back trace
    if ( isset($this->trace) )
    {
      $code = $dom->createElement('code');
      $root->appendChild($code);
      foreach ( $this->trace AS $entry )
      {      
        $line = $dom->createElement( 'backtrace' );
        $code->appendChild($line);

        // Set attributes
        $line->appendChild( $dom->createElement( 'file', $entry['file'] ) );
        $line->appendChild( $dom->createElement( 'line', $entry['line'] ) );
        $line->appendChild( $dom->createElement( 'function', $entry['function'] ) );
        $line->appendChild( $dom->createElement( 'class', $entry['class'] ) );
        $type = $dom->createElement( 'type' );
        $type->appendChild( $dom->createCDataSection( $entry['type'] ) );
        $line->appendChild( $type );
        
        // Add args
        $args = $dom->createElement( 'args' );
        $line->appendChild( $args );
        
        for ( $y = 0; $y < count( $entry['args'] ); $y++ )
        {
          $arg = $dom->createElement( 'arg' );
          $args->appendChild( $arg );

          $name = get_class( $entry['args'][$y] );
          $arg->appendChild( $dom->createElement( 'name', $name ) );
          $value = $dom->createElement( 'value' );
          $arg->appendChild( $value );
          
          // Convert the values to a UL
          if ( is_object( $entry['args'][$y] ) ||  is_array( $entry['args'][$y] ) )
          {
            $importNode = $this->print_rToUL( $entry['args'][$y] );
            $value->appendChild( $dom->importNode( $importNode, true ) );
          } else {
            $value->appendChild( $dom->createCDataSection( $entry['args'][$y] ) );
          }  
        }
    
        // Get the line of code
        $file = file_get_contents($entry['file']);
        $data = explode("\n", $file);
        
        $source = $dom->createElement('source');
        $line->appendChild($source);

        // Figure out where to start showing source
        $offset = $entry['line'] - 5;
        if ( $offset < 0 ) $offset = 0;
        $max = $offset + 10;
        if ( $max > count($data) ) $max = count($data);
        
        // Get the code
        for ( $l = $offset; $l < $max; $l++ )
        {
          $sourceCode = $dom->createElement( 'line' );
          $sourceCode->setAttribute( 'number', $l );
          $sourceCode->appendChild( $dom->createCDataSection( $data[$l] ) );        
          $source->appendChild( $sourceCode );
        }
      }
    }
    $dom->appendChild($root);
    return $dom;
  }

  /**
   * Pass in an object/array and turn it into an XHTML UL
   *
   * @param $data
   * @return DOM Document
   */
  function print_rToUL( $data, $baseDepth = 1 )
  {
    $lines = explode("\n", print_r($data, true));
    $depth = 0;
    $dom = new DOMDocument('1.0', 'UTF-8');
    $ul = $dom->createElement('ul');
    $last = $ul;
    $dom->appendChild($last);
    foreach ( $lines AS $line )
    {
      $line = Trim($line);
      switch ( substr($line, 0, 1) )
      {
        case '(':
          if ( $depth >= $baseDepth )
          {
            $subnode = $dom->createElement('ul');
            $current->appendChild($subnode);
            $previous[$depth + 1] = $last;
            $last = $subnode;
          }
          $depth++;
          break;
        case ')':
          $last = $previous[$depth];
          $depth--;
          break;
        default:
          if ( ( strlen( Trim($line) ) ) && ( $depth >= $baseDepth ) )
          {
            $node = $dom->createElement('li', $line);
            $last->appendChild($node);
            $current = $node;
          }
      }
    }
    return $ul;
  }

}
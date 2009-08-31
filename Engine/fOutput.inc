<?php // $Id: fOutput.inc 8486 2007-12-20 15:22:22Z jhansche $
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
 * fOutput
 *
 * Contains the fOutput Class
 */
/**
 * Output Buffer Management and Type Object
 *
 * This Object is the primary buffer for output generation.  It will allow
 * for text data such as html, xml, rss, etc and for binary data such as
 * jpeg images and movie files.  It also manages the needed information
 * for sending the output such as the mime type.
 *
 * @author     Gavin M. Roy <gavinmroy@gmail.com>
 * @link       http://framewerk.org
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2004-2007 the Framewerk Development Group
 * @version    $Rev: 8486 $
 * @package    Engine
 * @subpackage Core
 * @since      2004
 */

class fOutput extends fSingletonObject
{
  static private $instance = NULL;
  private $cachable = false, $expires, $header, $headers, $lastModified, $mime, $outputContent, $pageName, $pageCategory, $fTheme, $parse = true, $registry;
  public $enabled = true, $engineInfo = true, $previous = 0, $showPageName = true, $tidy = false;

  /**
   * Set a default mime type on initialization
   */
  protected function __construct()
  {
    parent::__construct();

    $this->registry = new fRegistry($this);

    $this->fTheme = fTheme::getInstance();

    // Set our default mimetype
    $this->setMimeType('text/html; charset=' . $this->registry->charset . ';');  // Default Mime-Type

    // Load our registry
    $this->tidy = $this->registry->tidy;
    $this->expires = $this->registry->expires;
  }

  /**
   * Append content to our existing output content
   *
   * @param string $data
   */
  function appendContent($data = NULL)
  {
    if ( $data != NULL )
    $this->outputContent .= $data;
  }

  /**
   * Append HTTP Headers
   *
   * @param string $data
   */
  function appendHTTPHeaders($data = NULL)
  {
    if ( $data != NULL )
    {
      $temp = $this->headers;
      $temp[] = $data . "\r\n";
      $this->headers = $temp;
    }
  }

  /**
   * Get the current or new instance
   *
   * @return fOutput instance of this object
   */
  static function getInstance()
  {
    if ( self::$instance == NULL ) self::$instance = new fOutput();
      return self::$instance;
  }

  /**
   * Return the current contents of the output buffer
   * 
   * @return string
   */
  function getContent()
  {
    return $this->outputContent;
  }

  /**
   * Get the current page category
   *
   * @return String Page category
   */
  function getPageCategory()
  {
    return $this->pageCategory;
  }

  /**
   * Return the current page name for display
   *
   * @return String Page Name
   */
  function getPageName()
  {
    if ( $this->showPageName == true ) return $this->pageName;
    return NULL;
  }

  /**
   * return the page title
   *
   * @return String Page Name for HTML Head Title
   */
  function getTitle()
  {
    return $this->pageName;
  }

  /**
   * Reset our content and mime-type to the default, initialized state
   */
  function reset()
  {
    $this->outputContent = "";     // Delete anything in $this->outputContent
    $this->setMimeType('text/html; charset=' . $this->registry->charset . ';');  // Default Mime-Type
  }

  /**
   * Prepend content to our existing output content
   *
   * @param string $data
   * @see fOutput::appendContent()
   */
  function prependContent($data = NULL)
  {
    if ( $data != NULL )
    $this->outputContent = $data . $this->outputContent;
  }

 /**
  * Replace { and } with HTML equivilents
  *
  * @param string $input
  * @return string
  */
  static function sanitizeVariables($input)
  {
    $find = array('{', '}');
    $replace = array('&#123;','&#125;');
    return str_replace($find,$replace,$input);
  }

  /**
   * Send our output to the browser
   */
  function sendOutput()
  {
    if ( $this->enabled == false ) return;
    $this->fProfile->profile('fOutputSend');

    // if our mime type is of type text then Parse the output
    $temp = explode("/", $this->mime);

    // Set the Generator Header
    $this->appendHTTPHeaders('X-Generator', 'Framewerk');

    // If we have a text document allow parsing if we're allowed to
    if ( $temp[0] == "text" )
    {
      // Combine our output with our theme
      $output = $this->fTheme->buildPage($this->outputContent, $this->mime);

      // Parse our output
      if ( $this->parse == true )
      {
        $this->fProfile->profile('fOutput_Parse');
        $this->outputContent = fParser::getInstance()->parse($output);
        $this->fProfile->profile('fOutput_Parse', 'fOutput: Parsing final output');
      }

    } else {
      $this->appendHTTPHeaders('Content-Transfer-Encoding', 'binary');
    }

    // Set cache stuff if it's cacheable
    if ( $this->cachable === true )
    {
      $this->appendHTTPHeaders('Pragma', 'cache');
      $this->appendHTTPHeaders('Cache-control','public, must-revalidate, max-age=0');
      $this->appendHTTPHeaders('Expires: ' . gmdate('D, d M Y H:i:s \G\M\T', time() + intval($this->expires)));
      if ( isset($this->lastModified) )
      {
        $this->appendHTTPHeaders('Last-Modified: ' . $this->lastModified);
      }
      $this->appendHTTPHeaders('Etag: ' . md5($this->outputContent));
      $this->fDebug->add('Sending Cache Control Headers', 4);
    } else {
      $this->appendHTTPHeaders("Pragma: no-cache");
      $this->fDebug->add('Sending No-Cache Header', 4);
    }

    // Send out the mime type
    $this->appendHTTPHeaders("Content-type: " . $this->mime);

    // Send the headers
    if ( !headers_sent() )
    {
      foreach ( $this->headers AS $header )
      {
        header($header);
      }
    }

    // Replace the engine info variables if we have a text document
    $temp = explode("/", $this->mime);

    // Append our fInformation output
    if ( $temp[0] == "text" )
    {
      // Change our our {pageHead} variable
      $this->outputContent = str_replace('{pageHead}', $this->fTheme->getHTMLHead(), $this->outputContent);
      // Do something similar for {pageFoot}, if the template supports it
      $this->outputContent = str_replace('{pageFoot}', $this->fTheme->getHTMLFoot(), $this->outputContent);

      // If we have fWarn/Debug/Profile Variables, replace them with output from the EngineWarn/Debug/Profile content
      if ( $this->engineInfo )
      {
        // Add our Warning and Debugging Information
        $find = '{EngineInfo:json}';
        if ( strpos($this->outputContent, $find) !== false )
        {
          $this->fProfile->profile('fOutputSend', 'fOutput Send Output [Up to fEngineInfo replacement]');
          $replace = fEngineInformation::debugUserOutput(true, true);
          $this->outputContent = str_replace($find, $replace, $this->outputContent);
        }
        else if ( strpos($this->outputContent, '{EngineInfo:') !== false )
        {
          $find = array("{EngineInfo:Warn}", "{EngineInfo:Debug}","{EngineInfo:Profile}");
          $replace = array(fWarn::getInstance()->output(), $this->fDebug->output(), $this->fProfile->output());
        }
      }

      // Clean up our output before sending.
      if ( ( $this->tidy == true ) && ( in_array('tidy', get_loaded_extensions()) ) )
      {
        switch ( $this->mime )
        {
          case "application/xhtml+xml":
          case "text/html":
            $config = array("output-xhtml"=>true, "indent"=>true, "wrap"=>false, "merge-divs"=>false);
            $this->outputContent = tidy_repair_string($this->outputContent, $config);
            break;
          case "text/xml":
            $config = array("input-xml"=>true, "add-xml-decl"=>true, "output-xml"=>true, "indent"=>true, "wrap"=>false);
            $this->outputContent = tidy_repair_string($this->outputContent, $config);
            break;
        }
      }
    }

    // Compute and send the size
    $size = strlen($this->outputContent);
    if ( $size > 0 )
    {
      Header("Content-Length: " . $size);
    }

    // Send the output
    echo $this->outputContent;
  }

  /**
   * Set the state of client side caching
   *
   * param bool Can cache?
   */
  function setCachable($canCache)
  {
    $this->cachable = $canCache;
  }

  /**
   * Set and/or return our content
   *
   * @param string $outputContent
   */
  function setContent($outputContent = NULL)
  {
    if ( $outputContent != NULL )
      $this->outputContent = $outputContent;
    return $this->outputContent;
  }

  /**
   * Set the expiration offset for caching
   *
   * param int Expiration offset in seconds
   */
  function setExpirationOffset($offset)
  {
    $this->expires = intval($offset);
    $this->fDebug->add("Setting expiration offset: " . $offset, 4);
  }

  /**
   * Set the last modified date to turn on cache control
   *
   * @param int Unix Timestamp $timestamp
   */
  function setLastModified($timestamp)
  {
    // Only reset this if the date is newer then the previous time this was called in this request
    if ( $timestamp >  $this->previous )
    {
      $this->previous = $timestamp;
      $this->lastModified = gmdate('D, d M Y H:i:s \G\M\T', $timestamp);
      $this->fDebug->add('Setting Last Modified Date/Time for Cache-Control: ' . $this->lastModified, 4);
    }
  }

  /**
   * Set and/or return our Mime Type
   *
   * @param String mime type
   * @return String mime type
   */
  function setMimeType($mime = NULL)
  {
    if ( $mime !== NULL )
      $this->mime = $mime;
    return $this->mime;
  }

  /**
   * Set the current page category
   *
   * @param String Page category
   */
  function setPageCategory($data)
  {
    $this->fDebug->add('Setting page category:' . $data, 4);
    $this->pageCategory = $data;
  }


  /**
   * Set the current page name
   *
   * @param String Page Name
   */
  function setPageName($data)
  {
    $this->fDebug->add('Setting page name:' . $data, 4);
    $this->pageName = $data;
  }

  /**
   * If sending a binary file, you can suggest the filename the browser uses to save or view
   *
   * @param string $filename
   */
  function setSuggestedFilename($filename)
  {
    $this->appendHTTPHeaders('Content-Disposition: attachment; filename="' . $filename . '"');
    $this->fDebug->add('Setting Suggested Filename: ' . $filename, 4);
  }

  /**
   * Determine whether we will use the parser
   * @param Boolean True to enable the parser (default); False to disable
   */
  function useParser($bool = true)
  {
    $this->parse = !(!$bool);
  }

  /**
   * Override the registry default of wether to use tidy or not
   *
   * @param bool $bool
   */
  function useTidy($bool = true)
  {
    $this->tidy = $bool;
  }

  /**
   * Sanitize output (remove nonprintable characters) 
   *
   * @param string $inputString
   * @return string sanitized string
   */

  static final function sanitize( $inputString )
  {
    if ( is_numeric($inputString) || $inputString === NULL || is_bool($inputString) )
    {
      return $inputString;
    }
    $stringLength = strlen( $inputString );
    $outputString = '';
    for( $i=0; $i<$stringLength; $i++ )
    {
      if( ord( $inputString[$i] ) < 32 )
      {
        continue;
      }

      $outputString .= $inputString[$i];
    }
    return $outputString;
  }

}
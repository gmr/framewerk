<?php // $Id: fRESTApplication.inc 919 2007-08-06 18:34:41Z dallas $
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
 * fRESTApplication
 *
 * Contains the fRESTApplication abstract class
 */
/**
 * Binary Application abstract object
 *
 * The Binary Application object is for sending out binary data such as graphics, PDF's, zip files, executables, or Flash.
 *
 * @author     Gavin M. Roy <gavinmroy@gmail.com>
 * @link       http://framewerk.org
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2008 the Framewerk Development Group
 * @version    $Rev: 919 $
 * @package    Engine
 * @subpackage Abstract_Applications
 * @since      2008-06-05
 * @abstract
 */

abstract class fRESTApplication extends fApplication
{
  private $dom, $parentNode, $result;
  protected $error = false;

  /**
   * Application Constructor
   *
   * Sets the all the needed internal variables and engine parameters.
   */
  function __construct()
  {
    parent::__construct();

    // Set our page category and name
    $this->pageCategory('default');
    $this->pageName('The object you called did not properly extend the fRESTApplication Object');

    // We're going to output XML
    $this->fOutput->setMimeType('text/xml');

    // No caching output
    //$this->fOutput->noCache = true;

    // Don't use the theme or try and merge them
    $this->fTheme->useTheme(false);

    // Setup our default DOM document
    $this->dom = new DOMDocument('1.0', 'UTF-8');
    $this->dom->formatOutput = true;

    // Create the root node and result subnode
    $this->parentNode = $this->dom->createElement(get_class($this));
    $this->dom->appendChild($this->parentNode);
    $this->result = $this->dom->createElement('result');
    $this->parentNode->appendChild($this->result);

    // Append the hostname for debugging
    $hostname = new tHostname();
    $this->parentNode->appendChild($this->dom->createElement('host', $hostname->get()));

    // Append the request time
    $this->parentNode->appendChild($this->dom->createElement('requestTime', strftime('%x %X')));
  }

  protected function buildOutput()
  {
    if ( !$this->error )
      $this->result->setAttribute('type', 'ok');

    // Transform and and set the output to our XML
    $xml = $this->dom->saveXML();
    /**
    $fp = fopen('/var/log/api.log', 'a');
    fputs($fp, $xml);
    fclose($fp);
    **/
    $this->fOutput->setContent($xml);
  }

  protected function error($error)
  {
    // Set the protocol we're using
    //header($_SERVER['SERVER_PROTOCOL'] . ' 501 Internal Server Error');

    // Build and return the output
    $this->error = true;
    $this->result->setAttribute('type', 'error');
    $this->result->appendChild($this->dom->createElement('message', $error));
    $this->buildOutput();
  }

  protected function noAction()
  {
    // Set the protocol we're using
    header($_SERVER['SERVER_PROTOCOL'] . ' 404 Not Found');

    // Build and return the output
    $this->error = true;
    $this->result->setAttribute('type', 'error');
    $this->result->appendChild($this->dom->createElement('message', 'Action not found'));
    $this->buildOutput();
  }


  protected function addArrayOutput($array, $parent = null)
  {
    // If we don't have a parent node, then set it to the default $this->result
    if ( !isset($parent) )
      $parent = $this->result;

    foreach ( $array AS $key=>$value )
    {

      $node = $this->dom->createElement($key);
      $parent->appendChild($node);

      if ( is_array($value) )
      {
        $this->addArrayOutput($value, $node);
      } else {
        $node->appendChild($this->dom->createTextNode($value));
      }
    }
  }

}
?>

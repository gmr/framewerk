<?php // $Id: fXMLApplication.inc 919 2007-08-06 18:34:41Z dallas $
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
 * fXMLApplication
 *
 * This file contains the fXMLApplication class
 */
/**
 * XML Application abstract class
 *
 * This abstract TXMLApplication class is inherited by all Framewerk XML Applications
 * Applications are the applications in Framewerk which are responsible for the
 * direct interaction with the user, that is Applications are the web applications
 * themselves.
 *
 * @author     Gavin M. Roy <gavinmroy@gmail.com>
 * @link       http://framewerk.org
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2005-2007 the Framewerk Development Group
 * @version    $Rev: 919 $
 * @since      2005-05-09
 * @package    Engine
 * @subpackage Abstract_Applications
 * @abstract
 */

abstract class fXMLApplication extends fApplication
{

  public $dom;

  /**
   * Object Constructor
   *
   * Sets our pointer variables, sets default values, and sets the output parameters for a XML file.
   */
  function __construct($mimeType = 'text/xml', $domVersion = '1.0', $domEncoding = 'UTF-8')
  {
    parent::__construct();

    // Set Our XML Parameters
    $this->fOutput->setMimeType( $mimeType );
    $this->fTheme->useTheme( false );
    $this->fOutput->engineInfo = false;
    $this->dom = new DOMDocument( $domVersion, $domEncoding );
  }
  /**
   * Output a string or DOMDocument
   *
   * @param boolean|string|DOMDocument $value false to use the class dom instance and call saveXML(); a string; or a DOMDocument object to call saveXML()
   * @param boolean $transform Whether or not to XSL transform $value
   * @param string $xslFile XSL File to use
   */
  function output($value = false, $transform = false, $xslFile = NULL)
  {
    if ($value instanceof DOMDocument)
      $value = $value->saveXML();
    elseif ($value === false)
      $value = $this->dom->saveXML();     
      
    parent::output($value,$transform,$xslFile);
  }
}
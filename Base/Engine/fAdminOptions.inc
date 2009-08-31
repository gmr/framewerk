<?php // $Id: fAdminOptions.inc 888 2007-06-14 02:43:28Z gmr $
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
 * fAdminOptions
 *
 * This file contains the fAdminOptions object
 */
/**
 * Admin Options
 *
 * Builds an xml object that gets transformed for admin options
 *
 * @author     Gavin M. Roy <gavinmroy@gmail.com>
 * @link       http://framewerk.org
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2005-2007 the Framewerk Development Group
 * @version    $Rev: 888 $
 * @package    Engine
 * @subpackage Core
 * @uses       fObject
 * @since      2005-09-10
 */

class fAdminOptions extends fObject
{

  function returnList()
  {
    // Setup the objects we use
    $fSession        = fSession::getInstance();
    $fExecutionStore = fExecutionStore::getInstance();
    $fTheme          = fTheme::getInstance();

    // Get our option list
    $values = $fExecutionStore->getValue('fAdminApplication', 'Options');

    // Create a DOM Document
    $document = new DomDocument('1.0');
    $domOptions = appendDomNode($document, 'Options');

    // Loop through our values adding them to the dom if the user has permission
    if ( is_array($values) )
    {
      foreach ( $values AS $value )
      {
        if ( $fSession->fUser->checkPermission($value[2]) )
        {
          $domOptions = appendDomNode($document, $domOptions, NULL, appendDomNode($document, 'Option', array('url' => $value[1], 'selected' => $value[3]), NULL, $value[0]));
        }
      }
    } else
    $domOptions = appendDomNode($document, $domOptions, NULL, appendDomNode($document, 'Option', array('selected' => true), NULL, '{@pageName}'));

    // Finalize our XML document and get an XML string
    $document->appendChild($domOptions);
    $xmlout = $document->saveXML();

    // Transform the XML via the XSL
    $xslfile = $fTheme->xslFile('fAdminOptions');
    $output =  $fTheme->transformXML($xslfile, $xmlout);
    if ( $output == false )
    $output = 'Error transforming XML';

    // Return the output
    return $output;
  }
  
}
<?php // $Id$
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
   * Soap Base Object
   *
   * Instances soap handler objects in Includes/Library/Soap
   *
   * @since      2005-07-12
   * @author     Gavin M. Roy <gavinmroy@gmail.com>
   * @link       http://framewerk.org
   * @license    http://opensource.org/licenses/bsd-license.php BSD License
   * @copyright  Copyright 2005-2007 the Framewerk Development Group
   * @package    Applications
   * @subpackage Toolkit
   * @uses       fApplication
   * @version    $Rev$
   */

class tSoapServer extends fApplication
{
  private $request, $soapServer, $tempfile;

  function __construct()
  {
    // Make sure we have the soap extension loaded
    $extensions = get_loaded_extensions();
    if ( !in_array("soap", $extensions) )
      throw new fHTTPException(501, "This installation of PHP does not support SOAP.");

    parent::__construct();

    // Set an ini variable so it doesnt cash the wsdl information
    ini_set("soap.wsdl_cache_enabled", "0");

    // Set Our XML Parameters
    $this->fOutput->enabled = false;

    // Change config parameters
    $this->fMain->configuration->debug = 0;
    $this->fMain->configuration->warnings = false;
    $this->fMain->configuration->profile = false;

    // If we have a class that was asked for, use it
    $className = $this->uriValues[0];

    // Put our request data in a buffer due to a bug in php5.2
    $this->request = file_get_contents('php://input');

    $vars = get_class_vars($className);
    if ( $vars['soapApplication'] == true )
    {
      // Get the WSDL
      $temp = new tWSDLGenerator();
      $wsdl = $temp->build($this->uriValues[0]);

      $this->tempfile = tempnam(NULL, 'WSDL');
      file_put_contents($this->tempfile, $wsdl);

      // Construct the soap server
      $this->soapServer = new SoapServer($this->tempfile, array('encoding'=>'UTF-8'));
      $this->soapServer->setClass($this->uriValues[0]);
    } else {
      throw new fHTTPException(404, 'Soap Server Class ' . $this->uriValues[0] . ' not found.');
    }
  }

  function execute()
  {
    $this->soapServer->handle($this->request);
    unlink($this->tempfile);
    return true;
  }

}
?>
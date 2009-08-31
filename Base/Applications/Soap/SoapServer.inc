<?php
/**
 * Soap Base Object
 *
 * Instances soap handler objects in Includes/Library/Soap
 *
 * @since      2005-07-12
 * @author     Gavin M. Roy <gavinmroy@gmail.com>
 * @link       http://framewerk.org
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2005-2009 the Framewerk Development Group
 * @package    Applications
 * @subpackage Soap
 * @uses       fApplication
 */

class SoapServer extends fApplication
{
  private $request, $soapServer, $tempfile;

  function __construct( )
  {
    // Make sure we have the soap extension loaded
    if ( !in_array( "soap", get_loaded_extensions( ) ) )
    {
      throw new fHTTPException(501, "This installation of PHP does not support SOAP.");
    }
    
    parent::__construct( );

    // Set an ini variable so it doesnt cash the wsdl information
    ini_set( "soap.wsdl_cache_enabled", "0" );

    // Set Our XML Parameters
    $this->fOutput->enabled = false;

    // Change config parameters
    $this->fMain->configuration->debug    = 0;
    $this->fMain->configuration->warnings = false;
    $this->fMain->configuration->profile  = false;

    // If we have a class that was asked for, use it
    $className = $this->uriValues[0];

    // Put our request data in a buffer due to a bug in php5.2
    $this->request = file_get_contents( 'php://input' );

    // Check to make sure that the soapApplication variable is set to true
    // This is set by extending fSoapApplicaiton
    $vars = get_class_vars($className);
    if ( $vars['soapApplication'] == true )
    {
    
      // Get the WSDL
      $temp = new WSDLGenerator( );
      $wsdl = $temp->build( $this->uriValues[0] );
      $this->tempfile = tempnam( NULL, 'WSDL' );
      file_put_contents( $this->tempfile, $wsdl );

      // Construct the soap server
      $this->soapServer = new SoapServer( $this->tempfile, 
                                          array( 'encoding' => 'UTF-8' )
                                        );
      $this->soapServer->setClass($this->uriValues[0]);
    } else {
      throw new fHTTPException(404, 'Soap Server Class ' . $this->uriValues[0] . ' not found.');
    }
  }

  function execute( )
  {
    $this->soapServer->handle( $this->request );
    unlink( $this->tempfile );
    return true;
  }

}
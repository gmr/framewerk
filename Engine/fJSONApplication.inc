<?php

/**
 * JSON Application abstract class
 *
 * This abstract fJSONApplication class is inherited by all Framewerk JSON Applications
 * Applications in Framewerk are responsible for the direct interaction with the user.
 *
 * @author     Dallas Gutauckis <dgutauckis@myyearbook.com>
 * @since      2007-08-15
 * @package    Engine
 * @subpackage Applications
 * @abstract
 */

abstract class fJSONApplication extends fApplication
{
  protected $object;
  protected $doDebug = false;
  protected $JSONPCallback = NULL;
  protected $outputSent = false;

  /**
   * Object Constructor
   *
   * Sets our pointer variables, sets default values, and sets the output parameters for a JSON file.
   */
  function __construct( $mimeType = 'application/json' )
  {
    parent::__construct();

    // Set Our JSON Parameters
    if ( $mimeType !== false )
    {
      $this->fOutput->setMimeType( $mimeType );
      $this->fTheme->useTheme( false );
      $this->fOutput->engineInfo = false;
    }

    $this->object = new stdClass;

    if ( $this->fSession->mybData['SESS_USERTYPE'] == 3 || $this->fSession->mybData['SESS_USERTYPE'] == 2 )
    {
      $this->doDebug = true;
    }
  }
  
  /**
   * Get or set the JSONP callback function name
   */
  function paddingCallback( $JSONPCallback = NULL )
  {
    $ret = $this->JSONPCallback;
    
    if ( $JSONPCallback === false )
    {
      $this->JSONPCallback = NULL;
      $this->fOutput->setMimeType( 'application/json' );
      return $ret;
    }
    elseif ( $JSONPCallback !== NULL )
    {
      if ( false === ereg( '^[a-zA-Z][.a-zA-Z0-9]*$', $JSONPCallback ) )
      {
        throw new Exception( 'Security exception!' );
      }
      // Need to re-set content-type to application/javascript for JSON-with-Padding to work
      $this->fOutput->setMimeType( 'application/javascript' );
      $this->JSONPCallback = $JSONPCallback;
    }
    return $ret;
  }

  function pushOutput($key, $value = NULL )
  {
    if ( $value === NULL && ( is_array( $key ) || is_object( $key ) ) )
    {
      foreach( $key as $label => $value )
      {
        $this->pushOutput( $label, $value );
      }
      return;
    }
    $this->object->$key = $value;
  }

  function output( $close = false )
  {
    if ( $this->outputSent === true )
    {
      fWarn::getInstance()->add('Trying to send to browser when output is already sent!');
      return false;
    }
    
    $debug = false;
    if ( $this->doDebug === true )
    {
      $this->object->debug = fEngineInformation::debugUserOutput(true, false);
    }
    
    $out = $json = json_encode($this->object);
    
    if ( $close === true )
    {
      header( 'Connection: close' );
    }
    if ( $this->JSONPCallback !== NULL )
    {
      $out = $this->JSONPCallback . '(' . $json . ');';
      // If we have debug data, try to call mybDebug separately from data output, since this is a JSONP call, and won't be handled automatically by the plugin
      if ( $this->object->debug )
      {
        $out .= 'if(jQuery&&jQuery.mybDebug)jQuery.mybDebug(' . $json . ');';
      }
    }
    $this->outputSent = true;
    header( 'Content-length: ' . strlen($out) );
    return parent::output($out);
  }

}
?>

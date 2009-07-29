<?php

/**
 * tStompConnection Class. Extends StompConnection to build instances based on the give fRegistry entry
 * 
 * @author Dallas Gutuackis <dgutauckis@myyearbook.com>
 * @since 2008-06-18 16:54:00 EST
 */
class tStompConnection extends StompConnection 
{
  /**
   * fRegistry instance
   *
   * @var fRegistry
   */
  protected $fRegistry;
  /**
   * Array of object instances
   *
   * @var array
   */
  static protected $instances = array();

  /**
   * Factory getInstance function
   *
   * @param string $instanceName
   * @return StompConnection
   */
  static public function getInstance( $instanceName )
  {
    if ( false === ( self::$instances[$instanceName] instanceof tStompConnection ) )
    {
      self::$instances[$instanceName] = new tStompConnection( $instanceName );
    }
    return self::$instances[$instanceName];
  }
  
  /**
   * Class constructor
   *
   * @param string $instanceName Name of the instance to use for our connection
   */
  function __construct( $instanceName )
  {
    // Get the brokerURI from fRegistry
    $this->fRegistry = new fRegistry( 'StompConnection' );
    $instances = $this->fRegistry->instance;
    
    foreach ( $instances as $instance )
    {
      if ( $instance->name == $instanceName )
      {
        $uris = array();
        foreach ( $instance->server as $server )
        {
          $port = ( $server->port ) ? ( $server->port ) : ( StompConnection::DEFAULT_PORT );
          if ( ! isset( $server->ip ) )
          {
            throw new Exception( 'All StompConnection instance server definitions must have at least an ip defined.');
          }
          $uri = 'tcp://' . $server->ip . ':' . $port;
                   
          $uris[] = $uri;
          
        }
        
        // Determine what URI to build
        $uriCount = count( $uris );
        if ( $uriCount > 1 )
        {
          $uri = 'failover:(' . implode( ',', $uris );
        } elseif ( $uriCount == 1 ) {
          $uri = $uris[0];
        } else {
          throw new Exception( 'All StompConnection instance server definitions must have at least one server defined.' );
        }
        
        if ( $instance->queryString )
        {
          $uri .= '?' . $instance->queryString; 
        }
        
        break;
      }
    }
    
    // Make sure we found the instance!
    if ( ! isset( $uri ) )
    {
      throw new Exception( 'Instance "' . $instanceName . '" not found in fRegistry.' );
    }

    // Construct our parent class with the URI
    parent::__construct( $uri );
  }
  
  /**
   * Auto-serializes input, then sends to the queue
   *
   * @param string $queue
   * @param any $item
   */
  function sendSerialized( $queue, $item )
  {
    $item = serialize( $item );
    $this->send( $queue, $item );
  }
  
  /**
   * Class destructor. Performs cleanup of connection
   *
   * @author Dallas Gutauckis <dgutauckis@myyearbook.com>
   * @since 2008-06-18 16:16:00 EST
   */
  function __destruct()
  {
    $this->disconnect();
  }
}

?>
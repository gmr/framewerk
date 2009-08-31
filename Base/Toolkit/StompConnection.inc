<?php

/**
 * StompFrames are messages that are sent and received on a StompConnection.
 *
 * @package Stomp
 * @author Hiram Chirino <hiram@hiramchirino.com>
 * @author Dejan Bosanac <dejan@nighttale.net>
 * @author Joe Hansche <jhansche@myyearbook.com> - ported to PHP5 and sockets extension
 * @version $Revision$
 */
class StompFrame
{
  public $command;
  public $headers = array();
  public $body;

  function __construct ($command = null, $headers = null, $body = null)
  {
    $this->init($command, $headers, $body);
  }

  function init ($command = null, $headers = null, $body = null)
  {
    $this->command = $command;
    if ( $headers != null )
      $this->headers = $headers;
    $this->body = $body;
  }
}

/**
 * Basic text stomp message
 *
 * @package Stomp
 * @author Dejan Bosanac <dejan@nighttale.net>
 * @version $Revision$
 */
class StompMessage extends StompFrame
{
  function __construct($body, $headers = null)
  {
    $this->init('SEND', $headers, $body);
  }
}

/**
 * Message that contains a stream of uninterpreted bytes
 * 
 * @package Stomp
 * @author Dejan Bosanac <dejan@nighttale.net>
 */
class BytesMessage extends StompMessage
{
  function __construct ($body, $headers = null)
  {
    $this->init('SEND', $headers, $body);
    if ( $this->headers == null )
    {
      $this->headers = array();
    }
    $this->headers['content-length'] = count($body);
  }
}

/**
 * Message that contains a set of name-value pairs
 *
 * @package Stomp
 * @author Dejan Bosanac <dejan@nighttale.net>
 */
class MapMessage extends StompMessage
{
  public $map;

  function __construct ($msg, $headers = null)
  {
    if ( $msg instanceOf StompFrame )
    {
      $this->init($msg->command, $msg->headers, $msg->body);
      $this->map = json_decode($msg->body);
    } else
    {
      $this->init('SEND', $headers, $msg);
      if ( $this->headers == null )
      {
        $this->headers = array();
      }
      $this->headers['amq-msg-type'] = 'MapMessage';
      $this->body = json_encode($msg);
    }
  }
}

/**
 * A Stomp Connection
 *
 * @package Stomp
 * @author Hiram Chirino <hiram@hiramchirino.com>
 * @author Dejan Bosanac <dejan@nighttale.net>
 * @author Joe Hansche <jhansche@myyearbook.com> - Sockets Extension 
 * @version $Revision$
 */
class StompConnection
{
  const DEFAULT_PORT = 61613;
  const NOOP = "\x00\n";
  
  protected $socket = null;
  protected $hosts = array();
  protected $params = array();
  protected $subscriptions = array();
  protected $currentHost = -1;
  protected $attempts = 10;
  protected $username = '';
  protected $password = '';

  function __construct($brokerUri)
  {
    $uri = parse_url($brokerUri);

    // Failover takes a format of:  failover:(tcp://host1:port1,tcp://host2:port2)?params=vals
    if ( $uri['scheme'] === 'failover' )
    {
      $urls = explode(',', trim($uri['path'], '()'));
      foreach( $urls as $url )
      {
        $tempuri = parse_url($url);
        if ( ! isset( $tempuri['port'] ) ) 
        {
          $tempuri['port'] = self::DEFAULT_PORT;
        }
        $this->hosts[] = array( $tempuri['host'], $tempuri['port']);
      }
      unset($uri['path'], $uri['scheme']);
    }
    // Non-failover format is:  tcp://host:port?params=vals
    else
    {
      if ( ! isset($uri['port']) )
      {
        $uri['port'] = self::DEFAULT_PORT;
      }
      $this->hosts[] = array( $uri['host'], $uri['port'] );
      unset($uri['host'], $uri['scheme']);
    }
    
    if ( isset($uri['user']) )
    {
      $this->username = $uri['user'];
    }
    if ( isset($uri['pass']) )
    {
      $this->password = $uri['pass'];
    }
    
    // Parse the query string as parameters
    if ( isset($uri['query']) )
    {
      parse_str( $uri['query'], $this->params );
    }
    
    if ( ! is_numeric( $this->params['connectionTimeout'] ) )
    {
      // 30 second connection timeout
      $this->params['connectionTimeout'] = 30000;
    }
    
    if ( ! is_numeric( $this->params['soTimeout'] ) )
    {
      // No socket timeout (wait forever)
      $this->params['soTimeout'] = 0;
    }
    
    if ( ! is_numeric( $this->params['socketBufferSize']) )
    {
      // 64KB
      $this->params['socketBufferSize'] = 65536;
    }
    
    $this->makeConnection();
    $this->connect( $this->username, $this->password );
  }

  /**
   * Builds the socket and connection to the server
   *
   * @return boolean
   */
  function makeConnection()
  {
    if ( count($this->hosts) == 0 )
    {
      trigger_error('No broker defined', E_USER_ERROR);
      return false;
    }
    
    $i = $this->currentHost;
    $attempt = 0;
    $connected = false;
    $numHosts = count($this->hosts);
    
    while ( false === $connected && ++$attempt <= $this->attempts )
    {
      if ( $numHosts > 1 && isset( $this->params['randomize'] ) && (bool)($this->params['randomize']) === true )
      {
        $i = rand(0, $numHosts - 1);
      } else
      {
        $i = ($i + 1) % $numHosts;
      }
      
      $broker = $this->hosts[$i];
      
      list( $host, $port ) = $broker;
      
      if ( ! is_numeric( $port ) )
      {
        $port = self::DEFAULT_PORT;
      }
      
      if ( $this->socket !== NULL )
      {
        socket_close($this->socket);
        $this->socket = NULL;
      }
      
      $this->socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
      
      if ( $this->params['socketBufferSize'] > 0 )
      {
        socket_set_option($this->socket, SOL_SOCKET, SO_SNDBUF, $this->params['socketBufferSize']);
        socket_set_option($this->socket, SOL_SOCKET, SO_RCVBUF, $this->params['socketBufferSize']);
      }
      
      if ( $this->params['soTimeout'] > 0 )
      {
        stream_set_timeout($this->socket, (int)($this->params['soTimeout'] / 1000), (int)($this->params['soTimeout'] % 1000) * 1000 );
        socket_set_option($this->socket, SOL_SOCKET, SO_SNDTIMEO, $this->params['soTimeout']);
        socket_set_option($this->socket, SOL_SOCKET, SO_RCVTIMEO, $this->params['soTimeout']);
      }
      
      if ( $this->params['connectionTimeout'] > 0 )
      {
        // Use socket_select() on a non-blocking socket to see if it connected successfully
        socket_set_nonblock( $this->socket );
        socket_connect( $this->socket, $host, $port);
        socket_set_block( $this->socket );
        // socket_select() takes a timeout, so use that for the connection timeout
        $tempSelect = socket_select( $r = array($this->socket), $w = array($this->socket), $f = array($this->socket), (int)($this->params['connectionTimeout'] / 1000), (int)($this->params['connectionTimeout'] % 1000) * 1000 );
        
        switch ( $tempSelect )
        {
          case 2:
            throw new Exception('AMQ Connection refused.  Is the service running?');
            break;
          case 0:
            throw new Exception('AMQ Connection timed out.  Possibly network problems?');
            break;
          case 1:
            $result = true;
            break;
        }
      }
      else
      {
        // For no timeout, just use socket_connect()
        $result = socket_connect( $this->socket, $host, $port);
      }
      
       
      if ( false === $this->socket || $result !== true )
      {
        trigger_error('Could not connect to '.$host.':'.$port.' (Try #'.$attempt.'/'.$this->attempts.')', E_USER_WARNING);
      } else
      {
        $connected = true;
        $this->currentHost = $i;
        break;
      }
    
    }
    
    if ( $connected === false )
    {
      socket_shutdown( $this->socket, 2 );
      socket_close( $this->socket );
      $this->socket = NULL;
      trigger_error('Could not connect to a broker', E_USER_ERROR);
      return false;
    }
    return $connected;
  }

  /**
   * Connect to the amq server
   *
   * @param string $username
   * @param string $password
   * @return StompFrame
   */
  protected function connect ($username = '', $password = '')
  {
    if ( $username !== '' )
      $this->username = $username;
    
    if ( $password !== '' )
      $this->password = $password;
    
    $this->writeFrame(new StompFrame('CONNECT', array('login' => $this->username, 'passcode' => $this->password)));
    return $this->readFrame();
  }

  /**
   * Send a message to a queue
   *
   * @param string $destination
   * @param mixed $msg String body, or StompFrame object
   * @param array $properties
   */
  function send ($destination, $msg, $properties = null)
  {
    if ( $msg instanceOf StompFrame )
    {
      $msg->headers['destination'] = $destination;
      $this->writeFrame($msg);
    } else
    {
      $headers = array();
      if ( isset($properties) )
      {
        foreach ( $properties as $name => $value )
        {
          $headers[$name] = $value;
        }
      }
      $headers['destination'] = $destination;
      $this->writeFrame(new StompFrame('SEND', $headers, $msg));
    }
  }

  /**
   * Register a subscription to a queue
   *
   * @param string $destination Queue name
   * @param array $properties
   */
  function subscribe ($destination, $properties = null)
  {
    $headers = array('ack' => 'client');
    if ( isset($properties) )
    {
      foreach ( $properties as $name => $value )
      {
        $headers[$name] = $value;
      }
    }
    $headers['destination'] = $destination;
    $this->writeFrame(new StompFrame('SUBSCRIBE', $headers));
    $this->subscriptions[$destination] = $properties;
  }

  /**
   * Unsubscribe from a queue destination
   *
   * @param string $destination Queue name
   * @param array $properties
   */
  function unsubscribe ($destination, $properties = null)
  {
    $headers = array();
    if ( isset($properties) )
    {
      foreach ( $properties as $name => $value )
      {
        $headers[$name] = $value;
      }
    }
    $headers['destination'] = $destination;
    $this->writeFrame(new StompFrame('UNSUBSCRIBE', $headers));
    unset($this->subscriptions[$destination]);
  }

  /**
   * Begin a Stomp/AMQ transaction
   *
   * @param string $transactionId
   */
  function begin ($transactionId = null)
  {
    $headers = array();
    if ( isset($transactionId) )
    {
      $headers['transaction'] = $transactionId;
    }
    $this->writeFrame(new StompFrame('BEGIN', $headers));
  }

  /**
   * Commit a pending AMQ transaction
   *
   * @param string $transactionId
   */
  function commit ($transactionId = null)
  {
    $headers = array();
    if ( isset($transactionId) )
    {
      $headers['transaction'] = $transactionId;
    }
    $this->writeFrame(new StompFrame('COMMIT', $headers));
  }

  /**
   * Abort a pending AMQ transaction
   *
   * @param string $transactionId
   */
  function abort ($transactionId = null)
  {
    $headers = array();
    if ( isset($transactionId) )
    {
      $headers['transaction'] = $transactionId;
    }
    $this->writeFrame(new StompFrame('ABORT', $headers));
  }

  /**
   * Acknowledge a pending AMQ message
   *
   * @param string $message amq "message-id" header being acknowledged
   * @param string $transactionId
   */
  function ack ($message, $transactionId = null)
  {
    if ( $message instanceOf StompFrame )
    {
      $this->writeFrame(new StompFrame('ACK', $message->headers));
    } else
    {
      $headers = array();
      if ( isset($transactionId) )
      {
        $headers['transaction'] = $transactionId;
      }
      $headers['message-id'] = $message;
      $this->writeFrame(new StompFrame('ACK', $headers));
    }
  }

  /**
   * Disconnect from AMQ server
   */
  function disconnect ()
  {
    if ( $this->socket )
    {
      $this->writeFrame(new StompFrame('DISCONNECT'));
    }
    socket_shutdown($this->socket, 1);
    usleep(500);
    socket_shutdown($this->socket, 2);
    socket_close($this->socket);
    $this->socket = NULL;
  }

  /**
   * Write a frame
   *
   * @param StompFrame $stompFrame
   */
  protected function writeFrame ($stompFrame)
  {
    $data = $stompFrame->command . "\n";
    if ( isset($stompFrame->headers) )
    {
      foreach ( $stompFrame->headers as $name => $value )
      {
        $data .= $name . ': ' . $value . "\n";
      }
    }
    $data .= "\n";
    if ( isset($stompFrame->body) )
    {
      $data .= $stompFrame->body;
    }
    $data .= self::NOOP;
    
    socket_write($this->socket, self::NOOP, strlen(self::NOOP));
    
    $r = socket_write($this->socket, $data, strlen($data));
    
    if ( $r === false || $r == 0 )
    {
      trigger_error('Could not send stomp frame to server');
      $this->reconnect();
      
      $this->writeFrame($stompFrame);
    }
  
  }

  /**
   * Read a StompFrame from the queue
   *
   * @return StompFrame
   */
  function readFrame ()
  {
    $start = microtime( true );
    $data = $buf = '';
    
    $rc = socket_recv($this->socket, $buf, 1, 0);
    // rc === 0: EOF / FIN received
    // rc === false: RST received / connection forcefully closed by remote side
    
    if ( $rc === 0 || $rc === false )
    {
      // EOF / FIN recvd
      $this->reconnect();
      return $this->readFrame();
    }
    
    // Read until end of frame (\0)
    while ( ord($buf) != 0 )
    {
      $data .= $buf;
      
      $rc = socket_recv($this->socket, $buf, 1, 0);
      
      if ( $rc === false || $rc === 0 )
      {
        $this->reconnect();
        return $this->readFrame();
      }
    }
    
    // Verify next byte is \n
    $rc = socket_recv($this->socket, $buf, 1, 0);
    if ( $rc === false || $rc === 0 || ord($buf) != 10 )
    {
      $this->reconnect();
      return $this->readFrame();
    }
    
    list ($header, $body) = explode("\n\n", $data, 2);
    $header = explode("\n", $header);
    $headers = array();
    
    $command = NULL;
    foreach ( $header as $v )
    {
      if ( isset($command) )
      {
        list ($name, $value) = explode(':', $v, 2);
        $headers[$name] = trim($value);
      } else
      {
        $command = $v;
      }
    }
    
    $frame = new StompFrame($command, $headers, trim($body));
    if ( isset($frame->headers['amq-msg-type']) && $frame->headers['amq-msg-type'] == 'MapMessage' )
    {
      return new MapMessage($frame);
    } else {
      return $frame;
    }
  }

  /**
   * Reconnects and renews subscriptions (if there were any)
   * Call this method when you detect connection problems     
   */
  function reconnect ()
  {
    socket_shutdown( $this->socket, 1 );
    socket_shutdown( $this->socket, 2 );
    socket_close( $this->socket );
    $this->socket = NULL;
    $this->makeConnection();
    $this->connect();
    foreach ( $this->subscriptions as $dest => $properties )
    {
      $this->subscribe($dest, $properties);
    }
  }
}
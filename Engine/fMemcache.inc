<?php // $Id:fSHMOPCache.inc 876 2007-06-13 16:22:22Z gmr $
############################################################################
#    Copyright (C) 2007 by the Framewerk Development Group                 #
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
 * Memcache Class
 *
 * This object allows classes to use php's memcache class with some default behaviors
 *
 * @author     Gavin M. Roy <gavinmroy@gmail.com>
 * @link       http://framewerk.org
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2007 the Framewerk Development Group
 * @version    $Rev:876 $
 * @package    Toolkit
 * @subpackage Caching
 * @since      2007-07-03
 */

class fMemcache extends fObject
{
  protected $memcache, $prefix;
  public $compress = NULL, $defaultExpiration = 86400;
  static protected $usageStats = array( 'sets' => 0, 'hits' => array(),
     'misses' => array(), 'repeats' => array(), 'adds' => 0, 'deletes' => array(), 'decrements' => 0,
     'increments' => 0, 'time' => 0, 'instances' => 0, 'connectedInstances' => 0,
     'connections' => 0, 'replaces' => 0 );

  static protected $memcacheInstances = array();
  static protected $registryInstances;
  static protected $registry;
  static protected $localCache = array();

  static protected function getMemcacheInstance( $id = NULL )
  {
    if ( false === array_key_exists( $id, self::$memcacheInstances) )
    {
      $registry = self::$registry;

      $fDebug = fDebug::getInstance();

      $memcache = new Memcache;
      if ( !method_exists( $memcache, 'set' ) )
      {
        throw new Exception("Memcache is being requested but PECL extension isn't available");
      }

      $connected = false;
      foreach ($registry->instance AS $instance )
      {
        if ( strval($instance->id) == $id )
        {

          self::$registryInstances[$id] = $instance;

          // Loop through the servers for this instance
          foreach ( $instance->server AS $server )
          {
            // We must have at least a server IP.
            if ( strlen(trim($server->ip)) == 0 )
            {
              throw new Exception('Empty server IP in //fMemcache/instance/server/ip');
            }

            // If we set a port, use it, otherwise use the default
            $port = isset($server->port) ? $server->port : 11211;

            // If we set a timeout, use it, otherwise use the default
            $timeout = isset($server->timeout) ? $server->timeout : 1;

            // If we set a pconnect, use it, otherwise use connect
            $pconnect = isset($server->pconnect) ? $server->pconnect : 0;

            // If we set a retryInterval use it, otherwise the default of 15
            $retryInterval = isset($server->retryInterval) ? $server->retryInterval : 15;

            // We allow pools or single connections
            if ( $instance->pool == true )
            {
              if ( intval($server->weight) == 0 )
              {
                // As of PHP >=5.2.5
                throw new Exception('Server pool server entry with weight of 0 is invalid, due to buggy memcache::addServer!');
              }
              $fDebug->add('Adding memcached to the pool: ' . $server->ip . ':' . $port, 4, 'fMemcache');
              $result = $memcache->addServer(strval($server->ip), intval($port), intval($pconnect),
                                                   intval($server->weight), intval($timeout), intval($retryInterval),
                                                   intval($server->status));
            } else {
              // Otherwise we have aa single connection.  Use pconnect or connect depending on configuration
              if ( $server->pconnect == true )
              {
                $fDebug->add('PConnect to memcached: ' . $server->ip . ':' . $port, 4, 'fMemcache');
                $result = $memcache->pconnect(strval($server->ip), intval($port), intval($timeout));
              } else {
                $fDebug->add('Connect to memcached: ' . $server->ip . ':' . $port, 4, 'fMemcache');
                $result = $memcache->connect(strval($server->ip), intval($port), intval($timeout));
              }
            }

            // if we have a true result, we've connected, even if we have failures in the pool (i.e. don't assign connected directly from result)
            if ( $result == true )
            {
              self::$usageStats['connections'] += 1;
              $connected = true;
            }
          }

          self::$usageStats['connectedInstances'] += 1;
          self::$memcacheInstances[$id] = $memcache;
          // We hit our instance end so no need to keep looping
          break;
        }
      }
      // If we didn't connect to any server, throw an exception
      if ( $connected == false )
        throw new Exception("fMemcache created but did not connect to a memcache daemon");
    }

    return self::$memcacheInstances[$id];
  }

  function __construct($id = NULL, $prefix = NULL)
  {
    $begin = microtime( true );

    if ( $id === NULL )
      throw new Exception('fMemcache object created without specifying an instance id.');

    // Assign our internal Framewerk variable
    $this->fDebug = fDebug::getInstance();
    $this->fProfile = fProfile::getInstance( );

    if ( NULL === self::$registry )
    {
      self::$registry = new fRegistry( 'fMemcache' );
    }

    // Add our servers
    $registry = self::$registry;

    if ( $registry->enabled != true )
      throw new Exception('fMemcache is not enabled but was requested.');

    // If we passed in an object for the prefix, get the class name
    if ( is_object($prefix) )
      $prefix = get_class($prefix);

    // Create our memcache object
    $this->memcache = self::getMemcacheInstance($id);

    // If prefix is set in the instance, use it
    if ( $prefix != '' )
    {
      $this->prefix = $prefix . ':';
    }
    elseif ( strval(self::$registryInstances[$id]->prefix) != '' )
    {
      $this->prefix = self::$registryInstances[$id]->prefix . ':';
    }
    else
    {
      $this->prefix = '';
    }

    self::$usageStats['instances'] += 1;
    self::$usageStats['time'] += round(microtime(true) - $begin, 3);
  }

  /**
   * Add an item to the cache auto-serializing objects and arrays
   *
   * @param unknown_type $key
   * @param unknown_type $value
   * @param unknown_type $expire
   * @return unknown
   */
  function add($key, $value, $expire = NULL)
  {
    $begin = microtime( true );
    self::$usageStats['add'] += 1;
    // Set the default expiration if it is not set
    if ( $expire === NULL )
      $expire = $this->defaultExpiration;

    // If it's an object or array serialize it
    if ( ( is_object($value) ) ||(  is_array($value) ) )
      $value = serialize($value);

    $this->fDebug->add('Adding ' . $this->prefix . $key, 2, 'fMemcache');
    $result = $this->memcache->add($this->prefix . $key, $value, $this->compress, $expire);
    self::$usageStats['time'] += round( microtime( true ) - $begin, 3);
    return $result;
  }

  /**
   * Decrement using replace so we can extend the expiration
   *
   * @param string $key
   * @param int $value
   * @param int $expire
   * @return bool
   */
  function decrement($key, $value = 1)
  {
    $begin = microtime( true );
    self::$usageStats['decrements'] += 1;

    $key = $this->prefix . $key;

    // Add a debug statement
    $this->fDebug->add('Decrementing ' . $key, 2, 'fMemcache');

    // Directly return the result of the decrement function
    $new = $this->memcache->decrement($key, $value);
    if ( false !== $new )
    {
      self::$localCache[$key] = $new;
    }

    self::$usageStats['time'] += round( microtime( true ) - $begin, 3);
    return $new;
  }

  /**
   * Delete a memcache key entry
   *
   * @param string $key
   */
  function delete($key)
  {
    $begin = microtime( true );

    // Prepend prefix to key
    $key = $this->prefix . $key;

    self::$usageStats['deletes'][] = $key;
    $this->fDebug->add('Deleting ' . $key, 2, 'fMemcache');
    $result = $this->memcache->delete($key);

    if ( false !== $result )
    {
    	unset( self::$localCache[$key] );
    }

    self::$usageStats['time'] += round( microtime( true ) - $begin, 3);
    return $result;
  }
  /**
   * Turn off compression
   */
  function disableCompression()
  {
    $this->compress = NULL;
  }

  /**
   * Turn on compression passing in an optional threshold of minimum bytes to compress and a minimum savings to achieve before storage
   *
   * @param int $threshold
   * @param float $minSavings
   * @return bool
   */
  function enableCompression($threshold = 2048, $minSavings = 0.2)
  {
    // If zlib is available we can compress
    $extensions = get_loaded_extensions();
    if ( in_array('zlib', $extensions) )
    {
      // Set the compress value to what Memcached extension expects
      $this->compress = MEMCACHE_COMPRESSED;

      // Add a debug message
      $this->fDebug->add('Turning on zlib compressed memcached storage with threshold of ' . $threshold . ' and min savings of '  , ($minSavings * 100) . '%', 8, 'fMemcache');

      // Set the compression thresholds
      $this->memcache->setCompressThreshold($threshold, $minSavings);

      // Let the calling function know we were able to do so
      return true;
    } else {
      // Set our compress variable to null
      $this->compress = NULL;

      // Add a debug message
      $this->fDebug->add('zlib compressed memcached storage unavailable', 8, 'fMemcache');

      // Return false since we cant compress
      return false;
    }
  }

  /**
   * Expire all the keys in memcached
   *
   * @return bool
   */
  function flush()
  {
    // Add a debug statement
    $this->fDebug->add('Expiring all items in memcached', 8, 'fMemcache');

    // Return the direct result of the flush() call
    return $this->memcache->flush();
  }

  /**
   * Get the value of a key, auto-unserializing arrays and objects
   *
   * @param string $key
   * @return any
   */
  function get($key, $usePrefix = true )
  {
    $begin = microtime( true );

    if ( $this->prefix === NULL )
    {
      $usePrefix = false;
    }

    if ( is_array( $key ) )
    {
      $arrayKeys = array_values($key);
      if ( $usePrefix === true )
      {
        foreach ( $arrayKeys AS $i => $key )
        {
          $arrayKeys[$i] = $this->prefix . $key;
          $this->fDebug->add('Getting value for ' . $key, 2, 'fMemcache');
        }
      }

      $value = $this->memcache->get( $arrayKeys );

      $hits = count( $value );
      foreach ( array_keys($hits) as $tempHit )
      {
        self::$usageStats['hits'][] = $tempHit;
      }
      foreach ( $key as $tempKey )
      {
        if ( isset($hits[$tempKey]) === false )
        {
          self::$usageStats['misses'][] = $tempKey;
        }
      }
    } else {

      if ( $usePrefix === true )
      {
        $key = $this->prefix . $key;
      }

      $value = $this->_get( $key );

      if( $value === false )
      {
        self::$usageStats['misses'][] = $key;
      }
      else
      {
        self::$usageStats['hits'][] = $key;
      }
    }

    // We have a normal value to return
    self::$usageStats['time'] += round( microtime( true ) - $begin, 3);
    return $value;
  }

  /**
   * This function performs the localCache || memcache get, based on whether or not we have the value
   *
   * @param string $key
   * @return mixed
   */
  function _get( $key )
  {
    if ( false === array_key_exists( $key, self::$localCache ) )
    {
    	self::$localCache[$key] = $this->memcache->get( $key );
    } else {
    	self::$usageStats['repeats'][] = $key;
    }
    return self::$localCache[$key];
  }

  /**
   * Return an array of stats for all servers
   *
   * @return array
   */
  function getExtendedStats()
  {
    // Add a debug statement
    $this->fDebug->add('Getting stats for all memcached servers', 8, 'fMemcache');

    // Directly return the result of getExetendedStats()
    return $this->memcache->getExtendedStats();
  }

  /**
   * Get memcached stats
   *
   * @return array
   */
  function getStats()
  {
    // Add a debug statement
    $this->fDebug->add('Getting stats for primary memcached server', 8, 'fMemcache');

    // directly return the value of getStats()
    return $this->memcache->getStats();
  }

  /**
   * Get fMemcache usage stats, including gets, sets, misses, etc.
   *
   */
  static function getUsageStats()
  {
    if ( self::$usageStats['hits'] + self::$usageStats['misses'] > 0 )
    {
      self::$usageStats['effectiveness'] = number_format(100 * count(self::$usageStats['hits']) / (count(self::$usageStats['hits']) + count(self::$usageStats['misses'])), 1) . '%';
    } else {
      self::$usageStats['effectiveness'] = 0;
    }
    return self::$usageStats;
  }

  /**
   * Return the memcached version
   *
   * @return string
   */
  function getVersion()
  {
    // Add a debug statement
    $this->fDebug->add('Getting version for primary memcached server', 8, 'fMemcache');

    // Directly return the result of getVersion()
    return $this->memcache->getVersion();
  }

  /**
   * Increment a value by
   *
   * @param string $key
   * @param int $value
   * @param int $expire default of one hour
   * @return unknown
   */
  function increment($key, $value = 1)
  {
    $begin = microtime( true );
    self::$usageStats['increments'] += 1;

    // Prepend prefix to key
    $key = $this->prefix . $key;

    // Add a debugging message
    $this->fDebug->add('Incrementing ' . $key, 2, 'fMemcache');

    $new = $this->memcache->increment($key, $value);

    if ( $new !== false )
    {
    	self::$localCache[$key] = $new;
    }

    self::$usageStats['time'] += round( microtime( true ) - $begin, 3);
    return $new;
  }

  /**
   * Replace data at the server, auto-serializing objects and arrays
   *
   * @param string $key
   * @param any $value
   * @param int $expire default of one hour
   * @return bool
   */
  function replace($key, $value, $expire = NULL)
  {
    $begin = microtime( true );
    self::$usageStats['replaces'] += 1;

    // Set the default expiration if expiration is not set
    if ( $expire === NULL )
    {
      $expire = $this->defaultExpiration;
    }

    // Prepend prefix to the key
    $key = $this->prefix . $key;

    // Add a debug message
    $this->fDebug->add('Replacing ' . $key, 2, 'fMemcache');

    // Replace and return the result bool
    $result = $this->memcache->replace($key, $value, $this->compress, $expire);

    if ( false !== $result )
    {
    	self::$localCache[$key] = $value;
    }

    self::$usageStats['time'] += round( microtime( true ) - $begin, 3);
    return $result;
  }

  /**
   * Store data at the server, auto-serializing arrays and objects
   *
   * @param string $key
   * @param any $value
   * @param int $expire default of one hour
   * @return bool
   */
  function set($key, $value, $expire = NULL)
  {
    $begin = microtime( true );
    self::$usageStats['sets'] += 1;

    // Set the default expiration if it is not set
    if ( $expire === NULL )
    {
      $expire = $this->defaultExpiration;
    }

    // Prepend key prefix
    $key = $this->prefix . $key;

    // Add a debug message
    $this->fDebug->add('Setting ' . $key, 2, 'fMemcache');

    // Set and return the result bool
    $result = $this->memcache->set($key, $value, $this->compress, $expire);

    if ( false !== $result )
    {
    	self::$localCache[$key] = $value;
    }

    self::$usageStats['time'] += round( microtime( true ) - $begin, 3);
    return $result;
  }

  /**
   * Set the default expiration value in seconds
   *
   * @param int $value
   * @return int
   */
  function setDefaultExpiration($value)
  {
    // Set the default expiration variable
    $this->defaultExpiration = intval($value);

    // Return what it was set to
    return intval($value);
  }

  /**
   * Changes server parameters and status at runtime
   *
   * @param string $host Point to the host where memcached is listening for connections.
   * @param int $port Point to the port where memcached is listening for connections. This parameter is optional and its default value is 11211.
   * @param int $timeout Value in seconds which will be used for connecting to the daemon.
   * @param int $retry_interval Controls how often a failed server will be retried, the default value is 15 seconds.
   * @param bool $status Controls if the server should be flagged as online.
   * @param function $failure_callback  Allows the user to specify a callback function to run upon encountering an error. The callback is run before failover is attemped. The function takes two parameters, the hostname and port of the failed server.
   * @return bool
   */
  function setServerParams($host, $port= NULL, $timeout = NULL, $retry_interval = NULL, $status = NULL, $failure_callback = NULL )
  {
    // Add a debug message
    $this->fDebug->add('Overwriting server settings for ' . $host, 8, 'fMemcache');

    // Call the setServerParams function and return the result bool
    return $this->memcache->setServerParams($host, $port, $timeout, $retry_interval, $status, $failure_callback);
  }

  /**
   * If there is a memcache pool and the pool encounters an error, throw a warning
   *
   * @param string $hostname
   * @param string $port
   */
  function poolFailureCallback($hostname, $port)
  {
    fWarn::getInstance()->add('fMemcache Pool Error - Host: ' . $hostname . ':' . $port);
  }

}
?>

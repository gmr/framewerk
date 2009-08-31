<?php

/**
 * A mutually exclusive lock helper class
 * Creates a mutex lock via memcache when instantiated, or throws an exception if it can't create the lock.
 * Lock will be released on destruction, but can be released manually as well
 * @since 2008-06-12
 */

class tMutex extends fObject
{
  protected $id;
  /**
   * @var fMemcache
   */
  protected $fMemcache;
  protected $locked = false;
  protected $maxLockDuration = 10;
  
  // Time to pause between checks, in microseconds (1,000,000us in 1 second).  50000 == 50ms
  const POLL_INTERVAL = 50000;
  
  /**
   * Constructor
   * @param string $key prefix
   * @param string $id ID specific to this lock
   * @param numeric $timeToWait if set, the time in seconds to wait for the lock to become available before failing; 
   *                            if not set, will fail immediately
   * @throws Exception when mutex lock is unavailable
   */
  function __construct( $key, $id, $timeToWait = NULL, $maxLockDuration = NULL )
  {
    $this->fMemcache = new fMemcache( 'data', 'mutex|' . $key );
    
    $this->id = $id;
    
    if ( $maxLockDuration !== NULL && is_numeric( $maxLockDuration ) )
    {
      $this->maxLockDuration = $maxLockDuration;
    }
    
    // check if we're allowed to wait for the lock
    if ( $timeToWait !== NULL )
    {
      $timeToWait = microtime( true ) + $timeToWait;

      // Keep checking as long as we don't have a lock yet, and the wait time hasn't expired
      while ( $this->locked === false && microtime( true ) < $timeToWait )
      {
        $this->locked = ( $this->fMemcache->add( $this->id , 1, $this->maxLockDuration ) !== false );
        
        // If this lock didn't succeed, pause for a bit, then loop to try again
        if ( $this->locked === false )
        {
          usleep( self::POLL_INTERVAL );
        }
      }
    }
    else
    {
      $this->locked = ( $this->fMemcache->add( $this->id , 1, $this->maxLockDuration ) !== false );
    }
    
    // Throw an exception if we weren't able to obtain the lock
    if ( $this->locked === false )
    {
      throw new Exception( 'Could not obtain a lock on ' . $key );
    }
  }
  
  /**
   * Manually releases the mutex lock
   * @return boolean True if previously held lock is successfully released, false if not already locked or if release fails
   */
  function release( )
  {
    if ( $this->locked === false )
    {
      return false;
    }
    $this->locked = ( $this->fMemcache->delete( $this->id ) !== true );
    return $this->locked === false;
  }
  
  /**
   * Destructor
   * Releases mutex lock, if currently held
   */
  function __destruct()
  {
    $this->release();
  }
}
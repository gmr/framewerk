<?php // $Id: fSessionStoreMemcache.inc 8486 2007-12-20 15:22:22Z jhansche $
############################################################################
#    Copyright (C) 2004-2007 by the Framewerk Development Group            #
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
 * Session Store memcached Object
 *
 * This object handles storing session data into memcache
 *
 * @author     Gavin M. Roy <gavinmroy@gmail.com>
 * @author     Joe Hansche <madcoder@gmail.com>
 * @link       http://framewerk.org
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2005-2007 the Framewerk Development Group
 * @since      2007-08-21
 * @version    $Rev: 8486 $
 * @package    Engine
 * @subpackage Core
 * @uses       iSessionStore
 * @uses       fMemcache
 */

class fSessionStoreMemcache implements iSessionStore
{
  protected $cache, $fDebug, $fMain;

  /**
   * Validate we can use SHMOPCache
   */
  function __construct()
  {
    // Make sure we have memcached support
    try
    {
      $this->cache = new fMemcache('session', 'fw');
    } Catch ( Exception $e) {
      throw $e;
    }

    /**
     * @todo Make this configurable
     */
    $this->cache->disableCompression();

    // Set our internal fMain pointer
    $this->fDebug = fDebug::getInstance();
    $this->fMain = fMain::getInstance();
    $this->fRegistry = new fRegistry( 'fSession' );

    if ( $this->fRegistry->visitorTime > 0 )
    {
      $this->cache->setDefaultExpiration($this->fRegistry->visitorTime);
    }
  }

  /**
   * Load the session data
   *
   * @param String ID
   * @return Array Session Data
   */
  function loadSession($id)
  {
    $this->fDebug->add("Loading session ID: ".$id.".", 6);

    // Set our ID Value
    $data = $this->cache->get($id);

    // If this is a new session, start by set()ing the data.
    if ( ! is_array($data) )
    {
      $this->cache->set($id, array());
      $data = array();
    }

    // Return our session data
    return $data;
  }

  /**
   * Prune old sessions out of the session table and remove their shared memory blocks
   */
  function pruneStaleSessions()
  {
    $this->fDebug->add("Pruning stale sessions, but sessions expire naturally within memcached", 6);
    return true;
  }

  /**
   * Save our Session Data
   *
   * @param String ID
   * @param Array Session Data
   */
  function save($id, $data)
  {
    $this->fDebug->add("Saving session ID: ".$id.".", 6);
    return $this->cache->set($id, $data);
  }

  /**
   * Counts the number of active sessions as determined by visitor_time_threshold
   * @return int Visitor Count
   */
  function countSessions()
  {
    $this->fDebug->add("Counting Sessions.", 6);
    return 0;
  }

  /**
   * Counts the number of authenticated active sessions
   * @return int Authenticated Visitor Count
   * @todo Add support for add'l session counts
   */
  function countAuthedSessions()
  {
    return 0;
  }

  /**
   * Counts the number of authenticated active sessions
   * @return int Authenticated Visitor Count
   * @todo Add support for add'l session counts
   */
  function countGuestSessions()
  {
    return 0;
  }
}
?>

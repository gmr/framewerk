<?php // $Id: fSessionStoreSHM.inc 881 2007-06-14 01:51:06Z gmr $
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
 * Session Store SHM Object
 *
 * This object is responsible implementing the functions defined in ISessionStore for sessions data management
 *
 * @author     Gavin M. Roy <gavinmroy@gmail.com>
 * @link       http://framewerk.org
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2005-2007 the Framewerk Development Group
 * @since      2004-02-23
 * @version    $Rev: 881 $
 * @package    Engine
 * @subpackage Core
 * @uses       iSessionStore
 * @uses       fSHMOPCache
 */

class fSessionStoreSHM implements iSessionStore
{
  private $cache, $fDebug, $fMain;

  /**
   * Validate we can use SHMOPCache
   */
  function __construct()
  {
    // Make sure we have Shared Memory support
    try
    {
      $this->cache = new fSHMOPCache('fSessionStoreSHM');
    } Catch ( exception $e) {
      throw new exception("SHMOP (Shared Memory) Is not installed.  You can not select SessionStoreSHM for Session Storage.");
    }

    // Set our internal fMain pointer
    $this->fDebug = fDebug::getInstance();
    $this->fMain = fMain::getInstance();
  }

  /**
   * Load the session data
   *
   * @param String ID
   * @return Array Session Data
   */
  function loadSession($id)
  {
    $this->fDebug->add("Loading session ID: $id.", 6);

    // Set our ID Value
    $this->cache->setID('Engine/fSessionStoreSHM.inc-' . $id);

    // Read in our Data
    $sessions = $this->cache->readSHMData();
    if ( !$sessions )
    {
      $this->save($id, array("empty"=>"true"));
      $this->fDebug->add("Session data not found for ID: $id.", 4);
      return false;
    }

    // Return our session data
    return $sessions;
  }

  /**
   * Prune old sessions out of the session table and remove their shared memory blocks
   */
  function pruneStaleSessions()
  {
    $this->fDebug->add("Pruning stale sessions.", 6);

    // Read in our table of sessions
    $this->cache->setID('Engine/fSessionStoreSHM.inc');
    $temp = $this->cache->readSHMData();

    // create a new session table array
    $new = array();
    $duration = intval($this->fMain->configuration->session->cookie->duration);

    // Loop through each row and if it falls in the duration keep it,
    // else wipe the memory and omit it from the new table
    foreach($temp AS $key=>$value )
    if ( $value >= time() - $duration )
    {
      $new[$key] = $value;
    } else {
      // Overwrite the TSHMCache::id value so we dont internally rehash it in TSHMCache::computeIDValue
      $this->cache->id = $key;

      // Wipe the old data
      $this->cache->saveSHMData(NULL);
      $this->fDebug->add("Pruning the key $key.", 6);
    }

    // write the new table
    $this->cache->setID('Engine/fSessionStoreSHM.inc');
    $this->cache->saveSHMData($new);
  }

  /**
   * Update our session timestamp for a given ID to the current timestamp
   *
   * @param Session ID to update
   */
  function updateSessionTimestamp($id)
  {
    $this->cache->setID('Engine/fSessionStoreSHM.inc');
    $data = $this->cache->readSHMData();
    $data[$id] = time();
    if ( !$this->cache->saveSHMData($data) )
    {
      $this->fDebug->add("Error writing session timestamp table.", 4);
    }
  }

  /**
   * Save our Session Data
   *
   * @param String ID
   * @param Array Session Data
   */
  function saveSession($id, $data)
  {
    $this->fDebug->add("Saving session ID: $id.", 6);

    // Get our ID Value
    $id = $this->cache->setID('Engine/fSessionStoreSHM.inc-' . $id);

    // Write out our data
    if ( $this->cache->saveSHMData($data) )
    {
      $this->updateSessionTimestamp($id);
      return true;
    }

    // Update our index of sessions
    return false;
  }

  /**
   * Counts the number of active sessions as determined by visitor_time_threshold
   * @return int Visitor Count
   */
  function countSessions()
  {
    $this->fDebug->add("Counting Sessions.", 6);

    $this->cache->setID('Engine/fSessionStoreSHM.inc');
    $temp = $this->cache->readSHMData();

    $c = 0;
    foreach($temp AS $key=>$value )
    {
      if ( $value >= time() - intval($this->fMain->configuration->visitor_time_threshold) )
      {
        $c++;
      }
    }

    $this->fDebug->add("Returning a session count of: $c.", 6);
    return intval($c);
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
<?php // $Id: fSessionStoreDB.inc 919 2007-08-06 18:34:41Z dallas $
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
 * Session Store Database Class
 *
 * is object is responsible implementing the functions defined in iSessionStore for sessions data management
 *
 * @author     Gavin M. Roy <gavinmroy@gmail.com>
 * @link       http://framewerk.org
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2004-2007 the Framewerk Development Group
 * @since      2004-02-23
 * @version    $Rev: 919 $
 * @package    Engine
 * @subpackage Core
 * @uses       iSessionStore
 * @uses       fMain
 */

class fSessionStoreDB implements iSessionStore
{
  private $fDebug, $fMain;
  private $original;

  /**
   * Validate that we can write to the filesystem, raise a warning if not
   */
  function __construct()
  {
    // Set our internal fMain pointer
    $this->fMain = fMain::getInstance();
    $this->fDebug = fDebug::getInstance();

    // Check to make sure the database is connected
    if ( !isset($this->fMain->PDO) )
      trigger_error("The fMain::PDO object has yet to be instanced and cant be used.", E_USER_WARNING);
  }

  /**
   * Load the session from the table
   *
   * @param string $id
   * @return string session data
   */
  function loadSession($id)
  {
    $this->fDebug->add("Attempting to load a session from the database", 4);

    $query = $this->fMain->PDO->prepare("SELECT data FROM sessions WHERE sessionid = :sessionid;");
    $query = $this->fMain->PDO->prepare("SELECT data FROM sessions WHERE sessionid = :sessionid;");

    $query->bindParam(":sessionid", $id);
    $query->execute();
    $data = $query->fetch(PDO::FETCH_NAMED);
    unset($query);

    if ( is_array($data) )
    {
      if ( $this->fMain->configuration->session['audit'] == '1' )
      {
        $query = $this->fMain->PDO->prepare("INSERT INTO session_audit VALUES ( now(),'l', :ipaddress, :uri, :data );");
        $query->bindParam(':ipaddress', $_SERVER['REMOTE_ADDR']);
        $query->bindParam(':uri', $_SERVER['REQUEST_URI']);
        $query->bindParam(':data', $data['data']);
        $query->execute();
        unset($query);
      }

      // Update our timestamp
      $query = $this->fMain->PDO->prepare("UPDATE sessions SET lastactivity = now() WHERE sessionid = :sessionid;");

      $query->bindParam(":sessionid", $id);
      $query->execute();
      unset($query);

      $this->fDebug->add("Returning a valid session from the database", 4);
      $this->original = $data['data'];
      $out = unserialize(base64_decode($data['data']));
      $this->fDebug->add('Session Data:' . print_r($out, true), 6);
      return $out;
    } else {
      $this->fDebug->add("Creating a new session in the database", 4);
      $query = $this->fMain->PDO->prepare("INSERT INTO sessions VALUES( :sessionid, now(), '--Empty--');");
      $query->bindParam(":sessionid", $id);
      $query->execute();

      $this->fDebug->add("Session data not found.", 4);
      return false;
    }
  }

  /**
   * Remove old sessions
   *
   * @return bool
   */
  function pruneStaleSessions()
  {
    if ( strval($this->fMain->configuration->session->cleanup) == "true" )
    {
      $interval = $this->fMain->configuration->session->cookie['duration'];
      $query = $this->fMain->PDO->query("DELETE FROM sessions WHERE lastactivity < now() - interval '$interval seconds';");
      return true;
    }
    return false;
  }

  /**
   * Save our Session Data
   */
  function save($id, $data)
  {
    unset($data['starttime']);
    $serialized = base64_encode(serialize($data));

    // Temp audit stuff
    if ( strval($this->fMain->configuration->session->audit) == "true" )
    {
      $query = $this->fMain->PDO->prepare('INSERT INTO session_audit VALUES ( now(),\'s\', :ipaddress, :uri, :data );');
      $query->bindParam(':ipaddress', $_SERVER['REMOTE_ADDR']);
      $query->bindParam(':uri', $_SERVER['REQUEST_URI']);
      $query->bindParam(':data', $serialized);
      $query->execute();
      unset($query);
    }
    if ( $this->original == $serialized )
    {
      $this->fDebug->add("No changes to session data", 4);
      return true;
    }
    $this->fDebug->add("Attempting to save a session to the database", 4);
    $query = $this->fMain->PDO->prepare('UPDATE sessions SET lastactivity = now(), data = :data WHERE sessionid = :sessionid;');

    $query->bindParam(":sessionid", $id);
    $query->bindParam(":data", $serialized);
    if ( !$query->execute() )
    {
      throw new Exception('Error saving session data. '.print_r($query->errorInfo(),true));
    }
    return true;
  }

  /**
   * Counts the number of active sessions as determined by visitor_time_threshold
   * @return int Visitor Count
   */
  function countSessions()
  {
    $timestamp = strftime("%Y/%m/%d %H:%M", time() - intval($this->Configuration->visitor_time_threshold));
    $query = $this->fMain->PDO->prepare("SELECT sessionid FROM sessions WHERE lastactivity >= :timestamp;");
    $query->bindParam(':timestamp', $timestamp);
    $query->execute();
    return $query->rowCount();
  }

  /**
   * Counts the number of authenticated active sessions
   * @return int Authenticated Visitor Count
   */
  function countAuthedSessions()
  {
    $timestamp = strftime("%Y/%m/%d %H:%M", time() - intval($this->Configuration->visitor_time_threshold));
    $query = $this->fMain->PDO->prepare("SELECT sessionid,data FROM sessions WHERE lastactivity >= :timestamp;");
    $query->bindParam(':timestamp', $timestamp);
    $query->execute();
    $count = 0;
    foreach ( $query->fetchAll(PDO::FETCH_OBJ) as $row )
    {
      $sessionData = unserialize(base64_decode($row->data));
      if ( intval($sessionData['authenticated']) == 1 )
      {
        $count++;
      }
    }
    return $count;
  }

  /**
   * Counts the number of authenticated active sessions
   * @return int Authenticated Visitor Count
   */
  function countGuestSessions()
  {
    $x = $this->countSessions() - $this->countAuthedSessions();
    return $x;
  }
  
}
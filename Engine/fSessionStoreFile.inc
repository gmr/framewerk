<?php // $Id: fSessionStoreFile.inc 945 2007-12-20 21:10:24Z madcoder $
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
 * Session Store Filesystem Object
 *
 * This object is responsible implementing the functions defined in ISessionStore for session data management
 *
 * @author     Gavin M. Roy <gavinmroy@gmail.com>
 * @link       http://framewerk.org
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2004-2007 the Framewerk Development Group
 * @since      2004-02-23
 * @version    $Rev: 945 $
 * @package    Engine
 * @subpackage Core
 * @uses       iSessionStore
 */

class fSessionStoreFile implements iSessionStore
{
  private $fRegistry;

  /**
   * Validate that we can write to the filesystem, raise a warning if not
   */
  function __construct()
  {
    // Set our internal fMain pointer
    $this->fRegistry = new fRegistry('fSession');

    // Check if the session path is accessible
    if ( !is_writable($this->fRegistry->handler->path) )
    {
      trigger_error($this->fRegistry->handler->path . " is not writable, cannot save this session.", E_USER_WARNING);
    }
  }

  function loadSession($id)
  {
    $sessionFile = strval($this->fRegistry->handler->path) . '/' .$id;
    if ( is_file($sessionFile) && filesize($sessionFile) > 0 )
    {
      $data = unserialize(file_get_contents($sessionFile));
      if ( $data === false )
      {
        fDebug::getInstance()->add("Session data could not be read.", 4);
        return false;
      } else
      return $data;
    }
  }

  /**
   * Remove old session files that have expired cookies
   */
  function pruneStaleSessions()
  {
    $path = strval($this->fRegistry->handler->path) . '/';
    $expire = time() - intval($this->fRegistry->cookie->duration);
    if ( $this->fRegistry->cleanup )
    {
      if ( $handle = opendir($path) )
        while ( ( $filename = readdir($handle) ) != false )
          if ( is_file($path . $filename) )
            if ( ( filemtime( $path . $filename) < $expire ) && ( $filename != "." ) && ( $filename != ".." ) )
              unlink($path . $filename);
      closedir($handle);
    }
  }

  /**
   * Save our Session Data
   */
  function save($id, $data)
  {
    $sessionFile = strval($this->fRegistry->handler->path) . '/' . $id;
    if ( ( !is_writable($this->fRegistry->handler->path) ) ||
         ( is_file($sessionFile) && !is_writable($sessionFile) ) )
    {
      trigger_error($this->fRegistry->handler->path . " is not writable, cannot save this session.", E_USER_WARNING);
      return false;
    }
    file_put_contents($sessionFile, serialize($data) . "\n");
    return true;
  }

  /**
   * Counts the number of active sessions as determined by visitor_time_threshold
   * @return int Visitor Count
   */
  function countSessions()
  {
    fProfile::getInstance()->profile("SessionCount");
    $x = 0;
    $visittime = time() - intval($this->fRegistry->visitor_time);
    if ( $handle = opendir(strval($this->fRegistry->handler->path)) )
      while ( ( $filename = readdir($handle) ) != false )
        if ( ( filemtime(strval($this->fRegistry->handler->path) . '/' . $filename) > $visittime ) &&
             ( is_file(strval($this->fRegistry->handler->path) . '/' . $filename)) )
          $x++;
    closedir($handle);
    fProfile::getInstance()->profile("SessionCount", "GetVisitors()");
    return $x;
  }

  /**
   * Counts the number of authenticated active sessions
   * @return int Authenticated Visitor Count
   */
  function countAuthedSessions()
  {
    $count = 0;
    $visittime = time() - intval($this->fRegistry->visitor_time);
    if ( $handle = opendir(strval($this->fRegistry->handler->path)) )
      while ( ( $filename = readdir($handle) ) != false )
        if ( ( filemtime(strval($this->fRegistry->handler->path) . '/' . $filename) > $visittime ) &&
             ( is_file(strval($this->fRegistry->handler->path) . '/' . $filename)) )
        {
          $sessionData = $this->loadSession($filename);
          if ( intval($sessionData['authenticated']) == 1 )
            $count++;
        }
    closedir($handle);
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
?>
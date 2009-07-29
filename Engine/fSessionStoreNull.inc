<?php // $Id: fSessionStoreNull.inc 29065 2009-05-11 18:30:00Z jhansche $
############################################################################
#    Copyright (C) 2004-2009 by the Framewerk Development Group            #
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
 * Session Store NULL Object
 *
 * This object handles antistoring of session data
 *
 * @author     Joe Hansche <madcoder@gmail.com>
 * @link       http://framewerk.org
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2005-2009 the Framewerk Development Group
 * @since      2009-05-10
 * @version    $Rev: 29065 $
 * @package    Engine
 * @subpackage Core
 * @uses       iSessionStore
 */

class fSessionStoreNull implements iSessionStore
{
  /**
   * Validate we can use SHMOPCache
   */
  function __construct()
  {
  }

  /**
   * Load the session data
   *
   * @param String ID
   * @return Array Session Data
   */
  function loadSession($id)
  {
    return array( );
  }

  /**
   * Prune old sessions out of the session table and remove their shared memory blocks
   */
  function pruneStaleSessions()
  {
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
    return true;
  }

  /**
   * Counts the number of active sessions as determined by visitor_time_threshold
   * @return int Visitor Count
   */
  function countSessions()
  {
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

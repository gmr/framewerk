<?php // $Id: iSessionStore.inc 864 2007-05-27 19:54:06Z gmr $
############################################################################
#    Copyright (C) 2005-2007 by the Framewerk Development Group            #
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
 * Session Store Interface
 *
 * Defines functions for session storage container objects
 *
 * @author     Gavin M. Roy <gavinmroy@gmail.com>
 * @link       http://framewerk.org
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @since      2005-02-23
 * @copyright  Copyright 2005-2007 the Framewerk Development Group
 * @version    $Rev: 864 $
 * @package    Engine
 * @subpackage Core
 */

interface iSessionStore
{

  /** Constructor
   * Validates that data store type is available
   */
  function __construct();

  /**
   * Saves session data in data store
   *
   * @param String Session id
   * @param Array Session data
   * @return bool Successful
   */
  function save($id, $values);

  /**
   * Loads session data from data store
   *
   * @param String Session ID
   * @return Array Session Data
   */
  function loadSession($sessionid);

  /**
   * Deletes stale sessions
   */
  function pruneStaleSessions();

  /**
   * Counts the current number of active sessions
   */
  function countSessions();

  /**
   * Counts the number of active authenticated sessions
   */
  function countAuthedSessions();

  /**
   * Counts the number of active guest sessions
   */
  function countGuestSessions();

}
?>
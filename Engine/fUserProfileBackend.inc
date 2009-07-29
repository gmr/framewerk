<?php // $Id: fUserProfileBackend.inc 881 2007-06-14 01:51:06Z gmr $
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
 * fUserBackend
 *
 * Abstracted fUserProfileBackend class
 */
/**
 * User Profile Backend Abstract Class
 *
 * This is a fUserProfile Backend abstract class
 *
 * @author     Joe Hansche <madcoder@gmail.com>
 * @link       http://www.framewerk.org
 * @license    BSD
 * @copyright  Copyright 2005-2007 the Framewerk Development Group
 * @version    $Rev: 881 $
 * @package    Engine
 * @subpackage Core
 * @since      2006-12-17
 * @uses       fObject
 */

abstract class fUserProfileBackend extends fObject
{
  protected $data;
  protected $originalData;
  protected $userid;

  /**
   * Constructor
   *
   * Loads user data by username, email address or user ID or sets up a new blank user.
   *
   * @param string key
   */
  function __construct($key)
  {
    parent::__construct();

    if ( strlen(trim($key)) == 0 || false === strpos($key, ':') )
      throw new Exception('Cannot instantiate profile with this key: "'.$key.'"');

    $this->userid = $key;

    if ( $this->loadUserData() === false )
      $this->fDebug->add('Could not load profile data for ' . $key . ' in ' .get_class($this) . '.');

    // Keep a backup of original data so we know what needs to be updated in save()
    $this->originalData = $this->data;

    $this->fDebug->add('User profile backend ' . get_class($this) . ' loaded.', 6);

  }

  /**
   * Save the user data
   *
   * @return bool
   */
  abstract function save();

  /**
   * Load the data for this user
   *
   * @returns bool true on success, false on failure
   */
  abstract function loadUserData();

}
?>

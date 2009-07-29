<?php // $Id: fUserBackend.inc 919 2007-08-06 18:34:41Z dallas $
############################################################################
#    Copyright (C) 2006-2007 by the Framewerk Development Group            #
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
 * Abstracted fUserBackend class
 */
/**
 * PDO User Backend Abstract Class
 *
 * This is a fUser Backend abstract class
 *
 * @author     Gavin M. Roy <gavinmroy@gmail.com>
 * @author     Joe Hansche <madcoder@gmail.com>
 * @link       http://framewerk.org
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2006-2007 the Framewerk Development Group
 * @version    $Rev: 919 $
 * @package    Engine
 * @subpackage Core
 * @since      2006-09-13
 * @uses       fMainObject
 * @abstract
 */

abstract class fUserBackend extends fObject
{

  public $id;              // Unique ID string [optional identification string]
  public $username;        // Unique user name [optional identification string]
  public $email;           // Unique email address [optional identification string]
  public $password;        // Password in clear text
  public $permissionList;  // List of permissions available to the user
  public $roleList;        // Array of roles assigned to the user
  public $lastActivity;    // Timestamp of last activity
  public $status;          // Current status of the account

  /**
   * Constructor
   *
   * Loads user data by username, email address or user ID or sets up a new blank user.
   *
   * @param string key
   */
  function __construct($key = NULL)
  {
    parent::__construct();

    if ( $key == NULL )
    {
      $this->id = -1;
      return true;
    }
  }

  /**
   * Return an array of permissions, requires $this->roleList to already be set
   *
   * @return array
   */
  abstract function getPermissions();

  /**
   * Authenticate the password and make sure the user has a status of Active to allow for login
   *
   * @param string $password
   * @return bool
   */
  abstract function authenticate($password);

  /**
   * Return a key hash which can be used in remember me cookies and the sort
   *
   * @return string
   */
  function getKeyHash()
  {
    $this->fWarn->add("This fUser interface does not support hash authentication.");
    return false;
    //return md5((crc32($this->fMain->configuration->site->name . '|' .$this->id . '^' . $this->password) + $this->id) . '|' . $this->password);
  }

  /**
   * Authorize against the key hash generated in getKeyHash()
   *
   * @param string $hash
   * @return bool
   */
  function authenticateAgainstHash($hash)
  {
    $this->fWarn->add("This fUser interface does not support hash authentication.");
    return false;
  }

  /**
   * Save the user data
   *
   * @return bool
   */
  abstract function save();

  /**
   * Add a role to the users role list
   *
   * @param string $role
   * @return bool
   */
  function addRole($role)
  {
    $this->fWarn->add("This fUser interface does not support changing roles.");
    return false;
  }

  /**
   * Remove a role from the users Roll list
   *
   * @param  string $role
   * @return bool
   */
  function removeRole($role)
  {
    $this->fWarn->add("This fUser interface does not support changing roles.");
    return false;
  }
}
?>

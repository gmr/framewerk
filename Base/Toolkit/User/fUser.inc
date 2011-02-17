<?php // $Id: fUser.inc 945 2007-12-20 21:10:24Z madcoder $
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
 * fUser
 *
 * This file contains the fUser object.
 */
/**
 * Base User Class
 *
 * This is the base user class for the fMain Engine.  Authentication changes should be made in a new clone of fUserFramewerk.inc as fUser<auth_type>.inc where <auth_type> is specified in the config
 *
 * @author     Gavin M. Roy <gavinmroy@gmail.com>
 * @link       http://framewerk.org
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2005 - 2007 the Framewerk Development Group
 * @version    $Rev: 945 $
 * @since      2005-02-22
 * @package    Engine
 * @subpackage Core
 * @uses       fMainObject
 */

class fUser extends fObject
{

  private $dataInterface;  // Instance of the fUser backend containing the data for this user
  private $selectedBackend;// String containing the fUser backend for this user
  private $permissionList; // Array of permission strings available to the user

  public $key;             // The preferred key for accessing this user
  public $id;              // Unique ID string [optional identification string]
  public $username;        // Unique user name [optional identification string]
  public $email;           // Unique email address [optional identification string]
  public $password;        // Password in clear text
  public $lastActivity;    // Timestamp of last activity
  public $roleList;        // Array of roles available to the user
  public $status;          // Current status of the account
  public $uid;             // UID is handler:id
  public $userInfo;        // AccountInfo object

  /**
   * Class Constructor
   *
   * Pass the uniquely identifying information into the constructor so that it can attempt to locate the
   * user information from the various user backend classes.
   *
   * Valid key values:
   *   key
   *   backend:key
   * Examples
   *   PDO:1
   *   XML:gavinmroy
   *   LDAP:CN=Joe Hansche,OU=Administrators,DC=tech,DC=local
   *   MYBOld:1
   *
   * @param string $key
   */
  function __construct($key)
  {
    parent::__construct();
    
    // Try and get each element of a compound key as it comes in.
    $data = explode(':', $key);
    /**
     * If there is more than one element we came in with the Backend:Key pairing
     * So set the handler and key appropriately, otherwise get the handler from
     * the registy
     */
    if ( Count($data) > 1 )
    {
      $handler = 'fUser' . $data[0];
      $key = $data[1];
      $this->uid = $data[0] . ':' . $data[1];
    } else {
      $registry = new fRegistry($this);
      $handler = 'fUser' . $registry->handler;
      $this->uid = $registry->handler . ':' . $key;
    }

    // Try to create the handler class
    try
    {
      $this->dataInterface = new $handler($key);
      
      if ( !strlen($this->dataInterface->id) )
      {
        throw new fUserException('Could not load user data for the user:' . $key, fUserException::E_FUSER_NOSUCHUSER);
      }

      // Set our internal variables
      $this->selectedBackend = $handler;
      $this->key = $key;
    } catch ( fUserException $e ) {
      // If it is not marked with the E_FUSER_INFO flag, then it should be propagated
      if ( ($e->getCode() & fUserException::E_FUSER_INFO) == 0 )
        throw $e;
      else 
        return false;
    }

    // Get the values and fill our internal arrays
    $this->id             = $this->dataInterface->id;
    $this->username       = $this->dataInterface->username;
    $this->password       = $this->dataInterface->password;
    $this->email          = $this->dataInterface->email;
    $this->lastActivity   = $this->dataInterface->lastActivity;
    $this->status         = $this->dataInterface->status;
    $this->permissionList = $this->dataInterface->permissionList;
    $this->roleList       = $this->dataInterface->roleList;
    if ( $this->dataInterface->isEntitySet('info') )
      $this->userInfo     = $this->dataInterface->info;
    return true;
  }

  /**
   * Add a permission to the permission list if it's not in the list already
   *
   * @param string $permission
   * @return bool
   */
  function addPermission($permission)
  {
    if ( !in_array($permission, $this->permissionList) )
    {
      $this->permissionList[] = $permission;
      return true;
    }
    return false;
  }

  /**
   * Passthrough to the dataInterface to authenticate
   *
   * @param string $password
   * @return bool
   */
  function authenticate($password)
  {
    $auth = $this->dataInterface->authenticate($password);
    if ( $auth === false ) $this->permissionList = array();
    return $auth;
  }

  /**
   * Authenticate against a key hash as generated by dataInterface->getKeyHash()
   *
   * @param string $hash
   * @return bool
   */
  function authenticateAgainstHash($hash)
  {
    return $this->dataInterface->authenticateAgainstHash($hash);
  }

  /**
   * Check to see if a permission is in the permission list. Permission string
   * can contain any level of (), && and || for complex permissions.
   * e.g. "((foo||bar)&&derp)||herp"
   *
   * Special Permission Functions:
   * id(1234)
   * username(foo)
   * email(foo@bar.com)
   * e.g. "id(1234)||username(foo)||derp"
   *
   * @param string $permissionString
   * @return bool
   */
  function checkPermission($permissionString)
  {
    static $pregCallBack=null;

    if ( ( in_array('Administrator', $this->permissionList) ) ) return true;

    if(trim($permissionString)=='')
      return true;

    if($pregCallBack==null) {
      if($this->id)       $this->permissionList[] = "id({$this->id})";
      if($this->username) $this->permissionList[] = "username({$this->username})";
      if($this->email)    $this->permissionList[] = "email({$this->email})";

      $pregCallBack = create_function('$m', "\$a=array('".implode("','", $this->permissionList)."'); return in_array(\$m[1], \$a)?'true':'false';");
    }

    $permissionString = preg_replace('/ /', '', $permissionString);
    $permissionString = preg_replace_callback('/([A-z0-9_]+(\([A-z0-9_]+\))?)/', $pregCallBack, $permissionString);
    if(!preg_match('/^(true|false|[()|&])+$/', $permissionString)) return false;

    $evalsuccess = @eval("\$result = $permissionString;");

    return $evalsuccess===null && $result;
  }

  /**
   * Pass through to the Data Interface to return a key hash which consists of
   * the userid and other unique info to validate "passwordless" login such as
   * a remember me cookie
   *
   * @return string
   */
  function getKeyHash()
  {
    return $this->dataInterface->getKeyHash();
  }

  /**
   * Return an array of user data
   *
   * @todo Complete the design and implement this
   * @return array
   */
  function gefUserList()
  {
    return $this->dataInterface->gefUserList();
  }

  /**
   * Save changes to the user data
   *
   * @return bool
   */
  function save()
  {
    // If we need to save to our data interface, set it's values and do so
    if ( $this->validateChanges() )
    {
      if ( $this->dataInterface->username != $this->username ) $this->dataInterface->username = $this->username;
      if ( $this->dataInterface->password != $this->password ) $this->dataInterface->password = $this->password;
      if ( $this->dataInterface->email != $this->email ) $this->dataInterface->email = $this->email;
      if ( $this->dataInterface->lastActivity != $this->lastActivity ) $this->dataInterface->lastActivity = $this->lastActivity;
      if ( $this->dataInterface->status != $this->status ) $this->dataInterface->status = $this->status;
      //if ( $this->dataInterface->roleList != $this->roleList ) $this->dataInterface->roleList = $this->roleList;
      $result = $this->dataInterface->save();
      $this->uid .= $this->dataInterface->id;
      return $result;
    }
    return false;
  }

  /**
   * See if we need to update our data points
   *
   * @return bool
   */
  function validateChanges()
  {
    if ( ( $this->dataInterface->username != $this->username ) ||
    ( $this->dataInterface->password != $this->password ) ||
    ( $this->dataInterface->email != $this->email ) ||
    ( $this->dataInterface->lastActivity != $this->lastActivity ) ||
    ( $this->dataInterface->status != $this->status ) ||
    ( $this->dataInterface->permissionList != $this->permissionList ) )
    {
      return true;
    } else {
      return false;
    }
  }

  /**
   * Pass through to the dataInterface->addRole() function
   *
   * @param string $role
   * @return bool
   */
  function addRole($role)
  {
    return $this->dataInterface->addRole($role);
  }

  /**
   * Pass through to the dataInterface->getRoles() function
   *
   * @return array
   */
  function getRoles()
  {
    return $this->dataInterface->getRoles();
  }

  /**
   * Pass through to the dataInterface->removeRole() function
   *
   * @param string $role
   * @return bool
   */
  function removeRole($role)
  {
    return $this->dataInterface->removeRole($role);
  }
}

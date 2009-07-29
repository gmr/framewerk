<?php // $Id: fUserPDO.inc 919 2007-08-06 18:34:41Z dallas $
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
 * fUserPDO
 *
 * This file contains the fUserPDO object.
 */
/**
 * PDO User Class
 *
 * This is the default user class data handler for the fMain Engine
 *
 * @author     Gavin M. Roy <gavinmroy@gmail.com>
 * @link       http://framewerk.org
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2005-2007 the Framewerk Development Group
 * @version    $Rev: 919 $
 * @package    Engine
 * @subpackage Core
 * @since      2005-08-22
 * @uses       fMainObject
 */

class fUserPDO extends fUserBackend
{

  public $id;              // Unique ID string [optional identification string]
  public $username;        // Unique user name [optional identification string]
  public $email;           // Unique email address [optional identification string]
  public $password;        // Password in clear text
  public $permissionList;  // List of permissions available to the user
  public $roleList;        // Array of roles assigned to the user
  public $lastActivity;    // Timestamp of last activity
  public $status;          // Current status of the account
  public $info;		   // Dummy field added by Joe @todo find out what it does

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

    // Get the user data
    if ( is_numeric($key) )
    {
      $query = $this->PDO->prepare('SELECT * FROM fuser_users WHERE id = :id;');
    } else {
      $query = $this->PDO->prepare('SELECT * FROM fuser_users WHERE username = :id OR email = :id;');
    }
    $query->bindParam(':id', $key);
    $query->execute();
    $data = $query->fetch(PDO::FETCH_OBJ);
    if ( is_object($data) )
    {
      // Set our values
      $this->id           = $data->id;
      $this->username     = $data->username;
      $this->email        = $data->email;
      $this->password     = $data->password;
      $this->status       = $data->status;
      $this->lastActivity = $data->lastactivity;
      $this->roleList     = $this->getRoleListNames();
      $this->permissionList = $this->getPermissions();
      return true;
    } else {
      return false;
    }
  }

  /**
   * Return an array of permissions, requires $this->roleList to already be set
   *
   * @return array
   */
  function getPermissions()
  {
    // Get Role based permissions
    $roles = $this->getRoles();
    if ( Count($roles) == 0 )
    {
      return array();
    }
    $roleList = '';
    foreach ( $roles AS $role )
    {
      $roleList .= $role . ',';
    }
    $roleList = rtrim($roleList, ',');
    $sql = 'SELECT DISTINCT permission FROM fuser_rolepermissions WHERE roleid IN (?);';
    $query = $this->PDO->prepare($sql);
    $query->execute(array($roleList));
    $data = $query->fetchAll(PDO::FETCH_OBJ);
    $permissions = array();
    foreach($data AS $permission)
    {
      $permissions[] = $permission->permission;
    }

    // Get User specific permissions
    $query = $this->PDO->prepare('SELECT permission FROM fuser_userpermissions WHERE userid = :id;');
    $query->bindParam(':id', $this->id);
    $query->execute();
    $data = $query->fetchAll(PDO::FETCH_OBJ);
    foreach($data AS $permission)
    {
      $permissions[] = $permission->permission;
    }

    // Return the permission list
    return $permissions;
  }

  /**
   * Return an array of roles assigned to the user
   *
   * @param int $parentid
   * @return array
   */
  function getRoles($parentid = NULL)
  {
    $roles = array();
    if ( isset($parentid) )
    {
      $query = $this->PDO->prepare('SELECT id, parent FROM fuser_roles WHERE id = :id');
      $query->bindParam(':id', $parentid);
    } else {
      $query = $this->PDO->prepare('SELECT fuser_roles.id, fuser_roles.parent FROM fuser_roles JOIN fuser_rolemembership ON fuser_rolemembership.roleid = fuser_roles.id WHERE fuser_rolemembership.userid = :userid;');
      $query->bindParam(':userid', $this->id);
    }
    $query->execute();
    $data = $query->fetchAll(PDO::FETCH_OBJ);
    foreach ( $data AS $row )
    {
      if ( !in_array($row->id, $roles) )
      {
        $roles[] = $row->id;
      }
      if ( $row->parent > 0 )
      {
        $temp = $this->getRoles($row->parent);
        $roles = array_merge($roles, $temp);
      }
    }
    return $roles;
  }

  /**
   * Return an array of role names assigned to the user
   *
   * @param int $parentid
   * @return array
   */
  function getRoleListNames($parentid = NULL)
  {
    $roles = array();
    if ( isset($parentid) )
    {
      $query = $this->PDO->prepare('SELECT name, id, parent FROM fuser_roles WHERE id = :id');
      $query->bindParam(':id', $parentid);
    } else {
      $query = $this->PDO->prepare('SELECT fuser_roles.name, fuser_roles.id, fuser_roles.parent FROM fuser_roles JOIN fuser_rolemembership ON fuser_rolemembership.roleid = fuser_roles.id WHERE fuser_rolemembership.userid = :userid;');
      $query->bindParam(':userid', $this->id);
    }
    $query->execute();
    $data = $query->fetchAll(PDO::FETCH_OBJ);
    foreach ( $data AS $row )
    {
      if ( !in_array($row->name, $roles) )
      {
        $roles[] = $row->name;
      }
      if ( $row->parent > 0 )
      {
        $temp = $this->getRoleListNames($row->parent);
        $roles = array_merge($roles, $temp);
      }
    }
    return $roles;
  }

  /**
   * Authenticate the password and make sure the user has a status of Active to allow for login
   *
   * @param string $password
   * @return bool
   */
  function authenticate($password)
  {
    if ( ( md5($password) == $this->password ) && ( $this->status == 'Active' ) )
    {
      return true;
    }
    return false;
  }

  /**
   * Return a key hash which can be used in remember me cookies and the sort
   *
   * @return string
   */
  function getKeyHash()
  {
    return md5((crc32($this->fMain->configuration->site->name . '|' .$this->id . '^' . $this->password) + $this->id) . '|' . $this->password);
  }

  /**
   * Return an array of user data
   *
   * @return array
   */
  function gefuserList($fields = NULL)
  {
    if ( $query = $this->PDO->query('SELECT id, username, email, lastactivity, status FROM fuser_users;') )
    {
      return $query->fetchAll(PDO::FETCH_ASSOC);
    }

    return false;
  }

  /**
   * Authorize against the key hash generated in getKeyHash()
   *
   * @param string $hash
   * @return bool
   */
  function authenticateAgainstHash($hash)
  {
    if ( $hash == $this->getKeyHash() )
    {
      return true;
    }
    return false;
  }

  /**
   * Save the user data
   *
   * @return bool
   */
  function save()
  {
    // If it's a new user call addUser() otherwise call updateUser()
    if ( $this->id == -1 )
    {
      return $this->addUser();
    } else {
      return $this->updateUser();
    }
  }

  /**
   * Add a new user
   *
   * @return bool
   */
  function addUser()
  {
    // Get the user data
    $sql = 'SELECT * FROM fuser_users WHERE ';
    if ( strlen($this->username) > 0 )
    {
      $sql .= 'username = :username OR ';
    }
    $sql .= 'email = :email;';
    $query = $this->PDO->prepare($sql);
    if ( strlen($this->username) > 0 )
    {
      $query->bindParam(':username', $this->username);
    }
    $query->bindParam(':email', $this->email);
    $query->execute();
    $result = $query->fetchAll(PDO::FETCH_OBJ);
    if ( count($result) == 0 )
    {
      $query = $this->PDO->prepare('INSERT INTO fuser_users ( username, password, email, lastactivity, status ) VALUES ( :username, :password, :email, now(), :status);');
      $query->bindParam(':username', $this->username);
      $query->bindParam(':email', $this->email);
      // If the password isn't a MD5, make it one.  Assumes they can't enter a 32 character long password.
      if ( strlen($this->password) < 32 )
      {
        $this->password = md5($this->password);
      }
      $query->bindParam(':password', $this->password);
      $query->bindParam(':status', $this->status);
      if ( $query->execute() )
      {
        $query = $this->PDO->prepare('SELECT * FROM fuser_users WHERE email = :email');
        $query->bindParam(':email', $this->email);
        $query->execute();
        $data = $query->fetch(PDO::FETCH_OBJ);
        $this->id = $data->id;
        $this->updateRoles();
        return true;
      } else {
        throw new Exception('Error adding new user', 503);
      }
    } else {
      throw new Exception('Attempted to add a user when a pre-existing unique column already exists for another user.', 502);
    }
  }

  /**
   * If it's an existing user we want to update their record
   *
   * @return bool
   */
  function updateUser()
  {
    // Get the user data
    $sql = 'SELECT * FROM fuser_users WHERE ';
    if ( strlen($this->username) > 0 )
    {
      $sql .= 'username = :username OR ';
    }
    $sql .= 'email = :email OR id = :id;';
    $query = $this->PDO->prepare($sql);
    if ( strlen($this->username) > 0 )
    {
      $query->bindParam(':username', $this->username);
    }
    $query->bindParam(':id', $this->id);
    $query->bindParam(':email', $this->email);
    $query->execute();
    $result = $query->fetchAll(PDO::FETCH_OBJ);
    if ( count($result) == 1 )
    {
      if ( strlen($this->username) )
      {
        $username = 'username = :username, ';
      }
      $sql = 'UPDATE fuser_users SET ' . $username . 'password = :password, email = :email, lastactivity = :lastactivity, status = :status WHERE id = :id;';
      $query = $this->PDO->prepare($sql);
      $query->bindParam(':id', $this->id);
      if ( strlen($this->username) )
      {
        $query->bindParam(':username', $this->username);
      }
      $query->bindParam(':email', $this->email);
      // If the password isn't a MD5, make it one.  Assumes they can't enter a 32 character long password.
      if ( strlen($this->password) < 32 )
      {
        $this->password = md5($this->password);
      }
      $query->bindParam(':password', $this->password);
      $query->bindParam(':lastactivity', $this->lastActivity);
      $query->bindParam(':status', $this->status);
      if ( $query->execute() )
      {
        $this->updateRoles();
        return true;
      } else {
        throw new Exception('Error saving updated user information', 501);
      }
    } else {
      throw new Exception('Attempted to save when a pre-existing unique column already exists for another user.', 500);
    }
  }

  /**
   * Update the roles held by the user
   *
   * @return bool
   */
  function updateRoles()
  {
    // See if we have a missing role and add it to the saved role list if we do
    $roleList = $this->getRoleListNames();
    foreach ( $this->roleList AS $role )
    {
      if ( !in_array($role, $roleList) )
      {
        $query = $this->PDO->prepare('INSERT INTO fuser_rolemembership VALUES ( :userid, ( SELECT id FROM fuser_roles WHERE name = :role ));');
        $query->bindParam(':userid', $this->id);
        $query->bindParam(':role', $role);
        if ( !$query->execute() )
        {
          throw new Exception('Error adding role "' . $role . '" for this user.', 504);
        }
      }
    }

    /**
     * Look through the saved role list and look for the role in the new role list
     * If it's not in the new role list, add it to the SQL statement to remove it
     */
    $roleList = $this->getRoleListNames();
    $sql = 'DELETE FROM fuser_rolemembership WHERE userid = :userid AND roleid IN ( SELECT id FROM fuser_roles WHERE name IN (';
    $check = 0;
    foreach ( $roleList AS $role )
    {
      if ( !in_array($role, $this->roleList) )
      {
        $sql .= "'" . $role . "',";
        $check++;
      }
    }
    $sql = rtrim($sql, ',');
    $sql .= '));';

    // If we have rows to delete, do so
    if ( $check > 0 )
    {
      $query = $this->PDO->prepare($sql);
      $query->bindParam(':userid', $this->id);
      if ( !$query->execute() )
      {
        throw new Exception('Error removing roles for this user.', 505);
      }
    }

    // Update our role list as to what should be reflected in the database
    $this->roleList = $this->getRoleListNames();

    return true;
  }

  /**
   * Add a role to the users role list
   *
   * @param string $role
   * @return bool
   */
  function addRole($role)
  {
    if ( !in_array($role, $this->roleList) )
    {
      $query = $this->PDO->prepare('SELECT * FROM fuser_roles WHERE name = :role;');
      $query->bindParam(':role', $role);
      $query->execute();
      $data = $query->fetchAll(PDO::FETCH_OBJ);
      if ( !Count($data) )
      {
        trigger_error('Invalid role "' . $role . '" could not be added.', E_USER_ERROR);
        return false;
      }
      $this->roleList[] = $role;
      return $this->updateRoles();
    }
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
    if ( in_array($role, $this->roleList) )
    {
      $offset = array_search($role, $this->roleList);
      $start = array_slice($this->roleList, 0, $offset);
      $end = array_slice($this->roleList, $offset + 1);
      $this->roleList = array_merge($start, $end);
      return $this->updateRoles();
    }
    return false;
  }
}
?>

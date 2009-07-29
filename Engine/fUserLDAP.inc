<?php // $Id: fUserLDAP.inc 881 2007-06-14 01:51:06Z gmr $
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
 * LDAP User Backend Class
 *
 * This is a fUser LDAP backend class
 *
 * @author     Joe Hansche <madcoder@gmail.com>
 * @link       http://framewerk.org
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2006-2007 the Framewerk Development Group
 * @version    $Rev: 881 $
 * @package    Engine
 * @subpackage Core
 * @since      2006-09-13
 * @uses       fUserBackend
 */
class fUserLDAP extends fUserBackend
{

  private $ldap;
  private $fqun;

  /**
   * Loads user data by username, email address or user ID.  User creation via fUser is not implemented here.
   *
   * @param string key
   */
  function __construct($key = NULL)
  {
    parent::__construct($key);

    $this->fDebug->add("fUserLDAP: key = '$key'.", 6);

    $this->ldap = fLDAPCore::getInstance();

    if ( false === $this->ldap->pingLDAP() )
    {
      throw new Exception("An LDAP error occurred: " . $this->ldap->errorInfo());
    }


    /* XXX: Because the main focus of this backend at this time is MS Active Directory, that generally
     * means that it is going to be a corporate environment, so registering new users is not supported.
     * When this interface is updated to support more LDAP servers (like OpenLDAP), in which the directory
     * is simply a new place to store user info, this *will* be possible.
     */
    if ( $key === NULL )
    {
      $this->fWarn->add("This LDAP backend does not (yet) support creating new users!");
      return false;
    }


    /* For now, we're focusing on one domain with multiple domain controllers.  In the future, I will
     * update this backend to support multiple domains as well.  At that point, the domains given
     * here in the username will be useful - but for now, let's strip off the non-username portions
     * of the $key.
     */
    if ( strpos($key, '@') !== false )
    {
      // Key is in user@fq.domain.name format (might not be accurate if taken from email address)
      // This is the LDAPv3 preferred method, as long as it doesn't get confused with an email address
      // Generally, users will be more inclined to login with their plain username (sans domain)

      // Ignore the FQDN part (XXX: allow this in the future, if you have multiple separate domains defined in //LDAP/dc/)
      list($key) = explode('@', $key);
    } else if ( strpos($key, "\\") !== false )
    {
      // Key is in DOMAIN\user format
      // This is the format generally used by things like SMB file shares and NTLM/Kerberos authentication

      // Ignore the domain name (XXX: allow this in the future, if you have multiple separate domains defined in //LDAP/dc/)
      list(,$key) = explode("\\", $key);
    }
    // Otherwise, key is simply a username.  Hope fLDAP can figure it out.
    
    $data = $this->ldap->getUserInfo($key);

    if ( is_object($data) )
    {
      $this->username = $this->id = $data->username;
      // Build Fully-qualified Username from domain\username or username@domain.name (defined in configuration.xml)
      $this->fqun = $this->ldap->buildFQUN($this->username);
      $this->email = $data->email;
      $this->lastActivity = $data->lastactivity;
      $this->status = $data->status;
      $this->permissionList = $data->permissionList;
      $this->fDebug->add("User info retrieved for '$this->username'.",6);
    }
  }

  /**
   * Authenticate the password and make sure the user has a status of Active to allow for login
   *
   * @param string $password
   * @return bool
   */
  function authenticate($password)
  {
    $this->fDebug->add("Authenticating user '".$this->username."'...", 3);
    return $this->ldap->authenticate($this->fqun, $password);
  }

  /**
   * Save the user data -- NOT YET IMPLEMENTED
   *
   * @return bool
   * @todo Use this to save modifications to a user account.  This can only be accomplished with Domain Admin rights.
   */
  function save()
  {
    return false;
  }
  
  /** 
   * @returns Array User permissions as evaluated from all LDAP nested group memberships, all group-to-role
   *                nested mappings in configuration.xml, and all role-to-permission mappings in PDO.
   */
  function getPermissions()
  {
    return $this->permissionList;
  }

  /**
   * Add a role to the users role list - Not supported!
   * @todo This might be plausible once self::save() is implemented, but generally it is better to use the LDAP 
   *       administration interface to update users; especially in a corporate environment.
   */
  function addRole($role)
  {
    $this->fWarn->add("This fUser interface does not support changing roles.  Update group membership in the directory for changing roles.");
    return false;
  }

  /**
   * Remove a role from the users Roll list - Not supported!
   * @todo This might be plausible once self::save() is implemented, but generally it is better to use the LDAP 
   *       administration interface to update users; especially in a corporate environment.
   */
  function removeRole($role)
  {
    $this->fWarn->add("This fUser interface does not support changing roles.  Update group membership in the directory for changing roles.");
    return false;
  }
}
?>

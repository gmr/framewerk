<?php // $Id: fLDAPCore.inc 913 2007-08-06 17:35:03Z gmr $
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
 * Class to perform core LDAP functions (connect, query, TLS, etc)
 *
 * @author     Joe Hansche <madCoder@gamesurge.net>
 * @link       http://framewerk.org
 * @since      2006-05-22
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2006-2007 the Framewerk Development Group
 * @version    $Rev: 913 $
 * @package    Toolkit
 * @subpackage LDAP
 * @uses       fSingletonObject
 */
class fLDAPCore extends fSingletonObject
{
  private static $instance;

  // Holds the LDAP connection
  private $ldap;

  // LDAP options
  private $config;
  private $mappings;

  protected function __construct()
  {
    // Call TFramewerkSingletonObject constructor
    parent::__construct();

    $this->fProfile->profile('LDAPConstruct');

    $this->config = new fRegistry( $this );
    if ( ! $this->config )
    {
      throw new Exception('LDAPCore constructed, but no LDAP configuration in configuration.xml (/LDAP/)');
    }

    // Ensure bind user and password are configured
    if ( ! isset($this->config->bind->userdn, $this->config->bind->passwd) && ! $this->config->bind->anonymous )
    {
      throw new Exception ('Trying to instantiate fLDAPCore, but there is no bind user/pass pair in the config (/LDAP/bind/userdn and /LDAP/bind/passwd).');
    }
    if ( ! isset($this->config->dc) || count($this->config->dc) == 0 )
    {
      throw new Exception ('Trying to instantiate fLDAPCore, but there is no LDAP URI in the config (/LDAP/dc/uri). Ex: ldap://fully.qualified.domain.name:389/.  Note: If using MS Active Directory, do not specify an LDAPS:// schema, nor port 686; instead, set /LDAP/dc[@secure=1].');
    }

    // Prepare for LDAP Connection
    foreach($this->config->dc as $dc)
    {
      if ( ! $dc->enabled ) continue;

      if ( ! $conn = ldap_connect($dc->uri) )
      throw new Exception ('Could not connect to LDAP server ('.$dc->uri.').');

      $this->fDebug->add('Connected to LDAP Server', 4);

      // Use StartTLS when encryption is requested
      if ( $dc->secure )
      {
        if ( !$this->startTLS($conn) )
        throw new Exception('LDAP encryption is enabled in config, but StartTLS failed!');
      }

      // Set version number if specified.  PHP's default is version 2.
      if ( isset($dc->version) )
      {
        if ( ldap_set_option($conn, LDAP_OPT_PROTOCOL_VERSION, $dc->version) )
        {
          $this->fDebug->add('Set LDAP protocol version to '.$dc->version, 4);
        } else {
          $this->fWarn->add('Failed to set LDAP protocol version to '.$dc->version);
        }
      }

      // Set referrals option if specified.  For AD, we need '0'
      if ( $dc->referrals )
      {
        if ( ldap_set_option($conn, LDAP_OPT_REFERRALS, $dc->referrals) )
        {
          $this->fDebug->add('Set LDAP referrals option to '.$dc->referrals, 4);
        } else {
          $this->fWarn->add('Failed to set LDAP referrals option to '.$dc->referrals);
        }
      }

      // Try to bind with the configured bind user for pre-authenticated use.
      if ( ! $this->bindUser($conn) )
      {
        throw new Exception ('Binding to the LDAP Directory failed with user='.$this->config->bind->userdn.'.');
      }

      $this->ldap[] = $conn;
      unset($conn);

      $this->fDebug->add('Bound to LDAP Server with generic search access', 6);
    }

    /*
    if ( ! ldap_mod_replace($this->conn, 'CN=ldap search,CN=Users,DC=elp,DC=acs-inc,DC=com', array('lastlogontimestamp' => 0)) )
    echo ldap_error($this->conn);
     */
    $this->fDebug->add('LDAP Core constructed', 4);
    $this->fProfile->profile('LDAPConstruct', 'LDAP Core Constructed');
  }

  /**
   * Ping LDAP to ensure we have at least one open and ready connection.  Does not perform an actual ping to ensure
   * the server is still alive - only checks that we were able to connect to at least one server.
   *
   * @returns boolean True if we have at least one server, false otherwise.
   */
  function pingLDAP()
  {
    if ( count($this->ldap) > 0 )
    {
      return true;
    }
    return false;
  }

  /**
   * Get the static/singleton instance.  Instantiate it first, if it is not already.
   *
   * @return fLDAPCore Static instance of the fLDAPCore object
   */
  static function getInstance()
  {
    if ( self::$instance === NULL )
    {
      self::$instance = new fLDAPCore();
    }
    return self::$instance;
  }

  /**
   * Attempt to bind to the configured bind account on the LDAP directory. //configuration/LDAP/bind/(userdn|passwd)
   * UserDN should be either the distinguished name (CN=..,OU=..,DC=..), or
   *   the LDAP uid of the user.
   * For Active Directory, uid can be either 'DOMAIN\sAMAccountName', or
   *   userPrincipalName (username@fully-qualified.domain.name)
   *
   * @param LDAPConnection Connection to bind with
   * @returns True on success, false on failure.  Failure could indicate bad password, unknown user, or other error.
   */
  function bindUser(&$conn)
  {
    if ( $this->config->bind->anonymous )
    {
      fDebug::getInstance()->add( 'Using anonymous search of LDAP directory', 4 );
      return true;
    }
    return ldap_bind($conn, $this->config->bind->userdn, $this->config->bind->passwd);
  }

  /**
   * buildFQUN - Builds a fully-qualified username, suitable for binding to the LDAP directory
   *
   * @param String Username
   * @return A fully qualified username, in either of these forms: user@domain.name, or DOMAIN\user
   */
  function buildFQUN($user)
  {
    // if we have a fqdn config entry, use $user@f.q.d.n
    if ( $this->config->fqdn != '' )
    {
      return $user . '@' . $this->config->fqdn;
    }
    // if we have a nbdn (netbios), use NBDN\$user
    else if ( $this->config->nbdn != '' )
    {
      return strtoupper($this->config->nbdn) . "\\" . $user;
    }
    return $user;
  }

  /**
   * authenticate - Attempts to bind to the directory for authentication purposes
   * Note:  //config/LDAP/bind[@persist] should be enabled unless you have a specific
   *        reason not to.  Otherwise, binding as a non-privileged user might prevent
   *        certain tasks from being executed properly. It may be disabled to allow the
   *        LDAP core to stay authenticated as the user logging in.  This may not be
   *        possible across sessions, however, as authenticate() is only called during
   *        login
   *
   * @param String Username to bind with
   * @param String Password to bind with
   * @return bool True on success, or False on failure
   */
  function authenticate($user, $passwd)
  {
    $result = false;

    /* We loop through all connections, so that 1) any of the connections can
     * authenticate the user (allowing for multiple *different* directories),
     * and so that 2) if [@persist=0], we need to bind to the new user on *all*
     * connections.
     */
    foreach($this->ldap as $conn)
    {
      if ( ldap_bind($conn, $user, $passwd) && $result == false)
      {
        if ( $this->config->bind['persist'] == '1' )
        {
          // re-bind using the configured binduser instead of the user's account
          $this->bindUser($conn);
          return true;
        } else {
          // Stay bound with the end-user's credentials
          $result = true;
        }
      }
    }
    return $result;
  }

  /**
   * int8ToTimestamp - Converts a MSAD 8-byte timestamp value into a UNIX timestamp.
   *
   * @param Int64 8-byte Integer from a MSAD timestamp attribute.
   * @returns Int UNIX timestamp
   */
  static function int8ToTimestamp($int8)
  {
    $bias = 360;
    $v_secs = $int8/10000000;
    $new = $v_secs - 11644473600;
    return $new;
  }

  /**
   * This function begins TLS encryption on the LDAP connection.  This is only
   * necessary when you want to be able to change fields in Active Directory.
   * Note that SSL (ldaps://, and/or port 686) is becoming phased out and
   * should NOT be used, prefering TLS instead.  TLS connects unencrypted to
   * port 389 first, then negotiates an encrypted channel with the LDAP server.
   *
   * @param LDAPConnection Connection to start TLS negotiations with
   * @returns True on successful TLS negotiation, false otherwise.
   */
  function startTLS(&$conn)
  {
    if ( ldap_start_tls($conn) )
    {
      $this->fDebug->add('LDAP connection is now encrypted with TLS', 4);
      return true;
    }
    $this->fDebug->add('LDAP encryption via TLS failed!', 2);
    return false;
  }

  /**
   * search
   * Performs a search on the ldap directory and returns an array of ldap results
   *
   * @param String LDIF query (for ldap searches)
   * @param Array Optional Attributes we want to request from the server.  Default: all
   * @param String Optional The base point in the directory tree where the search should begin from.  Default: configured in /etc/ldap.conf or /etc/openldap/ldap.conf
   * @param Int Optional Integer index indicating which connection to use.  Default: choose random
   * @returns Mixed Array of results as returned by ldap_get_entries(), or false on failure.
   */
  function search($ldif, $attrs = NULL, $basedn = NULL, $idx = NULL)
  {
    if ( $idx === NULL )
    $idx = intval(rand() % count($this->ldap));

    if ( $attrs !== NULL && is_array($attrs) && count($attrs) > 0 )
    $result = ldap_search($this->ldap[$idx], $basedn, $ldif, $attrs);
    else
    $result = ldap_search($this->ldap[$idx], $basedn, $ldif);

    if ( ! $result )
    {
      $extra = '';
      if ( ldap_errno($this->ldap[$idx]) === 1 )
      {
        $extra = '  Note that this error can be caused if searching an Active Directory LDAP server, and "version" is not set to 3, and/or "referrals" is not set to 0!';
      }
      $this->fWarn->add('An LDAP error occurred while searching the directory: [' . ldap_errno($this->ldap[$idx]) . '] ' . ldap_error($this->ldap[$idx]) . '.' . $extra);
      return false;
    }
    try
    {
      $entries = ldap_get_entries($this->ldap[$idx], $result);
    } catch(Exception $e) {
      print_r($e);
    }
    return $entries;
  }

  /**
   * Fetches an attribute mapping, as configured in //config/LDAP/mappings/attributes
   *
   * @param String Framewerk-related attribute name
   * @returns String Configured implementation mapping for this attribute.  If not found, returns given parameter.
   */
  function getAttribute($attr)
  {
    // Loop through all attribute mappings for this implementation
    foreach($this->config->attributes as $attrs)
    {
      if ( $attrs->implements != $this->config->type )
      {
        continue;
      }

      foreach( $attrs->attribute as $attribute )
      {
        if ( $attribute->key == $attr )
        {
          return $attribute->name;
        }
      }
    }
    // Just use the default/given attribute if no mapping is provided
    return $attr;
  }

  /**
   * Processes the attribute mapping configuration, and saves the mappings.  Renames attributes from
   * implementation-specific name, to a universal Framewerk-usable name.
   *
   * @param Array LDAP results
   * @returns Array LDAP results with attribute mappings replaced.
   */
  function processAttributes($info)
  {
    for($i=0; $i<$info[0]['count']; $i++)
    {

      // Name of the LDAP attribute
      $name = $info[0][$i];

      if ( isset( $this->mappings['tmp'][$name] ) )
      {
        // Cache of the XML attribute mapping for this
        $tmp = $this->mappings['tmp'][$name];

        // The new "universal" name of the mapped attribute
        $newname = strval($tmp['name']);
      } else {
        $newname = $name;
      }

      // The existing value from the LDAP directory
      $entry = $info[0][$name];

      // If the attribute mapping has a conditional <test> element...
      if ( isset($tmp->test) )
      {
        // Prepare the conditional test string
        $test = 'return (' . str_replace('$1', $entry[0], $tmp->test['condition']) . ');';

        // Set the default value, if provided
        if ( isset($tmp->default['value']) )
        $entry[0] = strval($tmp->default['value']);

        // Evaluate the condition, and set the value on success
        if ( eval($test) == true )
        $entry[0] = strval($tmp->test['value']);
      }

      // Remove the old (implementation-specific) value from the ldap result
      unset($info[0][$name]);

      // Set the new attribute into the ldap result
      $info[0][$newname] = $entry;
      $info[0][$i] = $newname;
    }
    return $info;
  }

  /**
   * getRoles
   * Go through the list of LDAP groups, and map the groups to their respective roles
   *  as defined in configuration.xml at //LDAP/mappings/permissions/
   *
   * @param Array of "friendly" group names (sans CN=/OU=/DC= parts)
   * @returns Array of roles, for use in getPermissionsFromRoles()
   * @see getPermissionsFromRoles()
   */
  function getRoles($groups)
  {
    return false;
    $roles = array();
    foreach ($groups as $group)
    {
      list($g) = split(', ', $group);
      $groupUsed = false;
      foreach($this->config->mappings->permissions->group as $map)
      {
        if ( $g == $map['name'] )
        {
          $groupUsed = true;
          foreach($map->role as $role)
          {
            $roles = array_merge($roles, $this->getNestedRoles(strval($role)));
          }
        }
      }
      if ( $groupUsed === false )
      {
        $roles[] = $g;
      }
    }
    return array_unique(array_values($roles));
  }

  /**
   * getNestedRoles
   * Resolve nested roles (follow each role's parent until parent is null), from within the PDO tuser_roles table.
   *
   * @param String role name that we should drill up from
   * @returns Array of roles, including parent roles
   * @see getRoles()
   */
  function getNestedRoles($role)
  {
    return false;
    $roles = array();
    $pdo = $this->TFramewerk->PDO;
    $query = $pdo->prepare("SELECT a.*,b.name as parentrole FROM tuser_roles a LEFT OUTER JOIN tuser_roles b ON (a.parent = b.id) WHERE a.name=:role;");
    $query->execute(array(':role'=>$role)) or die(print_r($query->errorInfo()));
    for ( $i=0; $i<$query->rowCount(); $i++ )
    {
      $data = $query->fetch(PDO::FETCH_OBJ);
      $roles[] = $data->name;
      if ( $data->parentrole != '' )
      {
        $roles = array_merge($roles, $this->getNestedRoles($data->parentrole));
      }
    }
    return $roles;
  }

  /**
   * getPermissionsFromRoles
   * Iterate through the list of roles, and evaluate any corresponding permissions related to that role
   *
   * @param Array of roles with nested parents resolved (result of getNestedRoles())
   * @return Array of Permissions
   * @see getPermissions()
   */
  function getPermissionsFromRoles($roles)
  {
    return false;
    $permissions = array();
    $pdo = tPDO::getInstance();
    foreach($roles as $role)
    {
      $query = $pdo->prepare("SELECT permission,parent FROM tuser_rolepermissions a JOIN tuser_roles b ON (a.roleid=b.id) WHERE b.name=:role;");
      if ( ! $query->execute(array(':role'=>$role)) )
      {
        throw new Exception("Error in SQL query!" . print_r($query->errorInfo(),true));
      }
      for ( $i=0; $i<$query->rowCount(); $i++)
      {
        $data = $query->fetch(PDO::FETCH_OBJ);
        $permissions[] = $data->permission;
      }
    }
    return array_unique(array_values($permissions));
  }

  /**
   * getPermissions
   * Get a full list of permissions, given an array of groups
   *
   * @param Array of group memberships for this user, as specified in the (mapped) LDAP 'group' attribute
   * @return Array of permissions held by the user
   */
  function getPermissions($groups)
  {
    $groups = array_values( $groups );
    return $groups;
    return $this->getPermissionsFromRoles($this->getRoles($groups));
  }

  /**
   * getNestedGroups
   * Resolve nested group memberships within the LDAP directory.
   *
   * @param Array of direct group memberships
   * @return Array of parent/nested group memberships (all groups this user is a member of)
   */
  function getNestedGroups($groups)
  {
    return false;
    $perms = array();
    for ( $i=0; $i<$groups['count']; $i++ )
    {
      $g = $groups[$i];
      $perms[] = ldap_dn2ufn($g);
      $nest = $this->search("(member=$g)", array('dn'));
      foreach($nest as $newgroup)
      {
        $perms[] = ldap_dn2ufn($newgroup['dn']);
      }
    }
    return array_unique(array_values($perms));
  }

  /**
   * getUserInfo
   * Gather information about the user from the LDAP directory
   *
   * @param $key Unique user ID of a user in the directory
   * @return stdClass object containing the information expected for a user
   */
  function getUserInfo($key)
  {
    $this->fDebug->add("Getting user info for '$key'...", 6);

    $info = $this->search( '(' . $this->getAttribute( 'id' ) . '=' . $key . ')', array( $this->getAttribute( 'username' ), $this->getAttribute( 'email' ), $this->getAttribute( 'status' ), $this->getAttribute( 'firstName' ), $this->getAttribute( 'lastName' ) ) );

    $info = $this->processAttributes($info);

    if ( $info['count'] != 1 )
    {
      $this->fWarn->add("Could not find any matches for this username: " . $key);
      return false;
    }

    // Fix the memberOf attribute (follow nested group memberships, to obtain a FULL list of groups)
    $info[0]['groups'] = $this->findGroupMemberships( $info[0] );

    $data = new stdClass();
    $data->username = $info[0][ $this->getAttribute( 'username' ) ][0];
    $data->fullName = $info[0][ $this->getAttribute( 'firstName' ) ][0] . ' ' . $info[0][ $this->getAttribute( 'lastName' ) ][0];
    $data->email = $info[0][ $this->getAttribute( 'email' ) ][0];
    $data->permissionList = $this->getPermissions($info[0]['groups']);
    $data->status = $info[0][ $this->getAttribute( 'status' ) ][0];

    // We need to gather this attribute from ALL dc's in an MSAD environment
    $data->lastActivity = $this->getLastActivity($data->username);
    if ( $data->lastActivity > 1 ) $data->lastActivity = date('Y-m-d H:i:s', $data->lastActivity);

    return $data;
  }

  function findGroupMemberships( $info )
  {
    $groupSearch = $this->config->mappings->groups->search;
    $groups = array();

    foreach( $groupSearch as $search )
    {
      $attrs = array( );
      $attrs[] = '(objectClass=' . $search->objectClass . ')';
      $value = $info[$search->searchAttribute];
      if ( $search->searchAttribute != 'dn' )
      {
        $value = $value[0];
      }
      if ( ! $value )
      {
        continue;
      }
      $attrs[] = '(' . $search->attribute . '=' . $value . ')';
      $ldif = '(&' . join( '', $attrs ) . ')';
      $result = $this->search( $ldif, array( $search->groupNameAttribute ) );
      foreach( $result as $tempGroup )
      {
        $name = $tempGroup[ $search->groupNameAttribute ][0];
        if ( ! $name )
        {
          continue;
        }

        $groups[ $tempGroup['dn'] ] = $name;
      }
    }
    $groups = $this->resolveNestedGroups( $groups );
    $groups = array_values( $groups );
    natcasesort( $groups );
    return $groups;
  }
  
  function resolveNestedGroups( $groups )
  {
    $initialSize = count( $groups );
    foreach( $groups as $dn => $name )
    {
      $ldif = '(&(member=' . $dn . ')(objectClass=groupOfNames))';
      $result = $this->search( $ldif, array( 'cn' ) );
      if ( $result['count'] == 0 )
      {
        continue;
      }
      foreach( $result as $tempGroup )
      {
        if ( ! isset( $tempGroup[ 'cn' ][0] ) )
        {
          continue;
        }
        $groups[$tempGroup['dn']] = $tempGroup[ 'cn' ][0];
      }
    }
    if ( $initialSize != count( $groups ) )
    {
      $groups = $this->resolveNestedGroups( $groups );
    }
    return $groups;
  }
  
  /**
   * getLastActivity
   * Returns a UNIX timestamp of the last time the user logged on.  This function must iterate ALL
   * domain controllers, unlike other attributes which are regularly replicated between DCs.
   * NOTE: Do not modify this to use the lastLogonTimestamp attribute for MSAD, because that can be
   * as out-dated as 1 week!  For aged-account checking of >1w, lastLogonTimestamp should be fine.
   *
   * @param String The user's unique user ID
   * @return a UNIX timestamp representing the time the user last logged on to a domain controller
   * @todo Abstract the attribute names, as samaccountname and lastlogon are MSAD-specific attributes
   */
  function getLastActivity($user)
  {
    $lastlogon = 0;
    foreach($this->ldap as $i => $conn)
    {
      $info = $this->search("(samaccountname=$user)", array('lastlogon'), NULL, $i);
      $lastlogon = max($lastlogon, $info[0]['lastlogon'][0]);
    }
    return $this->int8ToTimestamp($lastlogon);
  }
}

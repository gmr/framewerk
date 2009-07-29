<?php // $Id: fSession.inc 8486 2007-12-20 15:22:22Z jhansche $
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
 * Session Management Class
 *
 * This object is responsible for keeping track of all the information about the session
 *
 * @author     Gavin M. Roy <gavinmroy@gmail.com>
 * @link       http://framewerk.org
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2004-2007 the Framewerk Development Group
 * @version    $Rev: 8486 $
 * @package    Engine
 * @subpackage Core
 * @since      2004-02-23
 * @uses       fSingletonObject
 */

class fSession extends fSingletonObject
{
  static private $instance = NULL;

  private   $authenticated, $dataStore;
  public    $fUser;
  private   $store;
  private   $registry;
  private   $objectPersistVariables;
  private   $objectAuthedVariables;
  private   $saveHashUserID, $saveHash;
  public    $country, $id, $ipaddress, $language, $logTraffic, $message, $starttime, $uid, $uriValues;
  public    $mybData;
  public    $ignore = false;

  /**
   * Initialization / constructor Routine
   */
  protected function __construct()
  {
    // Call fMainSingletonObject parent constructor
    parent::__construct();

    // Start our profiling timer
    $this->fProfile->profile( 'SessionConstruct' );

    // Get our configuration data
    $this->registry = new fRegistry($this);

    // Get our session data store object name
    $this->dataStore = $this->registry->handler->class;

    // by default we're not logged into the session
    $this->authenticated = false;

    // Set string values of our cookie settings
    $cookiename = $this->registry->cookie->name;
    $cookieduration = $this->registry->cookie->duration;

    // if the configuration for autodomain is set then use it
    if ( $this->registry->cookie->autodomain == true )
    {
      $cookiedomain = $_SERVER['SERVER_NAME'];
    } else {
      $cookiedomain = $this->registry->cookie->domain;
    }

    // Set our session ID if it's not already there.
    $this->fDebug->add("Checking for Cookie: ".$cookiename, 4);

    if ( !isset($_COOKIE[$cookiename]) )
    {
      // set our sessions unique id based upon the php's unique id feature
      $this->fDebug->add("Cookie not found.", 6);
      $this->id = md5(uniqid(rand(), true));
    } else 
    {
      $this->fDebug->add("Cookie found.", 6);
      $this->id = $_COOKIE[$cookiename];
    }

    // Send our session cookie
    if ( $cookieduration == 0 )
    {
      $cookieTime = 0;
    } else {
      $cookieTime = time() + $cookieduration;
    }
    if ( setCookie($cookiename, strval($this->id), $cookieTime, "/", $cookiedomain) )
    {
      $this->fDebug->add("Cookie set on client (".$this->id." on ".$cookiedomain.")", 6);
    } else {
      $this->fDebug->add('SetCookie failed!', 6);
    }

    // Construct our Session Store
    $this->store = new $this->dataStore();
    $data = $this->store->loadSession($this->id);
    
    $this->fDebug->add('Session Data:' . print_r($data, true), 6);
    if ( is_array($data) )
    {
      foreach ( $data AS $field => $value )
      {
        if ( $field == 'authenticated' )
        {
          if ( $value == 'true' )
          {
            $this->authenticated = true;
          } else {
            $this->authenticated = false;
          }
        } else {
          if ( $field != 'mybData' && array_search($field, $this->validEntities) === false )
          {
            $this->validEntity($field);
          }
          if ( isset($this->$field) == NULL )
          {
            $this->$field = $value;
          }
        }
        $this->fDebug->add('Session loading ' . $field . ': ' . $value, 6);
      }
    } else {
      $this->fDebug->add("No session data found (either new session, or error during load)", 4);
    }

    // Process the remember me cookie
    if ( $this->authenticated == false && isset($_COOKIE['rememberme']) )
    {
      $hash = explode(':', $_COOKIE['rememberme']);
      if ( is_numeric($hash[0]) )
      {
        $this->fDebug->add("'Remember Me' cookie was found with hash '$hash'.", 6);
        $this->saveHashUserID = $hash[0];
      }
    }

    // Prune stale sessions
    if ( $this->registry->cleanup == true )
      $this->store->pruneStaleSessions();

    // Set a default session start time and ip address if it's not already set
    if ( !isset($this->starttime) ) $this->starttime = time();
    if ( !isset($this->ipaddress) ) $this->ipaddress = $_SERVER['REMOTE_ADDR'];

    /*
    if ( !isset($this->language) )
    {
      foreach ( $this->fMain->configuration->engine->i18n->languages->language AS $language )
      {
        if ( $language['default'] )
        {
          $this->language = strval($language['code']);
          break;
        }
      }
    }
    */
    if ( !is_array($this->objectPersistVariables) ) $this->objectPersistVariables = array();
/*
    // Get the IP2Country information
    if ( $this->registry->ip2country->enabled == true )
    {
      $ip2country = new fIP2Country($this->fMain);
      $this->country = $ip2country->getCountry(2);
      unset($ip2country);
    }

    // Optional IP->Session Enforcement Checking
    if ( $this->registry->session->enforceIPAddr == true )
    {
      if ( $_SERVER['REMOTE_ADDR'] != $this->ipaddress )
      {
        // Delete out the previous cookie
        SetCookie($cookiename, "", time() - $cookieduration, "/", $cookiedomain);
        // End our execution
        throw new fHTTPException(403, "Session Doesn't Match your IP Address.  Possible Session Hijack Detected.");
      }
    }
*/
    $this->fProfile->profile("SessionConstruct", "Session Constructed");
  }

  function loginLogoutLink()
  {
    if ( is_callable(array($this->store, 'loginLogoutLink')) )
    {
      return $this->store->loginLogoutLink();
    }
    if ( $this->isAuthenticated() )
    {
      return 'xxSettings Log out';
    }
    return 'xxLog in Signup';
  }

  /**
   * Return a formatted session age
   *
   * @return String formatted session age
   */
  function getAge()
  {
    $age = ( time() - intval($this->starttime) );
    $seconds = $age;
    $time = $seconds." {!i18n:seconds:".$seconds."}";

    $minutes = (int) ( $age / 60 );
    if ( $minutes > 0 )
    {
      $seconds = (int) fmod($age, 60);
      $time = $minutes." {!i18n:minutes:".$minutes."}, ".$seconds." {!i18n:seconds:".$seconds."}";
    }

    $hours = (int) ( $minutes / 60 );
    if ( $hours > 0 )
    {
      $minutes = (int) fmod($minutes, 60);
      $time = "$hours {!i18n:hours:$hours}, $minutes {!i18n:minutes:$minutes}, $seconds {!i18n:seconds:$seconds}";
    }

    $days = (int) ( $minutes / 60 );
    if ( $days > 0 )
    {
      $hours = (int) fmod($hours, 60);
      $time = "$days {!i18n:days:$days}, $hours {!i18n:hours:$hours}, $minutes {!i18n:minutes:$minutes}, $seconds {!i18n:seconds:$seconds}";
    }

    return $time;
  }

  /**
	 * Return the variable array for an object
	 *
	 * @param Object $object
	 * @return array
   */
  function getAuthedVariables($object)
  {
    if ( is_object($object) )
    {
      $objectName = get_class($object);
    } else {
      $objectName = $object;
    }
    if ( !$objectName )
    {
      trigger_error('Error setting object variables for ' . $object, E_USER_WARNING);
    }
    return $this->objectAuthedVariables[$objectName];
  }

  /**
   * Checks to see if the object is instanced in the internal static variable $instance and if not, does so
   *
   * @return fSession instance
   */
  static function getInstance()
  {
    if ( !isset(self::$instance) )
    {
      self::$instance = new fSession();
    }
    return self::$instance;
  }

  /**
   * Return a list of available languages
   *
   * @todo this should be it's own object I think - gmr 2007-01-24
   * @return string
   */
  function getLanguageList()
  {
    /*
    $fTheme = fTheme::getInstance();
    $dom = new DOMDocument();
    $dom->formatOutput = true;

    $node = $dom->importNode(dom_import_simplexml($this->fMain->configuration->engine->i18n->languages), true);
    $node->setAttribute('current', $this->language);

    return html_entity_decode($fTheme->transformXML($fTheme->xslFile('LanguageList'), $dom->saveXML($node)));
    */
  }

  /**
   * Check to see if the passed in language is valid
   *
   * @todo this should be it's own object I think - gmr 2007-01-24
   * @param string $code
   * @return bool
   */
  function validLanguage($code = NULL)
  {
    return true;
    /*
    foreach( $this->fMain->configuration->engine->i18n->languages->language as $language )
    {
      if ( strval($language['code']) == $code )
      {
        return true;
      }
    }
    return false;
    */
  }

  /**
   * Return the variable array for an object
   *
   * @param Object $object
   * @return array
   */
  function getPersistVariables($object)
  {
    if ( is_object($object) )
    {
      $objectName = get_class($object);
    } else {
      $objectName = $object;
    }
    if ( !$objectName )
    {
      trigger_error('Error setting object variables for ' . $object, E_USER_WARNING);
    }
    return $this->objectPersistVariables[$objectName];
  }

  /**
   * Return the session id
   *
   * @return String session id
   */
  function getSessionID()
  {
    return $this->id;
  }

  /**
   * Get a session token. The data store will handle storing the value
   *
   * @return String session token
   */
  function getToken()
  {
    if ( is_callable(array($this->store, 'getToken')) )
    {
      return $this->store->getToken();
    } else {
      return false;
    }
  }

  /**
   * Counts the number of active sessions as determined by visitor_time_threshold
   *
   * @return int Visitor Count
   */
  function getVisitors()
  {
    return intval($this->store->counfSessions());
  }

  /**
   * Counts the number of active authenticated sessions
   *
   * @return int Authenticated Visitor Count
   */
  function getVisitorsAuthed()
  {
    return intval($this->store->countAuthedSessions());
  }

  /**
   * Counts the number of active authenticated sessions
   *
   * @return int Authenticated Visitor Count
   */
  function getVisitorsGuests()
  {
    return intval($this->store->countGuesfSessions());
  }

  /**
   * If we have a valid user assigned to the session, run through and set all the variables
   */
  function initializeUser()
  {
    // If we have a uid construct a fUser Object for this user.  This gets done *after* fSession is constructed.
    if ( ( isset($this->uid) ) && ( !is_object($this->fUser) ) )
    {
      $this->fDebug->add("Member ID #" . $this->uid . " found, creating fUser object", 6);
      $this->fUser = new fUser($this->uid);
    }
    elseif ( $this->saveHashUserID !== NULL )
    {
      try
      {
        $this->fUser = new fUser($this->saveHashUserID);
        if ( $this->fUser->authenticateAgainstHash($this->saveHash) )
        {
          $this->fDebug->add("Found a user (" . $this->fUser->id . ") for a 'Remember Me' cookie.", 2);
          $this->fUser->lastActivity = strftime('%X %x');
          $this->authenticated = true;
          $this->uid = $this->fUser->id;
        } else {
          $this->fDebug->add("No user was found matching the saved hash.",4);
          setcookie('rememberme', NULL, time() - 3600, '/');
          unset($_COOKIE['rememberme']);
        }
      } catch ( Exception $e ) {
        $e;
        $this->fDebug->add("No user was found matching the saved hash.",4);
        setcookie('rememberme', NULL, time() - 3600, '/');
        unset($_COOKIE['rememberme']);
      }
      unset($this->saveHashUserID);
    }
  }

  /**
   * Return the state of our user and if they are logged into the site or are anonymous
   *
   * @return bool
   */
  function isAuthenticated()
  {
    return $this->authenticated;
  }

  /**
   * Checks to see if the object is instanced in the internal static variable $instance.
   *
   * @return Bool True if the object has an instance already created.  False otherwise.
   */
  static function isInstanced()
  {
    return ( self::$instance !== NULL );
  }

  /**
   * Check to see if a requested language is valid
   *
   * @param string $code
   * @return bool
   */
  function isValidLanguage($code = NULL)
  {
    return true;
    /*
    foreach( $this->fMain->configuration->engine->i18n->languages->language as $language )
    {
      if ( strval($language['code']) == $code )
      {
        return true;
      }
    }
    return false;
    */
  }

  /**
   * Login function
   * @todo determine if the newly removed uid variable was needed
   */
  function login($key, $password)
  {
    // fUser will return an exception if the $key is not found
    try
    {
      $this->fUser = new fUser($key);
    } catch ( fUserException $e ) {
      if ( ($e->getCode() & E_fUser_INFO) > 0 )
      {
        $this->fMain->fWarn->add('Exception in fUser Backend for key "'.$key.': '.$e->getMessage());
        return false;
      }
      throw $e;
    }
    $password = stripslashes($password);
    if ( $this->fUser->authenticate($password) )
    {
      $this->fUser->lastActivity = strftime('%X %x');
      $this->authenticated = true;
      $this->uid = $this->fUser->id;
      return true;
    }
    return false;
  }

  /**
   * Log the user out of the session
   */
  function logout()
  {
    unset($this->fUser);
    setcookie('rememberme', NULL, time() - 3600, '/');
    $this->authenticated = false;
    $this->uid = NULL;
    $this->objectAuthedVariables = array();
  }

  /**
   * Save our Session Data
   */
  function save()
  {
    $this->fDebug->add("Saving the session data.", 4);

    $this->fProfile->profile("SessionSave");

    $variables = array();
    $variables['authenticated'] = $this->isAuthenticated();
    $variables['country'] = $this->country;
    $variables['ipaddress'] =$this->ipaddress;
    $variables['language'] = $this->language;
    $variables['uid'] = $this->uid;
    $variables['message'] = $this->message;
    $variables['starttime'] = $this->starttime;
    $variables['objectPersistVariables'] = $this->objectPersistVariables;
    $variables['objectAuthedVariables'] = $this->objectAuthedVariables;

    $variables['mybData'] = $this->mybData;

    if ( is_object($this->store) && $this->store->save($this->id, $variables) )
    {
      $this->fDebug->add("Session saved", 2);
      $this->fProfile->profile("SessionSave", "saveSession (Session Saved)");
      return true;
    } else {
      $this->fDebug->add("Couldn't save the session", 2);
      $this->fProfile->profile("SessionSave", "saveSession (Couldn't Save)");
      return false;
    }
  }

  /**
   * Set objectAuthedVariables variables
   * objectAuthedVariables expire with authentication
   *
   * param Object Object to store
   * param Array variable array
   */
  function setAuthedVariables($object, $variables)
  {
    if ( is_object($object) )
    $objectName = get_class($object);
    else
    $objectName = $object;
    if ( !$objectName )
    trigger_error('Error setting object variables for ' . $object, E_USER_WARNING);
    $this->objectAuthedVariables[$objectName] = $variables;
  }

  /**
   * Set objectPersistVariables variables
   * objectPersistVariables persist across authentication
   *
   * param Object Object to store
   * param Array variable array
   */
  function setPersistVariables($object, $variables)
  {
    if ( is_object($object) )
    {
      $objectName = get_class($object);
    } else {
      $objectName = $object;
    }
    if ( !$objectName )
    {
      trigger_error('Error setting object variables for ' . $object, E_USER_WARNING);
    }
    $this->objectPersistVariables[$objectName] = $variables;
  }

  /**
   * Destructor
   * Save out our session data when we're all done
   */
  function __destruct()
  {
    $this->save();
  }
}
?>

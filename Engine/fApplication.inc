<?php // $Id: fApplication.inc 8507 2007-12-20 19:38:42Z jhansche $
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
 * fApplication
 *
 * Contains the fApplication abstract class
 */
/**
 * Interface abstract object
 *
 * This abstract fApplication object is inherited by all Framewerk Interfaces
 * Interfaces are the applications in Framewerk which are responsible for the
 * direct interaction with the user, that is Interfaces are the web applications
 * themselves.
 *
 * @author     Gavin M. Roy <gavinmroy@gmail.com>
 * @link       http://framewerk.org
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2004-2007 the Framewerk Development Group
 * @version    $Rev: 8507 $
 * @package    Engine
 * @subpackage Abstract_Applications
 * @since      2004
 */

abstract class fApplication
{
  public    $blockTitle;
  /**
   * @var fDebug
   */
  protected $fDebug;
  /**
   * @var fProfile
   */
  protected $fProfile;
  /**
   * @var fMessages
   */
  protected $fMessages;
  /**
   * @var fMain
   */
  protected $fMain;
  /**
   * fOutput Instance
   *
   * @var fOutput
   */
  protected $fOutput;
  /**
   * @var fSession
   */
  protected $fSession;
  /**
   * fTheme Instance
   *
   * @var fTheme
   */
  protected $fTheme;
  /**
   * @var fUser
   */
  protected $fUser;
  /**
   * @var fjQuery
   */
  protected $jQuery;

  protected $process;
  protected $uriValues;
  protected $baseURI;

  /**
   * Interface Constructor
   *
   * Sets the all the needed internal variables and engine parameters.
   *
   * @param object fMain object is required
   */
  function __construct()
  {
    // Set our objects and our object shortcuts
    $this->fDebug     = fDebug::getInstance();
    $this->fProfile   = fProfile::getInstance();
    $this->fMessages  = fMessages::getInstance();
    $this->fMain      = fMain::getInstance();
    $this->fOutput    = fOutput::getInstance();
    $this->fSession   = fSession::getInstance();
    $this->fTheme     = fTheme::getInstance();
    $this->fUser      = $this->fSession->fUser;

    // First check if the app should run
    if ( ($message = tMaintenance::check()) !== false )
    {
      // Get mime type
      $mime = $this->fOutput->setMimeType(NULL);
      
      if ( 0 === strncmp($mime, 'application/json', 16) || $_SERVER['HTTP_X_REQUESTED_WITH'] == 'XMLHttpRequest' )
      {
        $obj = new stdClass();
        $obj->error = true;
        $obj->maintenance = true;
        $obj->message = $message;
        header('Status: 503 Performing Maintenance');
        echo json_encode($obj);
        exit;
      } else if ( 0 === strncmp($mime, 'text/xml', 8) )
      {
        header('Status: 503 Performing Maintenance');
        echo <<<XML
<?xml version="1.1" encoding="UTF-8"?>
<error>
 <message><![CDATA[$message]]></message>
</error>
XML;
        exit;
      } else
      {
        throw (new fHTTPException(503, $message));
      }
    }

    // Set our default values
    $this->process    = true;
    $this->uriValues  = $this->fSession->uriValues;
    for ( $y = 0; $y < Count($this->uriValues); $y++ )
    {
      $this->uriValues[$y] = str_replace('_', ' ', rawurldecode($this->uriValues[$y]));
    }

    // Set the baseURI
    $this->baseURI = $this->fMain->configuration->site->baseURI . $this->fMain->keyword . '/';
  }

  /**
   * Set the page category
   *
   * @param String Category
   */
  function pageCategory($category)
  {
    $this->fDebug->add('Setting page category:' . $category, 4);
    $this->fOutput->setPageCategory($category);
  }

  /**
   * Set the page name
   *
   * @param String Page Name
   */
  function pageName($name)
  {
    $this->fOutput->setPageName($name);
  }

  /**
   * Wrapper function to make adding output content easier
   *
   * @param string Value to append
   * @param bool Try and run an XSL transformation on it
   * @param string optional XSL file to try and use
   * @return bool Output added or not
   */
  function output($value, $transform = false, $xslFile = NULL)
  {
    // If processing is turned off exit
    if ( ! $this->process ) return false;

    // If we're doing an XSL transformation, do so
    if ( $transform )
    {
      $value = $this->getTransformedXML($value, $xslFile);
      if ( $value === false )
      {
        return false;
      }
    }

    // Add the data to the output buffer
    $this->fOutput->appendContent($value);
    return true;
  }

  function getTransformedXML($value, $xslFile = NULL)
  {
    // If xslfile wasnt set, use the interface name
    if ( $xslFile === NULL )
    {
      $xslFile = $this->fTheme->xslFile($this->fMain->lastInterface);
    } else {
      $xslFile = $this->fTheme->xslFile($xslFile);
    }

    // If we dont have a valid XSL File exit with false after adding a warning
    if ( $xslFile == false )
    {
      trigger_error('fApplication::output called with XSL transformation but no XSL file found', E_USER_WARNING);
      return false;
    }

    // Transform the data
    $value = html_entity_decode($this->fTheme->transformXML($xslFile, $value));

    // Return the transformed XML
    return $value;
  }

  /**
   * The block function is for extending the interface object to support block calls
   *
   * @return string block output
   */
  function block()
  {
    throw new Exception("Attempting to use Interface '" . __CLASS__ . "' as a block, but the Interface does not implement the appropriate method(s).  Contact the Interface author.");
  }

  /**
   * Framewerk Module execution Function
   *
   * This function must be extended by the module or it won't run
   *
   * @return bool true/false if execution of module was successful
   */
  function execute()
  {
    $this->pageName('Module Error');
    $this->pageCategory('Default');
    Throw new Exception("The module you have called has not properly extended the fApplication class");
  }

  /**
   * Require the permission to execute
   *
   * @param string Permission String to Require
   * @return bool User has permission
   */
  function requirePermission($permission = NULL)
  {
    $this->process = ( $permission == 'All' );

    if ( !$this->process && ( $this->fUser instanceOf fUser ) )
      $this->process = $this->fUser->checkPermission($permission);

    if ( $this->process == false ) $this->permissionError($permission);
      return $this->process;
  }

  /**
   * Return an error message and throw an exception
   */
  function permissionError($permission)
  {
    throw new fHTTPException(403, 'You do not have permission to access this object [' . $permission . ']');
  }

  /**
   * Some interface developers might want to use _ as something other than a replacement char for space.
   * Calling this function at the top of your execute function will rebuild the uriValues array without any
   * string replacement of _ to space.
   */
  function ignoreURIUnderscore()
  {
    $this->uriValues = $this->fSession->uriValues;
  }

  /**
   * Return to the referring page
   *
   * @return bool
   */
  public final static function returnToReferrer()
  {
    if ( !strpos($_SERVER['HTTP_REFERER'], $_SERVER['HTTP_HOST']) )
    {
      self::redirect(fMain::getInstance()->configuration->site->baseURI);
    } else {
      self::redirect($_SERVER['HTTP_REFERER']);
    }
    return true;
  }

  /**
   * Retdirect to a uri
   *
   * @param string $uri
   */
  public final static function redirect( $url )
  {
    if ( strlen(trim($url)) == 0 )
    {
      if ( isset($_REQUEST['redirect']) )
        $url = rawurldecode($_REQUEST['redirect']);
      else
        $url = fMain::getInstance()->configuration->site->baseURI;
    }

    // Save the session on a redirect
    fSession::getInstance()->save();

    // If we've sent headers, throw an exception
    if ( headers_sent( $file, $line ) )
      throw new Exception('Trying to redirect when output has already been sent to the browser. Headers sent at "' . $file . '" on line ' . $line);

    header('Location: ' . $url);
    exit;
  }

  /**
   * Redirect to the login page
   * @todo change this to use fSiteProcessor instead of a hardcoded login page
   */
  public final static function redirectLogin()
  {
    if ( strncmp($_SERVER['REQUEST_URI'], fSiteProcessor::siteURI('Login'), strlen(fSiteProcessor::siteURI('Login'))) )
      $redirect = '?redirect=' . fSiteProcessor::siteURI();
    self::redirect(fSiteProcessor::siteURI('Login') . $redirect);
  }
  
  protected final function pushDebugOutput( $return = false )
  {
    $this->fProfile->profile('debugusertype');
    $debug = new stdClass();
    //$debug->pdo = tPDO::getUsageStats();
    //$debug->memcache = fMemcache::getUsageStats();
    $debug->debug = fDebug::getInstance()->outputArray();
    $debug->warn = fWarn::getInstance()->outputArray();
    $this->fProfile->profile('debugusertype', 'Debug Usertype building output');
    $debug->profile = fProfile::getInstance()->outputArray();
    if ( $this->jQuery && $return === false )
    {
      $this->jQuery->addPlugin('myYearbook.DebugUser');
      $this->jQuery->addInit('jQuery.mybDebug('.json_encode($debug).');');
      return true;
    }
    return $debug;
  }
  
}
<?php // $Id: fMain.inc 946 2007-12-24 00:17:50Z gmr $
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
  * fMain
  *
  * This file contains the fMain object
  */
/**
  * Framewerk Base Class
  *
  * This is the base processing class for the fMain Engine.
  * It is responsible for coordinating the use of all the other classes
  * and actions in Framewerk.
  *
  * @author     Gavin M. Roy <gavinmroy@gmail.com>
  * @link       http://framewerk.org
  * @license    http://opensource.org/licenses/bsd-license.php BSD License
  * @copyright  Copyright 2004-2007 the Framewerk Development Group
  * @since      2004-02-23
  * @version    $Rev: 946 $
  * @package    Engine
  * @subpackage Core
  */
class fMain extends fSingletonObject
{
  public $configuration, $baseURI, $fMessages, $fOutput, $fSiteProcessor, $keyword, $lastError = array(), $lastInterface, $messages, $PDO, $site;

  // Object variables
  private static $instance = NULL;
  private $processed = false;

  /**
   * Checks to see if the object is instanced in the internal static variable $instance
   * and if not, does so
   *
   * @return fMain instance
   */
  static function getInstance()
  {
    if ( self::$instance == NULL ) 
    {
      self::$instance = new fMain();
    }
    return self::$instance;
  }

  /**
   * Forcefully unset a singleton instance.
   */
  static function destroy()
  {
    self::$instance = NULL;
  }

  /**
   * Framewerk Object Constructor
   *
   * DO NOT INSTANCE ANY OBJECTS THAT REFERENCE fMain::getInstance() WITHIN THIS FUNCTION
   * OR A NASTY LOOP WILL OCCUR
   */
  protected function __construct()
  {
    // Set our error handler
    set_error_handler('errorHandler');

    // Initialize our Debugging and Profiling Objects
    $this->fDebug   = fDebug::getInstance();
    $this->fProfile = fProfile::getInstance();
    $this->fWarn    = fWarn::getInstance();

    // Read our configuration file
    $this->fProfile->profile('ConfLoad');

    // Load our registry
    $registry = new fRegistry('fMain');
    $this->configuration = $registry;

    // Set our timezone and locale
    putenv('TZ=' . $registry->timezone);
    setLocale(LC_ALL, strval($registry->locale));

    // Add configuration loading and settings that to the profiling time
    $this->fProfile->profile('ConfLoad', 'Configuration Load');

    // Check our URL to make sure it's using the Site's specified URL
    if ( intval($registry->enforceURL) == true )
    {
      $this->fProfile->profile('URLCheck');
      $urldata = parse_url($registry->site->URL);
      if ( StrToLower($urldata['host']) != StrToLower($_SERVER['HTTP_HOST']) )
      {
        Header('Location: ' . $urldata['scheme'] . '://' . $urldata['host'] . $_SERVER['REQUEST_URI']);
        exit();
      }
      $this->fProfile->profile('URLCheck', 'URL Evaluation');
    }

    // Check our system load if max > 0 and exit if required
    if ( $registry->system_load->max > 0 )
    {
      // Make sure people see the penalty for this check
      $this->fProfile->profile('LoadCheck');

      // Get the system load via the UNIX uptime command
      $currentLoad = returnSystemLoad();

      // Compare the load with the Configured Max Load
      if ( $currentLoad > $registry->system_load->max )
      {
        // Exit based upon the configured exit strategy
        switch ( strval($registry->system_load->type) )
        {
          case 'redirect':
            // Redirect to a URL (usually staticly served)
            header('Location: ' .  $registry->system_load->location);
            break;
          case 'message';
            // Output a message to the browser
            echo $registry->system_load->message . "\n";
            flush();
            break;
          case 'fHTTPException':
          default:
            throw new fHTTPException(500,  $registry->system_load->message);
            break;
        }
        exit();
      }
      $this->fProfile->profile('LoadCheck', 'System Load Evalutation');
    }

    // Process our ini file settings
    $this->fProfile->profile('phpiniset');
    foreach ( $registry->php AS $setting )
    {
      foreach ( $setting AS $key => $value )
      {
        if ( ini_set(strval($key), strval($value)) !== false )
        {
          $this->fDebug->add('Setting PHP variable ' . strval($key) . ' => ' . strval($value));
        } else {
          $this->fWarn->add('Error Setting PHP variable ' . strval($key) . ' => ' . strval($value));
        }
      }
    }
    $this->fProfile->profile('phpiniset', 'Processing PHP INI Variables');

    // Assign our baseURI for dictionary lookups of the value
    $this->baseURI = $registry->site->baseURI;

    $this->fDebug->add('fMain constructed', 7);
  }

  /**
   * Initialize Framewerk Object and subobjects
   */
  function process()
  {
    $this->fProfile->profile('fMain-callbacks');
    // Process callbacks
    foreach($this->configuration->callbacks->callback as $callback)
    {
      $class = $callback->class;
      if ( $callback->type == 'singleton' )
      {
        $instance = call_user_func(array($class, 'getInstance'));
      } else {
        $instance = new $class;
      }

      if ( strlen($callback->function) > 0 )
      {
        call_user_func(array($instance, $callback->function));
      }

      if ( $callback->expose === true )
      {
        if ( ! $this->isEntitySet($class) )
        {
          $this->validEntity($class);
        }
        $this->$class = $instance;
      }
    }
    $this->fProfile->profile('fMain-callbacks', 'fMain Callbacks Processed');
    
    // Get any messages from the previous session
    /**
    $this->fProfile->profile('messsages');
    $this->fMessages = fMessages::getInstance();
    $this->fProfile->profile('messages', 'fMessages Initialization');
    **/

    // Initialize the fOutput Object
    $this->fProfile->profile('output');
    $this->fOutput = fOutput::getInstance();
    $this->fProfile->profile('output', 'fOutput Initialization');

    // Try and execute the request
    $this->fProfile->profile('process');

    // Load the site.xml document
    $this->fProfile->profile('sitexml');
    if ( is_file( 'Configuration/site.xml' ) )
    {
      $file = new fFile( 'Configuration/site.xml' );
      $data = $file->read();
      $this->site = simplexml_load_string( $data );
    }
    if ( ( !isset( $this->site ) ) || ( $this->site === false ) ) 
    {
      throw new fHTTPException(500, 'There was an error parsing site.xml');
    }
    $this->fProfile->profile('sitexml', 'Load site.xml');
    $this->fProfile->profile('fSiteProcessor');
    $this->fSiteProcessor = new fSiteProcessor($this->site);
    $this->fProfile->profile('fSiteProcessor', 'Construct fSiteProcessor');

    // If we've got a framewerk instance in a sub dir, make the uri to process without the prefix of the uri
    $start = strpos($_SERVER['REDIRECT_URL'], strval($this->configuration->site->baseURI));
    if ( $start !== false )
    {
      // start our new uri at the end of our baseURI
      $start += strlen(strval(strval($this->configuration->site->baseURI)));
      $processURI = '/' . substr($_SERVER['REDIRECT_URL'], $start, strlen($_SERVER['REDIRECT_URL']) - $start);
    } else {
      // just a normal uri for /
      $processURI = $_SERVER['REDIRECT_URL'];
    }
    $this->fProfile->profile('fSiteProcessorProcess');
    $processed = $this->fSiteProcessor->process($processURI);
    $this->fProfile->profile('fSiteProcessorProcess', 'fSiteProcessor->process()');
    

    // If we have not mapped the URL to an object, now run through the default handlers
    if ( !$processed )
    {
      $this->fProfile->profile('defaultHandlers');
      // set our processing keyword
      $values = explode('/', trim($processURI, '/'));
 	    $this->keyword = array_shift($values);
 	    $this->fSession->uriValues = $values;
      $this->fDebug->add('Could not find the interface for "' . $this->keyword . '", reverting to default handlers', 3);
      foreach ( $this->configuration->defaultHandlers->application as $handler )
      {
        $this->fProfile->profile('Handler-' . $handler);
        $handler = strval($handler);
        $this->fDebug->add('Attempting Handler: ' . $handler . ' passing key "' . $this->keyword . '".', 3);
        $interface = new $handler($this->keyword);

        // See if our content has been updated, if there's a HTTP_IF_MODIFIED_SINCE header and a modified function in the class
        if ( ( $this->configuration->allowCaching == true ) && ( isset($_SERVER['HTTP_IF_MODIFIED_SINCE']) ) && ( is_callable(array($interface, 'modified'))) )
        {
          if ( $interface->modified($this->keyword, strtotime($_SERVER['HTTP_IF_MODIFIED_SINCE'])))
          {
            $processed = $interface->execute($this->keyword);
          } else {
            // Data hasn't been updated, send a 304
            header('HTTP/1.0 304 Not Modified');
            $processed = true;
          }
        } else {
          $processed = $interface->execute($this->keyword);
        }
        $this->fProfile->profile('Handler-' .$handler, 'Default Handler Execution');
        if ( $processed ) break;
        unset($interface);
      }
      $this->fProfile->profile('defaultHandlers', 'Tried default handlers');
    }

    if ( !$processed )
    {
      throw new fHTTPException(404, 'File Not Found');
    }

    // Save our engine messages
    //$this->fMessages->save();
    $this->fProfile->profile('process', 'fMain->process() Container Profile');

    // Send the output
    $this->fOutput->sendOutput( );
  }

}

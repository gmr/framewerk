<?php // $Id: fParser.inc 938 2007-09-10 21:12:37Z gmr $
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
 * Content Parsing Class
 *
 * This is the Framewerk system parser that is responsible for variable and internationalization support.
 * This object is dependent on other objects coming down from the $framewerk object for it's execution.
 *
 * @author     Gavin M. Roy <gavinmroy@gmail.com>
 * @author     R. Brenton Strickler <def@unfit.net>
 * @link       http://framewerk.org
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2004-2007 the Framewerk Development Group
 * @since      2004
 * @version    $Rev: 938 $
 * @package    Engine
 * @subpackage Core
 */

class fParser
{

  private static $instance = NULL;

  private $atDictionary;
  private $dictLoaded;
  private $cache;
  private $i18n;
  private $maxDepth;
  private $parseDepth;
  private $tagPattern;
  private $fMain;
  private $fDebug;

  /**
   * Sets internal variable values
   */
  protected function __construct()
  {
    // Start profiling
    fProfile::getInstance()->profile("ParserConstruct");

    // Init our variables
    $this->cache = array();
    $this->dictLoaded = false;
    $this->tagPattern = '/({(@|%|!i18n:)([^{}]*((?1)[^{}]*)*)})/'; //allows for recursive subtags

    // Set our external objects
    $this->fMain  = fMain::getInstance();
    $this->fDebug = fDebug::getInstance();
    $this->i18n = fi18n::getInstance();
    $this->i18n->setLanguage(fSession::getInstance()->language);

    $registry = new fRegistry('fParser');
    $this->maxDepth = $registry->maxDepth;

    // Add profiling call
    fProfile::getInstance()->profile("ParserConstruct", "Parser Constructed");
  }

  /**
   * Get the current or new instance
   *
   * @return Object instance of this object
   */
  static function getInstance()
  {
    if ( self::$instance == NULL )
    {
      self::$instance = new fParser();
    }
    return self::$instance;
  }

  /**
   * High level parse function
   *
   * @return String $output
   */
  function parse($data)
  {
    fProfile::getInstance()->profile("Parse");
    $this->parseDepth = 0;
    $output = preg_replace_callback($this->tagPattern, array($this, '_parse'), $data);
    fProfile::getInstance()->profile("Parse", "Parsed Content");
    return $output;
  }


  /**
   * Tag explode function that gives special care to ':'
   *
   * @return Array $values
   */
  private function explodeVariable($string)
  {
    $values = array();
    $slashes = 0;

    $i = 0;
    for( $j = 0; $j < strlen($string); $j++ )
    {
      switch($string[$j])
      {
        case '\\';
        $slashes=($slashes+1)%2;
        break;
        case ':';
        if($slashes==0)
        {
          $values[] = substr($string, $i, $j - $i);
          $i = $j + 1;
        }
        $slashes = 0;
        break;
        default:
          $slashes = 0;
          break;
      }
    }
    $values[] = substr($string, $i, $j - $i);

    //strip slashes
    foreach($values as $i=>$value)
    {
      $values[$i] = preg_replace('/\\\\([\\\\:])/', '$1', $value);
    }

    // Return the array
    return $values;
  }


  /**
   * Internal recursive parser function
   *
   * @param String $data - Input data to parse
   * @return String Parsed Data
   */
  private function _parse($data)
  {
    // Set our variables
    $tag      = $data[1];   //$data[1] = '{(@|!i18n:)variable}';
    $type     = $data[2];   //$data[2] = '(@|!i18n:)';
    $variable = $data[3];   //$data[3] = 'variable';
    $subtag   = (isset($data[4]) ? $data[4] : null);   //$data[4] = set if subtags occur

    if ( $this->parseDepth >= $this->maxDepth )
    {
      return $tag;
    }

    $this->parseDepth++;

    if ( $subtag != '' )
    {
      $variable = preg_replace_callback($this->tagPattern, array($this, '_parse'), $variable);
    }

    $returnData = 'Undefined';

    // Optional per variable caching control
    $cache = true;

    // Check the Cache
    if ( isset($this->cache[$tag]) )
    {
      $returnData = $this->cache[$tag];
      $this->fDebug->add("Using cache for " . ( isset($var['type']) ? $var['type'] : '[Unknown Type] ' ) . $variable);
    } else {
      // Variable isn't in the cache
      switch ($type)
      {
        // @ variables which are parsed from our dictionary definitions.
        case '@':
          // Load our dictionary if it's not loaded
          if ( $this->dictLoaded == false )
          {
            $file = new fFile('Configuration/dictionary.xml');
            $temp = simplexml_load_string($file->read());
            $this->atDictionary = array();
            //index the dictionary for quick parsing
            foreach ( $temp->dictionary->variable as $value )
            {
              $key = $value->name;
              foreach ( $value as $name=>$setting )
              {
                $this->atDictionary["$key"]["$name"]=(string) $setting;
              }
            }
            unset($temp);
            $this->dictLoaded = true;
          }

          if ( strstr($variable, ":") )
          {
            $values = $this->explodeVariable($variable);
            $variable = array_shift($values);
          } else {
            $values = array();
          }

          $this->fDebug->add("Attempting to parse variable: $variable", 9);

          // Is this an at variable in the dictionary?
          if ( isset($this->atDictionary[$variable]) )
          {
            $var = $this->atDictionary[$variable];

            // Turn of caching if asked for
            if ( isset($var['cache']) )
            {
              if ( $var['cache'] == "no" )
              {
                $cache = false;
                $this->fDebug->add("Disabling caching for: $variable", 5);
              }
            }

            // Process different types of variables
            switch ( $var['type'] )
            {

              // Return the output value variable
              case 'value':
                $returnData = strval($var['value']);
                break;

                // Return the output of a Library function
              case 'function':
                $function = strval($var['routine']);
                try {
                  $returnData = call_user_func($function, $values);
                  $this->cache[$var['type'] . $variable] = $returnData;
                } catch (Exception $e) {
                  trigger_error("Invalid or broken function called: $function by variable @$variable" . $e->getMessage(), E_USER_WARNING);
                }
                break;

                // Return the output of the function of an instanciated object class
              case 'objectFunction':
                // Break out the variable's xpath so we know what to do
                $xpath = explode('/', $var['xpath']);
                $xpathCount = count($xpath);
                if ( $xpathCount < 2 )
                {
                  $returnData = '{!i18n:fParser XPath Recursion Depth Error}';
                } else {
                  //We want to call $this->fMain->Class->Method(Args, ...);
                  try
                  {
                    $object = new $xpath[2]();
                    if ( is_object($object) )
                    {
                      $returnData = call_user_func_array(array($object, $xpath[3]), $values);
                    } else {
                      trigger_error("Unitialized Object Referenced: $xpath[2]", E_USER_WARNING);
                      $returnData = "{!i18n:Error}: {!i18n:Uninitalized Object Referenced}";
                    }
                  } catch (Exception $e) {
                    trigger_error("Invalid object referenced: $xpath[2] by variable @$variable", E_USER_WARNING);
                  }
                }
                break;

                // Return the output of a singleton object function
              case 'singletonFunction':
                // Break out the variable's xpath so we know what to do
                $xpath = explode('/', $var['xpath']);
                $xpathCount = count($xpath);
                if ( $xpathCount < 4 )
                {
                  $returnData = '{!i18n:fParser XPath Recursion Depth Error}';
                } else {
                  try
                  {
                    if ( is_callable(array($xpath[2], "getInstance")) )
                    {
                      $instance = call_user_func(array($xpath[2], 'getInstance'));
                      $returnData = call_user_func_array(array($instance, $xpath[3]), $values);
                    } else {
                      trigger_error("Invalid Singleton Object Referenced: $xpath[2]", E_USER_WARNING);
                      $returnData = "{!i18n:Error}: {!i18n:Invalid Singleton Object Referenced}";
                    }
                  } catch (Exception $e) {
                    trigger_error("Invalid Singleton Member Object Referenced: $xpath[2] by variable @$variable", E_USER_WARNING);
                  }
                }
                break;

                // Return a variable from  a singleton object
              case 'singletonVariable':
                // Break out the variable's xpath so we know what to do
                $xpath = explode('/', $var['xpath']);
                $xpathCount = count($xpath);
                if ( $xpathCount < 4 )
                {
                  $returnData = '{!i18n:fParser XPath Recursion Depth Error}';
                } else {
                  //We want to call $this->fMain->Class->Method(Args, ...);
                  $argArray = array();
                  for ( $i=4; $i < $xpathCount; $i++ )
                  {
                    $argArray[] = $xpath[$i];
                  }
                  if ( is_callable(array($xpath[2], "getInstance")) )
                  {
                    try {
                      $instance = call_user_func(array($xpath[2], 'getInstance'));
                      $variable = $xpath[3];
                      $returnData = $instance->$variable;
                    } catch (Exception $e) {
                      trigger_error("Exception occured in Singleton Pointer execution: $xpath[2] by variable @$variable", E_USER_WARNING);
                    }
                  } else {
                    trigger_error("Invalid Singleton Object Referenced: $xpath[2]", E_USER_WARNING);
                    $returnData = "{!i18n:Error}: {!i18n:Invalid Singleton Object Referenced}";
                  }
                  unset($argArray);
                }
                break;

                // Return the function output from a static class function
              case 'staticFunction':
                // Break out the variable's xpath so we know what to do
                $xpath = explode('/', $var['xpath']);
                $xpathCount = count($xpath);
                if ( $xpathCount < 4 )
                {
                  $returnData = '{!i18n:fParser XPath Recursion Depth Error}';
                } else {
                  try
                  {
                    if ( is_callable(array($xpath[2],$xpath[3])) )
                    {
                      $returnData = call_user_func_array(array($xpath[2], $xpath[3]), $values);
                    } else {
                      trigger_error("Invalid Static Class/Function Referenced: $xpath[2]", E_USER_WARNING);
                      $returnData = "{!i18n:Error}: {!i18n:Invalid Static Class/Function Referenced}";
                    }
                  } catch (Exception $e) {
                    trigger_error("Invalid Static Class Referenced: $xpath[2] by variable @$variable", E_USER_WARNING);
                  }
                }
                break;

                // Return the value of a variable in an instanciated class
              case 'staticVariable':
                $xpath = explode('/', $var['xpath']);
                $xpathCount = count($xpath);
                if ( $xpathCount < 4 )
                {
                  $returnData = '{!i18n:fParser XPath Recursion Depth Error}';
                } else {
                  return 'Error: Not implemented';
                  // return $xpath[2]::$xpath[3];
                }
                break;

                // Return the value of a variable that is a child of fMain
              case 'fVariable':
                // Break our xpath into usable objects
                if ( !isset($var['exploded']) )
                {
                  $var['exploded'] = explode('/', $var['xpath']); //keep exploded array for future use
                }
                $ptr = $this->fMain;
                foreach ( $var['exploded'] as $i=>$value )
                {
                  if ( $value == NULL && $i < 2 ) continue; // get rid of our unneeded NULL entries
                  $ptr = $ptr->{$value};
                }
                $returnData = $ptr;
                break;

              default:
                $returnData = '{!i18n:Invalid variable type detected in dictionary.}';
                break;
            }
          }
          break;

          // i18n Variables
        case '!i18n:':
          $data = $this->explodeVariable($variable); // Get the content && qty if needed
          switch ( Count($data) )
          {
            case 2: // $data[1] contains the value to decide on showing the pluralization or not
            $data = $this->i18n->parse($data[0], $data[1]);
            break;
            default: // Singular
            $data = $this->i18n->parse($data[0]);
            break;
          }
          $returnData = $data;
          break;

          // If we have directExecutionVariables enabled parse and run them
          // This is a security risk because you can in essense get to *any* internal function in Framewerk
        case '%':
          // Make this fall through by default if we havent explicitly turned it on
          if ( !$this->fMain->configuration->directExecutionVariables )
          {
            trigger_error('Direct execution variable encountered, but directExecutionVariables is not enabled.  Variable: ' . $variable);
            $returnData = 'Error';
            break;
          }

          // Explode our string into parts
          $data = $this->explodeVariable($variable);

          // First see if we have a function
          $function = array_shift($data);
          if ( is_callable($function) )
          {
            // get our return data and exit out
            $returnData = call_user_func_array($function, $data);
            break;
          } else {
            $class = $function;
          }

          // See if we have a singleton object
          $function = array_shift($data);
          if ( ( method_exists($class, 'getInstance') ) && ( method_exists($class, $function) ) )
          {
            // Get our singleton instance and run the function if it exists
            $instance = call_user_func(array($class, 'getInstance'));
            if ( is_object($instance) )
            {
              $returnData = call_user_func_array(array($instance, $function), $data);
            } else {
              trigger_error("Invalid Singleton Object Referenced: " . $class . "::" . $function . "()", E_USER_WARNING);
              $returnData = "{!i18n:Error}: {!i18n:Invalid Singleton Object Referenced}: " . $class . ":" . $function . "()";
            }
            break;
          }

          // Now try and get a function because we apparently have a class combo
          if ( method_exists($class, $function) )
          {
            // Create a new instance of our class and execute the function
            $object = new $class();
            if ( is_object($object) )
            {
              $returnData = call_user_func_array(array($object, $function), $data);
            } else {
              trigger_error("Could not initialize class: " . $class . ":" . $function . "()", E_USER_WARNING);
              $returnData = "{!i18n:Error}: {!i18n:Could not initialize class}: " . $class . ":" . $function . "()";
            }
            break;
          }

          // We couldn't do anything with it
          $returnData = 'Could not execute directExecutionVariable: ' . $variable;
          break;
      }
    }

    if ( StrToLower($returnData) == 'undefined' )
    {
      $this->fDebug->add("Undefined Variable: $variable", 4);
    }

    $this->fDebug->add("Found value: $returnData", 9);
    if ( $cache == true )
    {
      $this->cache[$tag] = $returnData;
    }
    $returnData = preg_replace_callback($this->tagPattern, array($this, '_parse'), $returnData);
    $this->parseDepth--;
    return $returnData;
  }
  
}

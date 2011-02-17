<?php // $Id:fjQuery.inc 876 2007-06-13 16:22:22Z gmr $
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
 * jQuery
 *
 * Use this class to initialize and manage jQuery plugins and initialization functions
 *
 * @since      2006-10-05
 * @author     Gavin M. Roy <gavinmroy@gmail.com>
 * @link       http://framewerk.org
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2006-2007 the Framewerk Development Group
 * @package    Toolkit
 * @uses       fSingletonObject
 * @version    $Rev:876 $
 */

class fjQuery extends fSingletonObject
{
  static private $instance = NULL;
  private $initCommands, $plugins, $fTheme, $additionalCode;

  private $fns = array();

  /**
   * Set our internal counters and such
   */
  protected function __construct()
  {
    parent::__construct();
    $this->initCommands = array();
    $this->additionalCode = array();
    $this->plugins = array();

    $this->fns['name'] = $this->fns['prototype'] = $this->fns['body'] = array();

    $this->fTheme = fTheme::getInstance();
    $this->fTheme->onBuild('Singleton', 'fjQuery', 'build', 99);
  }

  /**
   * Checks to see if the object is instanced in the internal static variable $instance
   * and if not, does so
   *
   * @return fjQuery Object instance
   */
  public static function getInstance()
  {
    if ( self::$instance == NULL ) self::$instance = new fjQuery();
    return self::$instance;
  }


  /**
   * Add onInit code
   *
   * @param string $init
   */
  function addInit($init)
  {
    if ( !in_array($init, $this->initCommands) )
    {
      $this->initCommands[] = $init;
    }
  }

  /**
   * Add a jQuery plugin
   *
   * @param string $plugin
   */
  function addPlugin($plugin)
  {
    $fMain = fMain::getInstance();
    $this->plugins[] = $plugin;
  }

  /**
   * Add additional javascript code
   *
   * @param string $code
   */
  function addCode($code)
  {
    if ( !in_array($code, $this->additionalCode) )
    {
      $this->additionalCode[] = $code;
    }
  }

  /**
   * Add a function to call on init
   *
   * @param string $fnPrototype
   * @param string $fnBody
   * @return bool
   */
  function addFunction($fnPrototype, $fnBody)
  {
    if ( ($p = strpos($fnPrototype, '(')) === false )
    {
      throw new Exception('Trying to add a jQuery function with an invalid prototype.  Example:  "funcDoSomething()"');
    }

    $fnName = trim(substr($fnPrototype, 0, $p));
    if ( in_array($fnName, $this->fns['name']) )
    {
      $this->fWarn->add('Trying to add a jQuery function that has already been defined.');
      return false;
    }
    $this->fns['name'][] = $fnName;
    $this->fns['prototype'][] = $fnPrototype;
    $this->fns['body'][] = $fnBody;
    return true;
  }

  /**
   * Add the JavaScript files to the header
   *
   */
  function build()
  {
    $this->fDebug->add('Building jQuery JavaScript headers.', 4);

    // Add the core jQuery file
    //$this->fTheme->addJSFile('jQuery/jQuery');

    // Add the plugins
    foreach ( $this->plugins AS $plugin )
    {
      $this->fTheme->addJSFile('jQuery-'.fTheme::getInstance()->getVariable('jQuery_Version').'/Plugins/' . $plugin . '/' . $plugin);
    }

    $output = <<<JSOUT
  <script type="text/javascript">
JSOUT;

    if ( count($this->additionalCode) > 0 )
    {
      foreach($this->additionalCode as $code)
      {
        $output .= '    ' . $code . "\n";
      }
    }

    if ( count($this->fns['name']) > 0 )
    {
      $output .= "\n\n/*** Begin fjQuery Custom Functions ***/\n";
      for ( $i=0; $i<count($this->fns['name']); $i++ )
      {
        $output .= "\n";
        $output .= 'function '.$this->fns['prototype'][$i] . "\n{\n";
        $output .= $this->fns['body'][$i];
        $output .= "\n}\n";
      }
      $output .= "\n/*** End fjQuery Custom Functions ***/\n\n";
    }

    $output .= <<<JSOUT
  </script>
JSOUT;
    $this->fTheme->htmlHead($output);

    
    // Put init commands at the end of execution, outside of a document.ready event
    if ( count($this->initCommands) )
    {
      $output = <<<JSOUT
  <script type="text/javascript">
    (function($){

JSOUT;
      foreach ( $this->initCommands AS $command )
      {
        $output .= '      ' . $command . "\n";
      }
      $output .= <<<JSOUT
    })(jQuery);
  </script>
JSOUT;
      $this->fTheme->htmlFoot($output);
    }

  }
  
}

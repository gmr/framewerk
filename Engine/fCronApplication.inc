<?php // $Id: fCronApplication.inc 16681 2008-09-03 16:37:00Z gmr $
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
 * Contains the fCronApplication abstract class that console apps can extend
 */
/**
 * Interface abstract object
 *
 * @author     Joe Hansche <jhansche@myyearbook.com>
 * @link       http://framewerk.org
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2004-2007 the Framewerk Development Group
 * @version    $Rev: 16681 $
 * @package    Engine
 * @subpackage Abstract_Applications
 * @since      2007-10-26
 */

abstract class fCronApplication
{
  protected $fMain;
  protected $fOutput;
  protected $fSession;
  protected $fTheme;

  protected $process;
  protected $uriValues;
  protected $baseURI;
  protected $args = array();
  
  const ANSI_SAVE_POSITION = "\x1b[s";
  const ANSI_RESTORE_POSITION = "\x1b[u";
  const ANSI_CLEAR_LINE = "\x1b[2K";
  const ANSI_POSITION_UP = "\x1b[A";
  const ANSI_BOLD = "\x1b[1m";
  const ANSI_NORMAL = "\x1b[m";

  /**
   * Cron Application Constructor
   *
   * Sets the all the needed internal variables and engine parameters.
   *
   * @param object fMain object is required
   */
  function __construct()
  {
    if ( isset($_SERVER['REMOTE_ADDR']) && strlen($_SERVER['REMOTE_ADDR']) > 0 )
    {
      // Cannot be run via a web browser.
      throw new fHTTPException(404, 'Not Found');
    }
    // Set our objects and our object shortcuts
    $this->fMain      = fMain::getInstance();
    $this->fOutput    = fOutput::getInstance();
    $this->fSession    = fSession::getInstance();
    $this->fTheme     = fTheme::getInstance();
    
    $this->fOutput->setMimeType('text/plain');

    $this->fTheme->useTheme(false);

    // Set our default values
    $this->process    = true;
    $this->uriValues  = $this->fSession->uriValues;
    for ( $y = 0; $y < Count($this->uriValues); $y++ )
    {
      $this->uriValues[$y] = str_replace('_', ' ', rawurldecode($this->uriValues[$y]));
    }
    
    $this->prepareArguments( array_slice( $_SERVER['argv'], 2 ) );

    // Set the baseURI
    $this->baseURI = $this->fMain->configuration->site->baseURI . $this->fMain->keyword . '/';
  }
  
  function showHelp()
  {
    $flags = $vals = array();
    
    foreach( $this->args as $opt => $details )
    {
      if ( ! is_array( $details ) )
      {
        continue;
      }
      if ( $details['type'] == 'flag' )
      {
        $flags[] = $opt;
      } else {
        $vals[] = $opt;
      }
    }
    
    $this->output("Usage: " . $_SERVER['argv'][0] . ' ' . $_SERVER['argv'][1]);
    if ( count($flags) > 0 )
    {
      $this->output( ' [-' . join($flags,'') . ']' );
    }
    if ( count( $vals ) > 0 )
    {
      $this->output( ' [-' . join($vals, ' <value>] [-') . ' <value>]' );
    }
    if ( isset($this->args['text']) )
    {
      $this->output( ' [ "--" <text> ]' );
    }
    $this->output( "\n" );
    
    foreach( $this->args as $opt => $details )
    {
      if ( ! is_array( $details ) )
      {
        continue;
      }
      $longopt = ($details['longopt'] != '' ? '--' . $details['longopt'] : '' );
      $this->output("     " . str_pad('-'.$opt . ($details['type'] != 'flag' ? ' <value>' : ''), 12, ' ', STR_PAD_RIGHT) );
      $this->output("  " . str_pad($longopt . ($details['type'] != 'flag' ? '=<value>' : ''), 20, ' ', STR_PAD_RIGHT) );
      $this->output($details['desc'].(isset($details['value'])?" [Default: ".var_export($details['value'], true)."]":"")."\n");
    }
  }
  
  function prepareArguments( $argv )
  {
    for ( $i=0; $i<count($argv); $i++ )
    {
      $arg = $argv[$i];
      // Show help on --help
      if ( ltrim($arg, '-') === 'help' )
      {
        $this->showHelp();
        exit;
      }
      // Arguments should start with -; for text arguments, use --
      if ( $arg[0] != '-' )
      {
        $this->outputError('Unrecognized argument: ' . $arg . ".  Use: '\$0 -- <text>' for text arguments\n");
        $this->showHelp();
        exit;
      }
      // -- indicates end of arguments; remaining values are joined as a single string
      if ( $arg == '--' )
      {
        $this->args['text'] = join(' ', array_splice( $argv, ++$i ) );
        break;
      }
      // It's a longopt
      if ( $arg[1] == '-' )
      {
        $arg = substr( $arg, 2 );
        list( $arg, $val ) = explode('=', $arg, 2);
        foreach( $this->args as $short => $details )
        {
          if ( $arg == $details['longopt'] )
          {
            if ( $details['type'] != 'flag' )
            {
              if ( ! isset( $val ) )
              {
                $this->outputError( 'Value expected after argument: ' . $arg . "\n" );
                $this->showHelp();
                exit;
              }
              $this->args[$short]['value'] = $val;
            } else {
              $this->args[$short]['value'] = true;
            }
            break;
          }
        }
      } else 
      {
        $args = ltrim( $arg, '-' );
        foreach( str_split($args) as $arg )
        {
          if ( ! isset( $this->args[$arg] ) )
          {
            $this->outputError('Unrecognized argument: ' . $arg . "\n");
            $this->showHelp();
            exit;
          }
          if ( $this->args[$arg]['type'] != 'flag' )
          {
            $this->args[$arg]['value'] = $argv[++$i];
          } else {
            $this->args[$arg]['value'] = true;
          }
        }
      }
    }
  }

  /**
   * Wrapper function to make adding output content easier
   *
   * @param string Value to append
   * @param bool Try and run an XSL transformation on it
   * @param string optional XSL file to try and use
   * @return bool Output added or not
   */
  function output($value)
  {
    // If processing is turned off exit
    if ( !$this->process ) return false;
    
    return fwrite( STDOUT, $value );
  }
  
  function outputError( $string )
  {
    return fwrite( STDERR, $string );
  }

  function setTimeout($secs)
  {
    set_time_limit($secs);
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
   * Some interface developers might want to use _ as something other than a replacement char for space.
   * Calling this function at the top of your execute function will rebuild the uriValues array without any
   * string replacement of _ to space.
   */
  function ignoreURIUnderscore()
  {
    $this->uriValues = $this->fSession->uriValues;
  }
  
  function prompt ( $prompt, $echo = true )
  {
    // If $echo is disabled, then don't print what the user typed
    if ( $echo === false )
    {
      $oldstty = trim( shell_exec('stty -g') );
      shell_exec( 'stty -echo' );
    }
    
    // Read the terminal directly
    $fp = fopen( '/dev/tty', 'r' );
    
    // Get up to 1KB or until a newline
    $value = trim( fgets( $fp, 1024 ) );
    
    if ( $echo === false )
    {
      // Restore previous terminal settings
      shell_exec('stty ' . escapeshellarg( $oldstty ) );
      // Print a newline because the user's [ENTER] key didn't get printed
      echo "\n";
    }

    fclose( $fp );
    
    return $value;
  }
  
}
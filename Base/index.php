<?php
############################################################################
#    Copyright (C) 2004-2009 by the Framewerk Development Group            #
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
 * Framewerk Manager Program.
 *
 * This program is responsbile for the high level operation of the fMain object. All requests are funneled through this
 * via mod_rewrite and .htaccess.
 *
 * @author     Gavin M. Roy <gavinmroy@gmail.com>
 * @link       http://framewerk.org
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2004-2009 the Framewerk Development Group
 * @since      2004
 * @package    Engine
 * @subpackage Core
 */

/**
 * Change this to true if you would like to allow the engine to include tick/low level debugging.
 * This does not enable tick/low level debugging.  Once you have set this to true then you must
 * enable it in XML/configuration.xml.  By adding this level of check we remove redudant file loading
 * of the configuration which has all the important and related tick debugging settings.
 */
define( 'TICKS', false );

// If we enable ticks, include the code for it.
if ( TICKS == true )
{
  require_once("Engine/System/ticks.inc");
}


// Built in define for transparent file caching, change this to false if you don't want caching
define( 'FILE_CACHING_ENABLED', true );

// Prepare the Debugging class
require( 'Engine/System/debug.inc' );

/**
 * Build an array of all files
 *
 * @param string Directory
 */
function mapFiles( $directory )
{
  global $files, $ignore;

  $dir = opendir( $directory );
  while ( ( $entry = readdir( $dir ) ) )
  {
    if ( $entry != "." && $entry != ".." && !( strstr( $entry, '.svn' ) > -1 ) )
    {
      $file = $directory . '/' . $entry;
      if ( is_dir( $file ) && !in_array( $file, $ignore ) )
      {
        mapFiles( $file );
      }
      elseif ( is_file( $file ) && ( substr( $file, strlen( $file ) - 4, 4 ) == '.inc' ) )
      {
        $temp = explode( '.', $entry );
        $files[$temp[0]] = $directory . '/' . $entry;
      }
    }
  }
}

/**
 * Get the locations of our class files for the autoloader
 *
 * @param bool Can use cache?
 * @param bool Clear the cache?
 */
function getFiles( $useCache = true, $clearCache = false )
{
  global $files;

  // Decide to use cache based upon what is passed in and our global define
  $useCache = ( $useCache && FILE_CACHING_ENABLED !== false );

  // If cache is enabled and we don't want to clear the cache, get the cached value
  if ( $useCache == true && $clearCache == false )
  {
    $files = unserialize( file_get_contents( '/tmp/indexFileMap' ) );
  }
 
  // If we don't have the array, map it and store it if cache is enabled
  if ( !is_array( $files ) || count( $files ) == 0 )
  {
    $files = array( );
    
    // Map files in reverse of this order
    $searchPaths = array( '../Base/', '../Site/' );
    
    foreach ( $searchPaths as $searchPath )
    {
      mapFiles( realpath( $searchPath ) );
    }

    if ( $useCache === true )
    {
      file_put_contents( '/tmp/indexFileMap', serialize($files) );
    }
  }
}

/**
 * Using PHP5's Object Autoloader we will load the files when they need to be instanced
 *
 * @param Object $classname
 */
function __autoload($classname)
{
  global $debug, $files;

  if ( !isset( $files[$classname] ) )
  {
    // Get files ignoring the cache, to update it
    getFiles( true, true );
  }
  if ( isset( $files[$classname] ) && is_file( $files[$classname] ) )
  {
    if ( is_object( $debug ) )
    {
      $debug->AddMessage( "Autoloading Object: $classname" );
    }
    require( $files[$classname] );
  } else {
    getFiles( true, true );
    if ( isset( $files[$classname] ) && is_file( $files[$classname] ) )
    {
      require( $files[$classname] );
    }
  }
}

//Build our file and ignore arrays
$files = array( );
$ignore = array( );
$ignore[] = './Engine/System';
getFiles( true, false );

// Check for PHP Version Information
if ( version_compare( phpversion( ), "5.1", "<" ) )
{
  throw new fHTTPException( 501, "Framewerk requires PHP version 5.1 and above." );
}

// Set our Required extensions
$extension   = array( );
$extension[] = "PDO";
$extension[] = "SimpleXML";
$extension[] = "xsl";
$extension[] = "dom";

// Check for required extensions
$extensions = get_loaded_extensions( );
foreach ( $extension AS $k => $check )
{
  if ( in_array( $check, $extensions ) ) // If the required ext is in the loaded extension
  {
    unset( $extension[$k] );             // Remove it from the required-ext array
  }
}

// At this point $extension represents the required extensions that are not loaded.
if ( ( $cnt = count( $extension ) ) > 0 )
{
  throw new fHTTPException( 501, "Your PHP installation is missing $cnt required extension(s), as follows: " . join(", ", $extension) );
}

// Load our library of functions
include( "Engine/functions.inc" );

// Create our instance of the debug and framewerk object
$fMain = fMain::getInstance( );
$fMain->process( );

// Log the traffic if the configuration says we can
if ( $fMain->configuration->site->logTraffic['enabled'] ) $log = new fTrafficLog( );
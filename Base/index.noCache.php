<?php
/**
 * Framewerk Non-Caching Controlling Program
 *
 * This program is responsbile for the high level operation of the fMain object. All requests are funneled through this script.
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
 * enable it in Configuration/registry.xml.  By adding this level of check we remove redudant file loading
 * of the configuration which has all the important and related tick debugging settings.
 */
define( 'TICKS', false );

/**
 * Important: Change this to false if you know your system has everything needed to run Framewerk.
 * Set it to false when you're sure you're up and running correctly for a minor speed-up
 */
define( 'SANITY_CHECK', true );

// Built in define for transparent file caching, change this to false if you don't want caching
define( 'FILE_CACHING_ENABLED', false );

/**
 * Define PHP class filesystem map array.
 */
$_classFiles = null;

/**
 * Build an array of all files
 *
 * @param string Directory
 */
function mapClassFiles( $directory )
{
  global $_classFiles;
  static $ignore = array( 'Engine/System' );

  if ( in_array( $directory, $ignore ) )
  {
    return;
  }

  $dir = opendir( $directory );
  while ( ($entry = readdir( $dir ) ) ) {
    if ( $entry[0] == '.' )
    {
      //ignore ., .., .svn, .git, etc
      continue;
    }
    $file = "{$directory}/{$entry}";
    if ( is_dir( $file ) )
    {
      mapClassFiles($file);
    }
    elseif ( is_file( $file ) && preg_match( ';(^|/)([^/]+)[.]inc$;', $file, $regs ) )
    {
      $_classFiles[$regs[2]] = $file;
    }
  }
}

/**
 * Using PHP5's Object Autoloader we will load the files when they need to be instanced
 *
 * @param Object $classname
 */
function __autoload( $classname )
{
  global $_classFiles, $debug;

  if ( $_classFiles === null )
  {
    $_classFiles = array();

    // Map files in reverse of this order
    $searchPaths = array( '../Base/', '../Site/' );
    foreach ( $searchPaths as $path )
    {
      mapClassFiles( realpath( $path ) );
    }
  }

  // Check that the file is mapped
  if ( isset( $_classFiles[$classname] ) )
  {
    // If debugging is working, add a debug message
    if ( is_object( $debug ) )
    {
      $debug->addMessage( "Autoloading Object: $classname" );
    }
    // Since the user should never be requiring a file, no need for require_once
    require( $_classFiles[$classname] );
  }
}

// If we enable ticks, include the code for it.  
if ( TICKS == true )
{
  require_once( 'Engine/System/ticks.inc' );
}

// Prepare the Debugging class
require_once( 'Engine/System/debug.inc' );

// Make sure we have everything needed to run Framewerk.  Turn this off once you're up and running for a speed-up
if ( SANITY_CHECK === true )
{
  // Check for PHP Version Information
  if ( version_compare( phpversion( ), "5.1", "<" ) )
  {
    throw new fHTTPException( 501, "Framewerk requires PHP version 5.1 and above." );
  }
  
  // Set our Required extensions
  $extension = array( "dom", "SimpleXML", "xsl" );
  
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
    $extensions = implode( ', ', $extension);
    throw new fHTTPException( 501, "Your PHP installation is missing {$cnt} required extension(s), as follows: {$extensions}" );
  }

}

// Create our instance of the debug and Framewerk object
$fMain = fMain::getInstance( );
$fMain->process( );

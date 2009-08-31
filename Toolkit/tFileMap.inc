<?php
/**
 * File mapping class
 *
 * Maps files within framewerk directories for loading on-the-fly
 *
 * @author Gavin M. Roy <gmr@myyearbook.com>
 * @since 2008-07-07 10:20:00 EST
 */
class tFileMap extends fObject
{
  private $extension, $files = array();

  function __construct( $mapName, $extension, $searchPaths, $useCache = true )
  {
    $useCache = ( $useCache && FILE_CACHING_ENABLED !== false );
    if ( !isset($mapName) )
    {
      throw new Exception('Invalid tFileMap Map Name');
    }

    if ( !isset($extension) )
    {
      throw new Exception('Invalid tFileMap extension type');
    }

    if ( !is_array($searchPaths) )
    {
      throw new Exception('Invalid tFileMap search path array');
    }

    $this->extension = $extension;
    if ( $useCache )
    {
      $this->files = unserialize( file_get_contents( '/tmp/' . $_SERVER['HTTP_HOST'] . '-tFileMap-' . $mapName ) );
    }

    // If we don't have the array, map it and store it if cache is enabled
    if ( !is_array($this->files) || count($this->files) == 0 )
    {
      foreach ( $searchPaths as $searchPath )
      {
        $this->mapFiles( realpath( $searchPath ) );
      }

      file_put_contents('/tmp/' . $_SERVER['HTTP_HOST'] . '-tFileMap-' . $mapName, serialize($this->files) );
    }
  }

  function mapFiles($directory)
  {
    $dir = opendir($directory);
    while ( ( $entry = readdir($dir) ) )
    {
      if ( $entry != "." && $entry != ".." && !(strstr($entry, '.svn') > -1) )
      {
        $file = $directory . '/' . $entry;
        if ( is_dir($file) )
        {
          mapFiles($file);
        }
        elseif ( is_file($file) && ( substr($file, strlen($file) - strlen($this->extension), strlen($this->extension) ) == $this->extension) )
        {
          $temp = explode('.', $entry);
          $this->files[$temp[0]] = $directory . '/' . $entry;
        }
      }
    }
  }

  function getFiles()
  {
    return $this->files;
  }

}
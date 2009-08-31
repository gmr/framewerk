<?php // $Id:fFile.inc 876 2007-06-13 16:22:22Z gmr $
############################################################################
#    Copyright (C) 2007 by the Framewerk Development Group                 #
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
 * File Read/Caching Class
 *
 * This class is for reading files in and caching them in SHM cache to increase performance
 *
 * @author     Gavin M. Roy <gavinmroy@gmail.com>
 * @link       http://framewerk.org
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2007 the Framewerk Development Group
 * @version    $Rev:876 $
 * @package    Engine
 * @since      2007-08-14
 * @todo       Add file writing and recaching on writing
 */


class fFile
{
  const CACHE_DURATION = 86400;
  private $file, $data, $timestamp, $lastValidated;
  
  /**
   * Static cache
   *
   * @var array
   */
  private static $sCache = array();

  /**
   * Constructor
   *
   * Check the SHMOP Cache to see if there is a copy in cache, validate we can use it
   * otherwise read it new and cache it
   *
   * @param String $fileName
   * @return bool
   */
  function __construct($fileName, $noCache = false)
  {
    $noCache = ( $noCache || FILE_CACHING_ENABLED === false );
    
    // If it's not a URL, turn fileName into realpath(fileName)
    if ( strpos( $fileName, '://' ) === false )
    {
      $fileName = realpath( $fileName );
    }
    
    $hash = 'framewerk::fFile::' . md5($fileName) . '::' . $_SERVER['SERVER_NAME'];

    if ( $noCache === false )
    {
      // Check static cache first
      if ( isset(self::$sCache[$hash]) )
      {
        $this->data = self::$sCache[$hash];
      } else {
        $this->data = apc_fetch($hash);
      }
    }

    if ( ! $this->data || $noCache === true )
    {
      // Get the data
      $this->data = file_get_contents($fileName);
      if ( $noCache === false && $this->data )
      {
        apc_store($hash, $this->data, fFile::CACHE_DURATION);
        // Also save to our static cache
        self::$sCache[$hash] = $this->data;
      }
    }
  }
  

  /**
   * Return the file contents
   *
   * @return string
   */
  function read()
  {
     return $this->data;
  }

}
<?php // $Id:fSHMOPCache.inc 876 2007-06-13 16:22:22Z gmr $
############################################################################
#    Copyright (C) 2005-2007 by the Framewerk Development Group            #
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
 * SHMOP Cache Class
 *
 * This object allows classes to instance a Shared Memory Cache with little effort.
 *
 * @author     Gavin M. Roy <gavinmroy@gmail.com>
 * @link       http://framewerk.org
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2005-2007 the Framewerk Development Group
 * @version    $Rev:876 $
 * @package    Toolkit
 * @subpackage Caching
 * @since      2005
 */

class fSHMOPCache extends fObject
{
  private $block = NULL;

  function __construct($id)
  {
    // Assign our internal Framewerk variable
    $this->fDebug = fDebug::getInstance();

    // Make sure we can use shared memory
    if ( !function_exists("shmop_open") )
      throw new Exception("Shared Memory isn't available");

    // Set valid entities
    $this->validEntity('id');

    // Set our ID
    $this->setID($id);
  }

  /**
   * Create a "Unique ID" based upon the ord sum of the id passed in
   *
   * @param String ID
   * @return int Numeric ID
   */
  function computeIDValue($id)
  {
    $x = 0;
    for ( $y = 0; $y < strlen($id); $y++ )
      $x += ord($id[$y]);
    return $x;
  }

  /**
   * Create a "Unique ID" based upon the ord sum of the id passed in
   *
   * @param Char   Mode for opening (a,c,w,n)
   * @param String Permissions of Opening (on write)
   * @param String Size of memory block to allocate
   * @return bool Opened SHM Store Block
   */
  function openSHMStore($mode = 'a', $perm = 0, $size = 0)
  {

    // Open our shared memory block
    $this->block = shmop_open($this->id, $mode, $perm, $size);

    // If our block didn't allocate add a debugging message and return false
    if ( !$this->block )
    {
      $this->fDebug->add("Error opening a shared memory block for ID: " . $this->id . " Mode: $mode, Perm: $perm, Size: $size", 4);
      return false;
    } else
    $this->fDebug->add("Opened shared memory block ID: " . $this->id . " containing  " . shmop_size($this->block) . " bytes.", 6);

    // Let our calling routine know everything opened ok
    return true;
  }

  /**
   * Read in SHM Data
   *
   * @return String Data stored in SHM
   */
  function readSHMData()
  {
    // If someone forgot to close the previous store, do so
    if ( $this->block != NULL )
      $this->closeSHMStore();

    // Open the store or return false
    if ( !$this->openSHMStore() )
      return false;

    // Data size
    $size = shmop_size($this->block);

    // Read in our session data
    $temp = shmop_read($this->block, 0, $size);

    // If we have bzip compression decompress the data, since we saved it that way
    if ( function_exists("bzdecompress") )
    $temp = bzdecompress($temp);

    $data = unserialize($temp);

    // Close our SHM Store
    $this->closeSHMStore();

    // Send debugging output
    $this->fDebug->add("Read $size bytes out of shared memory for ID: " . $this->id, 6);

    return $data;
  }

  /**
   * Save SHM Data
   *
   * Takes data, compresses it if available and puts it in a SHM Store
   *
   * @param String data to save
   * @return bool Successfully saved data
   */
  function saveSHMData($data)
  {
    // Close our pre-existing open store
    if ( $this->block != NULL )
			$this->closeSHMStore();

    $out = serialize($data);

    // Compress it if we can
    if ( function_exists("bzcompress") )
    $out = bzcompress($out, 9);

    // How much data do we want to write out?
    $size = strlen($out);

    // Delete our old data
    $this->openSHMStore();
    if ( !shmop_delete($this->block) )
      $this->fDebug->add("Couldn't delete SHM Block " . $this->block, 4);

    // If we can open our SHM Store, write out the data else return false
    if ( $this->openSHMStore('c', 0644, $size) )
    $size = shmop_write($this->block, $out, 0);
    else {
      $this->fDebug->add("Couldn't open a Shared Memory block for writing " . strlen($out) . " bytes ID: " . $this->id . " Block: " . $this->block, 4);
      return false;
    }

    // Close our writing store
    $this->closeSHMStore();

    // Send our debugging output and return the size of the data returned
    $this->fDebug->add("Wrote $size bytes to shared memory for " . $this->id, 6);
    return $size;
  }

  /**
   * Set the SHMOP ID
   *
   * @param String ID Value
   * @return Int Numeric SHM ID
   */
  function setID($id)
  {
    // Get our ID Value
    $this->id = $this->computeIDValue($id);

    // Return our numeric ID
    return $this->id;
  }

  /**
   * Close an open SHM Store
   */
  function closeSHMStore()
  {
    shmop_close($this->block);
    $this->block = NULL;
  }

  /**
   * Destructor
   *
   * Close our Store if it's still open.
   */
  function __destruct()
  {
    if ( $this->block != NULL )
    {
      $this->closeSHMStore();
    }
  }
}
<?php
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
   * Sends a file that's stored via fFileStorage
   *
   * Sends out the data stored via fFileStorage
   *
   * @since      2005-12-15
   * @author     Gavin M. Roy <gavinmroy@gmail.com>
   * @link       http://framewerk.org
   * @license    http://opensource.org/licenses/bsd-license.php BSD License
   * @copyright  Copyright 2005-2007 the Framewerk Development Group
   * @package    Applications
   * @subpackage Base
   * @uses       fBinaryApplication
   * @version    $Rev: 881 $
   */

class FileDownload extends fBinaryApplication
{
  private $id, $obj;

  function execute($key = NULL)
  {
    // Set our obj wide vars
    $this->setVariables();

    $filestore = new fFileStorage($this->obj);
    return $filestore->send($this->id);
  }

  function modified($key, $timestamp)
  {
    // Set our obj wide vars
    $this->setVariables();

    // Create our filestore and return if it's been modified or not
    $filestore = new fFileStorage($this->obj);
    return $filestore->isModified($this->id);
  }

  /**
   * Set our object wide variables
   */
  function setVariables()
  {
    // Set what we expect our parameters to be
    $obj = $this->uriValues[0];
    $id = intval($this->uriValues[1]);

    // if we added an extension for asthetic purposes, remove it and just use the id
    if ( strstr($id, '.') )
    {
      $temp = explode('.', $id);
      $id = intval($temp[0]);
    }

    // If the $obj var is actually our id, use that
    if ( intval($obj) > 0 )
    {
      if ( strstr($obj, '.') )
      {
        $temp = explode('.', $obj);
        $obj = intval($temp[0]);
      }
      $query = $this->PDO->prepare('SELECT obj FROM tfilestorage WHERE id = :id;');
      $query->bindParam(':id', $obj);
      $query->execute();
      $data = $query->fetch(PDO::FETCH_OBJ);
      $id = $obj;
      $obj = $data->obj;
    }

    // Set our object wide values
    $this->id = $id;
    $this->obj = $obj;
  }
}
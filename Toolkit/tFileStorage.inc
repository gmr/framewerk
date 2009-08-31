<?php // $Id:tFileStorage.inc 876 2007-06-13 16:22:22Z gmr $
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
 * File Storage Object Source
 */
/**
 * File Storage Object
 *
 * Standard Abstracted File Storage Object
 *
 * @author     Gavin M. Roy <gavinmroy@gmail.com>
 * @link       http://framewerk.org
 * @since      2006-01-14
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2006-2007 the Framewerk Development Group
 * @version    $Rev:876 $
 * @package    Toolkit
 * @uses       fObject
 */

class tFileStorage extends fObject
{
  protected $fMain, $PDO, $fDebug;
  private $obj, $data;

  /**
   * Constructor
   *
   * @param Object/String The calling object or object name
   */
  function __construct($application)
  {
    parent::__construct();
    if ( is_object($application) )
    {
      $this->obj = get_class($application);
    } else  {
      $this->obj = $application;
    }
  }

  /**
   * Delete a stored file
   *
   * @param integer $id
   * @return bool Success
   */
  function delete($id)
  {
    // Prepare our query
    switch ( $this->fMain->PDODriver )
    {
      case 'pgsql':
        $query = $this->fMain->PDO->prepare('DELETE FROM tfilestorage WHERE obj = :obj AND id = :id;');
        break;
      default:
        Throw new Exception('Invalid PDO Driver requested');
    }

    // Bind our paramters and search
    $query->bindParam(':id', $id);
    $query->bindParam(':obj', $this->obj);
    $result = $query->execute();
    $this->fDebug->add('Deleting file id #' . $id . ' for ' . $this->obj . ' returned: ' . strval($result), 4);
    return $result;
  }

  /**
   * Get the file, returns raw file contents
   *
   * @param int $id
   * @return string
   */
  function get($id)
  {
    switch ( $this->fMain->PDODriver )
    {
      case 'pgsql':
        $query = $this->fMain->PDO->prepare("SELECT file FROM tfilestorage WHERE obj = :obj AND id = :id;");
        break;
      default:
        Throw new Exception('Invalid PDO Driver requested');
    }
    $query->bindParam(':id', $id);
    $query->bindParam(':obj', $this->obj);
    $query->execute();
    $data = $query->fetch(PDO::FETCH_OBJ);
    return base64_decode($data->file);
  }

  /**
   * Get the data record for an id
   *
   * @param int $id
   * @return bool
   */
  function getData($id)
  {
    switch ( $this->fMain->PDODriver )
    {
      case 'pgsql':
        $query = $this->fMain->PDO->prepare("SELECT  id, obj, owneruid, permission, mimetype, filename, filesize, uploaded, EXTRACT(epoch FROM uploaded) AS updated FROM tfilestorage WHERE obj = :obj AND id = :id;");
        break;
      default:
        Throw new Exception('Invalid PDO Driver requested');
    }
    $query->bindParam(':id', $id);
    $query->bindParam(':obj', $this->obj);
    $query->execute();
    $this->data = $query->fetch(PDO::FETCH_OBJ);
    return is_object($this->data);
  }

  /**
   * Return the filename
   *
   * @param int $id
   * @return string
   */
  function getFilename($id)
  {
    if ( ( !is_object($this->data) ) || ( $id != $this->data->id ) )
    {
      $this->getData($id);
    }
    return $this->data->filename;
  }

  /**
   * Return the file mime type
   *
   * @param int $id
   * @return string
   */
  function getMimeType($id)
  {
    if ( ( !is_object($this->data) ) || ( $id != $this->data->id ) )
    {
      $this->getData($id);
    }

    // Return our mimetype
    return $this->data->mimetype;
  }

  /**
   * Return the Permission string of the fileupload
   *
   * @return string Permission
   */
  function getPermission($id)
  {
    if ( ( !is_object($this->data) ) || ( $id != $this->data->id ) )
    {
      $this->getData($id);
    }
    return strval($this->data->permission);
  }

  /**
   * Return the filesize
   *
   * @param int $id
   * @return float
   */
  function getSize($id)
  {
    if ( ( !is_object($this->data) ) || ( $id != $this->data->id ) )
    {
      $this->getData($id);
    }
    return floatval($this->data->filesize);
  }


  /**
   * Return the UNIX timestamp of the fileupload
   *
   * @param int $id
   * @return int UNIX Timestamp
   */
  function getTimestamp($id)
  {
    if ( ( !is_object($this->data) ) || ( $id != $this->data->id ) )
    {
      $this->getData($id);
    }
    return intval($this->data->updated);
  }

  /**
   * Determine if an inbound unix timestamp is older than our last modification
   *
   * @param int $id
   * @param int $timestamp
   * @return bool
   */
  function isModified($id, $timestamp)
  {
    $last = $this->getTimestamp($id);
    if ( $last > $timestamp )
    {
      return true;
    }
    return false;
  }

  /**
   * Save a file
   *
   * @param string $file file contents or filename
   * @param string $mimeType
   * @param int $owner
   * @param string $permission
   * @return int unique ID for file
   */
  function save($file, $filename, $mimeType = NULL, $owner = NULL, $permission = NULL)
  {
    /**
     * If it's a filename that we can read, get the file contents otherwise we're expecting
     * the file contents to be in the $file variable
     */
    if ( is_file($file) && ( is_uploaded_file($file)) )
    {
      $file = base64_encode(file_get_contents($file));
      $this->fDebug->add('Base64 Encoded the file which resulted in a filesize of ' . strlen($file), 4);
    } else {
      $file = base64_encode($file);
      $this->fDebug->add('Base64 Encoded the string which resulted in a filesize of ' . strlen($file), 4);
    }

    /**
     * Because of how the id's come back differently, lets just put db specific code in
     * each switch instead of binding once to differently prepared queries
     */
    switch ( $this->fMain->PDODriver )
    {
      case 'pgsql':
        $query = $this->fMain->PDO->query("SELECT * FROM nextval('tfilestorage_id_seq');");
        $data = $query->fetch(PDO::FETCH_OBJ);
        unset($query);
        $query = $this->fMain->PDO->prepare('INSERT INTO tfilestorage VALUES ( :id, :obj, :owner, :permission, :mimetype, now(), :filesize, :filename, :file);');
        $query->bindParam(':id', $data->nextval);
        $query->bindParam(':obj', $this->obj);
        $query->bindParam(':owner', $owner);
        $query->bindParam(':permission', $permission);
        $query->bindParam(':mimetype', $mimeType);
        $size = strlen(base64_decode($file));
        $query->bindParam(':filesize', $size);
        $query->bindParam(':filename', $filename);
        $query->bindParam(':file', $file);
        if ( !$query->execute() )
        return false;
        break;
      default:
        Throw new Exception('TFileStorage: Invalid PDO Driver requested: ' . $this->fMain->PDODriver);
    }
    return $data->nextval;
  }

  /**
   * Send the file if we have a valid timestamp as we should always have a vaild timestamp for a valid file
   *
   * @param int $id
   * @return bool
   */
  function send($id)
  {
    $timestamp = $this->getTimestamp($id);
    if ( $timestamp <= 0 )
    {
      new THTTPError(404, 'File Not Found: ' . $this->obj . '/' . $id);
      return false;
    }

    // Set the mime type and the last update
    $TOutput = TOutput::getInstance();
    $TOutput->setLastModified(intval($timestamp));
    $TOutput->setMimeType($this->getMimeType($id));
    $TOutput->setSuggestedFilename($this->getFilename($id));
    $TOutput->setContent($this->get($id));
    return true;
  }

}
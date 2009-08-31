<?php // $Id: fUserProfilePDO.inc 919 2007-08-06 18:34:41Z dallas $
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
 * fUserProfilePDO backend
 */
/**
 * User Profile Backend for PDO
 *
 * @author     Joe Hansche <madcoder@gmail.com>
 * @link       http://www.framewerk.org
 * @license    BSD
 * @copyright  Copyright 2005-2007 the Framewerk Development Group
 * @version    $Rev: 919 $
 * @package    Engine
 * @subpackage Core
 * @since      2006-12-17
 * @uses       fUserProfileBackend
 */

class fUserProfilePDO extends fUserProfileBackend
{
  protected $pdoFields;

  /**
   * Constructor
   *
   * Loads user data by username, email address or user ID or sets up a new blank user.
   *
   * @param string key
   */
  function __construct($key)
  {
    $this->PDO = fMain::getInstance()->PDO;

    $query = $this->PDO->query('SELECT * FROM tuserprofile_fields ORDER BY name;');
    $num = $query->rowCount();
    for ( $i=0; $i<$num; $i++ )
    {
      $data = $query->fetch(PDO::FETCH_OBJ);
      $pdoFields[$data->name] = $data;
      $this->validEntity($data->name);
    }

    parent::__construct($key);
  }

  /**
   * Save the user data
   *
   * @return bool
   */
  function save()
  {
    $this->fWarn->add('Saving is not yet implemented');
  }

  /**
   * Load the data for this user
   *
   * @returns bool true on success, false on failure
   */
  function loadUserData()
  {
    $query = $this->PDO->prepare('SELECT * FROM tuserprofile_fields a JOIN tuserprofile_data b ON b.fieldname = a.name WHERE uid = ?');
    $query->execute(array($this->userid));

    $num = $query->rowCount();

    if ( $num === 0 )
      return false;

    for ( $i=0; $i<$num; $i++ )
    {
      $data = $query->fetch(PDO::FETCH_OBJ);

      switch($data->datatype)
      {
        case 'integer':
          $value = intval($data->data);
          break;
        case 'float':
          $value = floatval($data->data);
          break;
        case 'text':
          $value = strval($data->data);
          break;
        case 'boolean':
          $value = (bool)$data->data;
          break;
        case 'binary':
          $value = base64_decode($data->data);
          break;
        default:
          $value = NULL;
          break;
      }
      $this->{$data->name} = $value;
    }

    return true;
  }

}
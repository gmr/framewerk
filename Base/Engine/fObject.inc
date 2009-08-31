<?php // $Id: fObject.inc 8507 2007-12-20 19:38:42Z jhansche $
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
 * fObject
 *
 * A default object implementation
 *
 * @author     Gavin M. Roy <gavinmroy@gmail.com>
 * @link       http://framewerk.org
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2004-2007 the Framewerk Development Group
 * @since      2004-02-23
 * @version    $Rev: 8507 $
 * @package    Engine
 * @subpackage Core
 * @abstract
 */

abstract class fObject
{
  protected $fMain, $fDebug, $fProfile, $fWarn;
  protected $fMessages;
  private $entity = array();
  public  $validEntities = array();

  /**
   * Constructor, get parent reference
   */
  function __construct()
  {
    $this->fMain = fMain::getInstance();
    $this->fDebug = fDebug::getInstance();
    $this->fProfile = fProfile::getInstance();
    $this->fWarn = fWarn::getInstance();
    $this->fMessages = fMessages::getInstance();
  }

  function getDataValues()
  {
    return $this->entity;
  }

  /**
   * PHP Object property overload
   *
   * @param String Keyname ($Object->Keyname)
   * @param String Value
   */
  function __set($key, $value)
  {
    $this->fDebug->add(get_class($this) . ": Validating $key against ValidEntities for assignment.", 8);
    if ( array_search($key, $this->validEntities) === false )
    {
      $this->fDebug->add(get_class($this) . ': Attempt to set an invalid key: ' . $key, 4);
      throw new Exception('Attempt to set an invalid ' . get_class($this) . ' entity.', 0);
    }
    $this->entity[$key] = $value;

    // Hide passwords
    if ( $key == "password" ) $value = "*** masked ***";
    $this->fDebug->add(get_class($this) . ": Setting $key = $value", 6);
  }

  /**
   * PHP Object property overload
   *
   * @param String Keyname ($Object->Keyname)
   * @return String Value
   */
  function __get($key)
  {
    if ( !isset($this->fDebug) ) $this->fDebug = fDebug::getInstance();
    $this->fDebug->add(get_class($this) . ": Validating $key against ValidEntities for referencing.", 8);
    if ( !isset($this->entity[$key]) && array_search($key, $this->validEntities) === false )
    throw new Exception(get_class($this) . ": Attempt to reference an invalid " . get_class($this) . " entity \"$key\"", 0);

    $this->fDebug->add(get_class($this) . ": Returning [$key]: " . $this->entity[$key], 8);
    return $this->entity[$key];
  }

  /**
   * PHP Object method overload.  This is not supported.  Return a warning if someone tries to use it.
   *
   * @param String function name
   * @param Array arguments
   */
  function __call($func, $args)
  {
    $args;
    throw new Exception("Undefined function called:  " . get_class($this) . "::" . $func . "()");
  }

  /**
   * Add a key to the internal array that specifies it's a valid entity
   *
   * @param String Key
   */
  protected function validEntity($key)
  {
    if ( !isset($this->fDebug) ) $this->fDebug = fDebug::getInstance();
    if ( array_search($key, $this->validEntities) !== false )
    {
      trigger_error(get_class($this) . ": Attempting to add a duplicate valid entity '$key'.", E_USER_WARNING);
      return;
    }
    $this->entity[$key] = null;
    $this->validEntities[] = $key;
    $this->fDebug->add(get_class($this) . ": Adding $key to the valid entity list", 8);
  }


  /**
   * Validate that the key being asked for is a valid entity set with ValidEntity()
   *
   * @param String Key
   * @return bool is Valid
   */
  protected function isEntitySet($key)
  {
    if ( array_search($key, $this->validEntities) === false )
    {
      $this->fDebug->add(get_class($this) . "::isEntitySet(): '$key' is not a valid entity", 6);
      return false;
    }
    return (isset($this->entity[$key]) && $this->entity[$key] !== null);
  }

}
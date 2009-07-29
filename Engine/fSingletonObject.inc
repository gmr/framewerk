<?php // $Id: fSingletonObject.inc 8507 2007-12-20 19:38:42Z jhansche $
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
 * fSingletonObject
 *
 * A default object implementation
 *
 * @author     Gavin M. Roy <gmr@ehpg.net>
 * @link       http://www.framewerk.org
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2004-2007 the Framewerk Development Group
 * @since      2004-02-23
 * @version    $Rev: 8507 $
 * @package    Engine
 * @subpackage Core
 */

abstract class fSingletonObject
{
  /**
   * @var fMain
   */
  protected $fMain;
  /**
   * @var fDebug
   */
  protected $fDebug;
  /**
   * @var fProfile
   */
  protected $fProfile;
  /**
   * @var fWarn
   */
  protected $fWarn;
  
  private $entity = array();
  protected $validEntities = array();
  
  /**
   * @param Object fMain parent
   */
  protected function __construct()
  {
    $this->fMain    = fMain::getInstance();
    $this->fDebug   = fDebug::getInstance();
    $this->fProfile = fProfile::getInstance();
    $this->fWarn    = fWarn::getInstance();
  }

  /**
   * @abstract
   * @static
   * @return mixed
   */
  abstract public static function getInstance();

  /**
   * @todo We added this when we were having destruction sequence errors that I think have otherwise been resolved
   * I'd like to see if we can remove this entirely
   * abstract static function destroy();
   */

  /**
   * PHP Object property overload
   *
   * @param String Keyname ($Object->Keyname)
   * @param String Value
   */
  function __set($key, $value)
  {
    if ( !is_object($this->fDebug))
    $this->fDebug = fDebug::getInstance();

    if ( array_search($key, $this->validEntities) === false )
    throw new Exception('[' . get_class($this) . '] Attempt to set an invalid ' . get_class($this) . ' entity: ' . $key);

    $this->fDebug->add("Setting $key", 8, get_class($this));
    $this->entity[$key] = $value;
  }

  /**
   * PHP Object property overload
   *
   * @param String Keyname ($Object->Keyname)
   * @return String Value
   */
  function __get($key)
  {
    if ( !is_object($this->fDebug))
    $this->fDebug = fDebug::getInstance();

    if ( array_search($key, $this->validEntities) === false )
      throw new Exception('[' . get_class($this) . '] Attempt to get an invalid ' . get_class($this) . ' entity: ' . $key);

    $this->fDebug->add("Returning [$key]", 8, get_class($this));
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
  protected function validEntity($key, $default = '')
  {
    if ( array_search($key, $this->validEntities) )
    throw new Exception('Attempting to add a duplicate valid entity: ' . $key);

    $this->entity[$key] = $default;
    $this->validEntities[] = $key;

    if ( !is_object($this->fDebug))
    $this->fDebug = fDebug::getInstance();
    $this->fDebug->add('Adding "'  . $key . '" to the valid entity list', 8, get_class($this));
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
      if ( !is_object($this->fDebug))
      $this->fDebug = fDebug::getInstance();
      $this->fDebug->add(get_class($this) . "::isEntitySet(): '$key' is not a valid entity", 6, get_class($this));
      return false;
    }
    return (isset($this->entity[$key]) && $this->entity[$key] !== null);
  }

  static function revision()
  {
    /**
     * @todo figure out if we're able to read ourselves to get our revision
     */
  }
}
?>

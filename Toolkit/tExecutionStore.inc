<?php // $Id:tExecutionStore.inc 876 2007-06-13 16:22:22Z gmr $
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
  * Engine Store Management Object
  *
  * This is for respective functions and objects in the engine to store data for the current execution of the engine
  *
  * @author     Gavin M. Roy <gavinmroy@gmail.com>
  * @link       http://framewerk.org
  * @license    http://opensource.org/licenses/bsd-license.php BSD License
  * @copyright  Copyright 2005-2007 the Framewerk Development Group
  * @version    $Rev:876 $
  * @package    Toolkit
  * @subpackage Caching
  * @uses       fSingletonObject
  * @since      2005-09-08
  */

class tExecutionStore extends fSingletonObject
{

  private static $instance = NULL;
  protected $fMain, $fDebug;


  /**
   * Set a default mime type on initialization
   *
   * @param object pointer for fMain framewerk
   */
  protected function __construct()
  {
    $this->fDebug = fDebug::getInstance();
  }

  /**
   * Checks to see if the object is instanced in the internal static variable $instance
   * and if not, does so
   *
   * @return Object instance
   */
  public static function getInstance()
  {
    if ( self::$instance == NULL )
    self::$instance = new tExecutionStore();
    return self::$instance;
  }

  /**
   * Forcefully unset a singleton instance.
   */
  public static function destroy()
  {
    self::$instance = NULL;
  }

  /**
   * Set a value to be used by an object
   *
   * @param string $app
   * @param string $key
   * @param mixed $value
   */
  function setValue($app, $key, $value)
  {
    if ( is_array($value) ) $value = serialize($value);
    $name = $app . "-" . $key;
    $this->fDebug->add(get_class($this) . "::setValue(): Setting values for $name", 4);
    if ( !$this->isEntitySet($name) ) $this->ValidEntity($name);
    $this->$name = $value;
  }

  /**
   * return a value to an object
   *
   * @param string $app
   * @param string $key
   * @return mixed
   */
  function getValue($app, $key)
  {
    $name = $app . "-" . $key;
    if ( $this->isEntitySet($name) )
    {
      $this->fDebug->add(get_class($this) . "::getValue(): Returning values for $name", 4);
      $out = $this->$name;
      // Check to make sure we're actually something that is serialized
      if ( strstr($out, '{') )
      {
        $temp = unserialize($out);
        if ( is_array($temp) ) return $temp;
      }
      return $out;
    }
    return false;
  }
}
?>

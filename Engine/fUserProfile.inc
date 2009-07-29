<?php // $Id: fUserProfile.inc 919 2007-08-06 18:34:41Z dallas $
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
 * fUserProfile
 *
 * This file contains the fUserProfile object.
 */
/**
 * Base User Profile Class
 *
 * This is the base user profile class for the Framewerk Engine.
 *
 * @author     Joe Hansche <madCoder@gmail.com>
 * @link       http://www.framewerk.org
 * @license    BSD
 * @copyright  Copyright 2005 - 2007 the Framewerk Development Group
 * @version    $Rev: 919 $
 * @since      2006-12-17
 * @package    Engine
 * @subpackage Core
 * @uses       fObject
 */

class fUserProfile extends fObject
{

  private $dataInterfaces;  // Instance of the fUserProfile backend containing the data for this profile

  public $lastActivity;    // Timestamp of last activity

  /**
   * Class Constructor
   *
   * Pass the user id/key
   *
   * Examples
   *   PDO:1
   *   GSSrvx:madCoder
   *
   * @param string $key
   */
  function __construct($key)
  {
    parent::__construct();

    foreach ( $this->fMain->configuration->engine->fUser->profileHandlers->handler as $profileHandler )
    {
      try
      {
        $s = 'fUserProfile' . strval($profileHandler);

        // Make sure it exists and extends fUserProfileBackend
        $r = new ReflectionClass($s);
        if ( !$r->getParentClass() || $r->getParentClass()->getName() !== 'fUserProfileBackend' )
          throw new Exception($s . ' is not a fUserProfile backend object!');

        // Create the object
        $temp = new $s($key);

        // Save the profile object
        $this->dataInterfaces[] = $temp;
      } catch ( Exception $e )
      {
        $this->fWarn->add('Could not create Profile Handler "'.$profileHandler.'".  ' . $e->getMessage());
      }
    }

    return true;
  }

  function __get($key)
  {
    foreach ( $this->dataInterfaces as $data )
    {
      if ( $data->isEntitySet($key) && strlen($data->$key) > 0 )
        return $data->$key;
    }
    return NULL;
  }

  function __set($key, $value)
  {
    foreach ( $this->dataInterfaces as $data )
    {
      if ( $data->isEntitySet($key) )
      {
        $data->$key = $value;
        $this->lastActivity = time();
      }
    }
  }

  /**
   * Save changes to the user data
   *
   * @return bool
   */
  function save()
  {
    $save = false;
    if ( $this->lastActivity === NULL )
      return false;
    foreach ( $this->dataInterfaces as $data )
    {
      if ( $data->save() )
        $save = true;
    }
    return $save;
  }

}
?>

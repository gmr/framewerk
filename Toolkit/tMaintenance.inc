<?php // $Id$
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
 * Maintenance object
 *
 * This object checks for routine maintenance
 *
 * @author     Joe Hansche <jhansche@myyearbook.com>
 * @link       http://framewerk.org
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2005-2007 the Framewerk Development Group
 * @version    $Rev$
 * @package    Toolkit
 */

class tMaintenance
{
  protected $fMain, $fOutput;
  protected $fRegistry;
  protected $halt;
  static $instance;

  private function __construct()
  {
    $this->fRegistry = new fRegistry($this);
    if ( file_exists(strval($this->fRegistry->path)) )
    {
      $this->halt = true;
      $this->message = strval($this->fRegistry->message);
    } else {
      $this->halt = false;
    }
  }
  
  public static function check()
  {
    if ( self::$instance === NULL )
    {
      self::$instance = new tMaintenance();
    }
    if ( self::$instance->halt === true )
    {
      return self::$instance->message;
    }
    return false;
  }
}
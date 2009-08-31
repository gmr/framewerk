<?php // $Id: tEmail.inc 914 2007-08-06 17:45:17Z gmr $
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
 * GUID Class
 */
/**
 * Wrapper for Jason Farrell's genguid() function: http://jasonfarrell.com/misc/guid.php
 *
 * @author     Gavin M. Roy <gavinmroy@gmail.com>
 * @link       http://framewerk.org
 * @since      2007-08=06
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2007 the Framewerk Development Group
 * @version    $Rev: 914 $
 * @package    Toolkit
 * @uses       fObject
 */

class tGUID
{

  /**
   * Null Private Constructor - just use this as tGUID::generate();
   *
   * @return null
   */
  private function __construct()
  {
    return null;
  }

  /**
   * Generates a random GUID per http://www.ietf.org/rfc/rfc4122.txt
   *
   * @return String
   */
  static function generate() {
    //e.g. output: 372472a2-d557-4630-bc7d-bae54c934da1
    //word*2-, word-, (w)ord-, (w)ord-, word*3
    $guidstr = "";
    for ($i=1;$i<=16;$i++)
    {
      $b = (int)rand(0,0xff);
      if ($i == 7) { $b &= 0x0f; $b |= 0x40; } // version 4 (random)
      if ($i == 9) { $b &= 0x3f; $b |= 0x80; } // variant
      $guidstr .= sprintf("%02s", base_convert($b,10,16));
      if ($i == 4 || $i == 6 || $i == 8 || $i == 10) { $guidstr .= '-'; }
    }
    return $guidstr;
  }
}
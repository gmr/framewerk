<?php // $Id: LazyObject.inc 903 2007-07-24 20:07:07Z gmr $
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
   * Lazy Object Processor
   *
   * This is meant to be the last default handler.  If nothing was able to process the request
   * then we'll try one last effort to try and instance the $key as an object and execute it.
   *
   * @since      2005-02-12
   * @author     Gavin M. Roy <gavinmroy@gmail.com>
   * @link       http://framewerk.org
   * @license    http://opensource.org/licenses/bsd-license.php BSD License
   * @copyright  Copyright 2005-2007 the Framewerk Development Group
   * @package    Applications
   * @subpackage Base
   * @uses       fApplications
   * @version    $Rev: 903 $
   */

class LazyObject extends fApplication
{
  function execute($key = NULL)
  {
    if ( class_exists($key) )
    {
      $this->fDebug->add("Loading: $key", 4);
      $obj = new $key();
      return $obj->execute();
    } else
      return false;
  }
}

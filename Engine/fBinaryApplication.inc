<?php // $Id: fBinaryApplication.inc 919 2007-08-06 18:34:41Z dallas $
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
 * fBinaryApplication
 *
 * Contains the fBinaryApplication abstract class
 */
/**
 * Binary Application abstract object
 *
 * The Binary Application object is for sending out binary data such as graphics, PDF's, zip files, executables, or Flash.
 *
 * @author     Gavin M. Roy <gavinmroy@gmail.com>
 * @link       http://framewerk.org
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2005-2007 the Framewerk Development Group
 * @version    $Rev: 919 $
 * @package    Engine
 * @subpackage Abstract_Applications
 * @since      2005-09-10
 * @abstract
 */

abstract class fBinaryApplication extends fApplication
{

  /**
   * Application Constructor
   *
   * Sets the all the needed internal variables and engine parameters.
   */
  function __construct()
  {
    parent::__construct();

    // Set our page category and name
    $this->pageCategory('default');
    $this->pageName('The object you called did not properly extend the fApplication Object');

    // Set the Mime Type to something other than text/html so the parser doesnt parse it,
    // objects extending this Application should set their mime type
    $this->fOutput->setMimeType('invalid/invalid');

    // Don't use the theme or try and merge them
    $this->fTheme->useTheme(false);
  }
  
}
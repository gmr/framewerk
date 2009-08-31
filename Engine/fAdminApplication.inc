<?php // $Id: fAdminApplication.inc 865 2007-05-27 20:15:16Z gmr $
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
 * fAdminApplication
 *
 * Contains the fAdminApplication Class
 */
/**
 * Admin Application abstract object
 *
 * Abstract Class for Admin Applications
 *
 * @author     Gavin M. Roy <gavinmroy@gmail.com>
 * @link       http://framewerk.org
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2005-2007 the Framewerk Development Group
 * @version    $Rev: 865 $
 * @package    Engine
 * @subpackage Abstract_Applications
 * @uses       fApplication
 */

abstract class fAdminApplication extends fApplication
{

  protected $baseURI;

  /**
   * Application Constructor
   *
   * Sets the all the needed internal variables and engine parameters.
   *
   * @param object fMain object is required
   */
  function __construct()
  {
    parent::__construct();

    // Override to use our Administration theme
    $this->fTheme->override($this->fMain->configuration->site->theme->administration);

    // Set a default pagename and  category
    $this->pageName('Administration Page');
    $this->pageCategory('Administration');

    // Set the baseURI
    $this->baseURI = $this->fMain->configuration->site->baseURI . $this->fMain->keyword . '/';
  }

  /**
   * Add an application option for the Administrative options index
   *
   * @param String option name
   * @param String option url
   * @param string permission
   */
  function addOption($name, $url, $permission, $selected = false)
  {
    if ( is_object($this->fUser) )
    {
      if ( $this->fUser->checkPermission($permission) )
      {
        $store = fExecutionStore::getInstance();
        $values = $store->getValue('fAdminApplication', 'Options');
        $values[] = array($name, $url, $permission, $selected);
        $store->setValue('fAdminApplication', 'Options', $values);
      }
    }
  }
}
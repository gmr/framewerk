<?php // $Id: tTinyMCE.inc 914 2007-08-06 17:45:17Z gmr $
############################################################################
#    Copyright (C) 2006-2007 by the Framewerk Development Group            #
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
 * tTinyMCE
 *
 * This file contains the TTinyMCE Class
 */
/**
 * TinyMCE Support Class
 *
 * This class is responsible for building the JavaScript headers needed for multiple tinymce instances
 *
 * @author     Gavin M. Roy <gavinmroy@gmail.com>
 * @link       http://framewerk.org
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2006-2007 the Framewerk Development Group
 * @version    $Rev: 914 $
 * @package    Toolkit
 * @subpackage Widgets
 * @uses       fSingletonObject
 * @since      2006-06-08
 */

class tTinyMCE extends fSingletonObject
{
  static private $instance = NULL;
  private $fTheme, $element, $registry;

  /**
   * Set a default mime type on initialization
   */
  protected function __construct()
  {
    parent::__construct();

    $this->element = array();
    $this->registry = new fRegistry($this);
    $this->fTheme = fTheme::getInstance();
  }

  /**
   * Get the current or new instance
   *
   * @return Object instance of this class
   */
  static function getInstance()
  {
    if ( self::$instance == NULL )
      self::$instance = new tTinyMCE();
    return self::$instance;
  }

  /**
   * Add an element
   *
   * @param string $name
   * @param string $configType
   */
  function addElement($name, $configType)
  {
    $this->element[$name]['type'] = $configType;
    $this->element[$name]['value'] = $this->registry->$configType;
  }

  /**
   * Build the HTML header for this class
   */
  function buildHeader()
  {
    // Build the first part of the output
    $javascript = '<script language="javascript" type="text/javascript" src="/JavaScript/tiny_mce/tiny_mce.js"></script>' . "\n";
    $javascript .= '<script language="javascript" type="text/javascript">' . "\n";

    foreach ( $this->element AS $name=>$element )
    {
      $theme = $element['value'];
      $javascript .= <<<JAVASCRIPT
tinyMCE.init({
 mode : "exact",
 elements : "$name",
 $theme
});
JAVASCRIPT;
    }
    $javascript .= "</script>\n";
    $this->fTheme->htmlHead($javascript);
  }
}
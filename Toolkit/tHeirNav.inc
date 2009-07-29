<?php // $Id: tHeirNav.inc 914 2007-08-06 17:45:17Z gmr $
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
 * Hierarchy Navigation Object
 *
 * Standard Hierarchical Navigation Widget for managing a hierarchy of nav links
 *
 * @author     Joe Hansche <madcoder@gamesurge.net>
 * @link       http://framewerk.org
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2005-2007 the Framewerk Development Group
 * @version    $Rev: 914 $
 * @package    Toolkit
 * @subpackage Widgets
 * @uses       fSingletonObject
 * @todo       Make this XML/XSL based?
 */

class tHierNav extends fSingletonObject
{
  static $instance;

  protected $links;
  protected $fMain;

  protected function __construct()
  {

    parent::__construct();
    $this->fMain = fMain::getInstance();

    fTheme::getInstance()->addObjectCSSFile("THierNav");

    $this->validEntity('separator');
    $this->validEntity('linkLastItem');

    // Set some defaults
    $this->separator = " &raquo; ";
    $this->linkLastItem = false;
  }

  static function getInstance()
  {
    if ( self::$instance === NULL )
      self::$instance = new THierNav();
    return self::$instance;
  }

  static function destroy()
  {
    if ( self::$instance === NULL ) return;
    self::$instance = NULL;
  }

  function addPage($title, $url)
  {
    $this->links[] = array($title, $url);
  }

  protected function returnLink($linkData)
  {
    return '<a href="' . $linkData[1] . '" title="'.$linkData[0].'">'.$linkData[0].'</a>';
  }

  /**
   * Display our Page List
   */
  function display()
  {
    $pages = array();
    if ( count($this->links) == 0 ||
    ( count($this->links) == 1 && $this->linkLastItem !== true ) )
    {
      return "";
    }
    foreach( $this->links as $i => $linkData )
    {
      $pages[] = $this->returnLink($linkData);
    }
    if ( $this->linkLastItem !== true )
    {
      unset($pages[count($pages) - 1]);
      $pages[] = '<span title="'. $linkData[0] .'">' . $linkData[0] . '</span>';
    }
    return '<div class="tHierNav">' . join($this->separator, $pages) . '</div>';
  }
}
?>

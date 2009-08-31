<?php // $Id: tBreadCrumb.inc 945 2007-12-20 21:10:24Z madcoder $
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
   * tBreadCrumb
   *
   * Creates a bread crumb path technique from a given keyword,  or the Framewerk matched keyword
   *
  * @author     Fred Ghosn <fredghosn@gmail.com>
  * @link       http://framewerk.org
  * @license    http://opensource.org/licenses/bsd-license.php BSD License
  * @copyright  Copyright 2006-2007 the Framewerk Development Group
  * @version    $Rev: 945 $
  * @package    Engine
  * @subpackage Toolkit
  * @uses       fMainSingletonObject
  * @todo       Move display logic to XSL
  * @since      2006-10-15
  */

class tBreadCrumb extends fSingletonObject
{
  private $extraCrumbs = array();
  private static $instance = NULL;

  protected function __construct() {
    parent::__construct();
  }

  public static function getInstance()
  {
    if ( self::$instance === NULL )
    {
      self::$instance = new tBreadCrumb();
    }
    return self::$instance;
  }

  public function display($keyword = NULL)
  {
    if ( $keyword == NULL )
    {
      $keyword = $this->fMain->keyword;
    }

    $crumbs = explode('/', $keyword);
    $string = '';
    $path   = '';
    $xpath  = '/site';

    foreach( $crumbs as $crumb )
    {
      $path  .= '/' . $crumb;
      $xpath .= '/key[@uri=\'/' . $crumb . '\']';
      $key    = $this->fMain->site->xpath($xpath);
      $title  = ( count($key) > 0 ? strval($key[0]->title) : '' );
      $string .= '<a href="' . $path . '">{!i18n:' . ((strlen($title) ? $title : $path)) . '}</a> &gt; ';
    }

    foreach ( $this->extraCrumbs as $link => $title)
    {
      if ( !is_numeric($link) )
      {
        $string .= '<a href="' . $link . '">{!i18n:';
      }
      $string .= ((strlen($title) > 0 ? $title : $link));
      if ( !is_numeric($link) )
      {
        $string .= '}</a>';
      }
      $string .= ' &gt; ';
    }

    return rtrim($string, ' &gt; ');
  }

  public function addCrumb($title = NULL, $link = NULL)
  {
    if ( trim($link.$title) == '' ) return false;
    if ( $link === NULL )
    {
      $this->extraCrumbs[] = $title;
    } else {
      $this->extraCrumbs[$link] = $title;
    }
    return true;
  }
}
<?php // $Id: tAZNav.inc 914 2007-08-06 17:45:17Z gmr $
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
 * A-Z Navigation Object
 *
 * Standard A-Z Navigation Object
 *
 * @author     Gavin M. Roy <gavinmroy@gmail.com>
 * @link       http://framewerk.org
 * @since      2005-12-12
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2005-2007 the Framewerk Development Group
 * @version    $Rev: 914 $
 * @package    Toolkit
 * @subpackage Widgets
 * @uses       fObject
 */

class tAZNav extends fObject
{

  /**
   * @param Object TFramewerk parent
   */
  function __construct()
  {
    $this->validEntity('currentPage');
    $this->validEntity('delimiter');
    $this->validEntity('letters');

    $this->letters = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ';
    $this->delimiter = ' - ';
    $this->getPageLetter();
  }


  /**
   * Display our Page List
   *
   * @return string Page List
   */
  function display($baseURL)
  {
    $output = "";
    for ( $y = 0; $y < strlen($this->letters); $y++ )
    if ( $this->letters[$y] == $this->currentPage )
    {
      $output .= '<strong>' . $this->letters[$y] . '</strong>' . $this->delimiter;
    } else {
      $output .= '<a href=\'' . $baseURL . '/' . $this->letters[$y] . '/\'>' . $this->letters[$y] . '</a>' . $this->delimiter;
    }
    $output = rtrim($output, $this->delimiter);
    return $output;
  }

  /**
   * Return the current page letter
   *
   * @return string Page Letter
   */
  function getPageLetter()
  {
    // Strip off the last / if it's there
    $uri = $_SERVER['REQUEST_URI'];
    $uri = rtrim($uri, "/");
    $data = explode("/", $uri);
    $page = array_pop($data );
    fDebug::getInstance()->add("Parsed current page letter as $page", 5);
    if ( strlen($page) != 1 )
    {
      $page = substr($this->letters, 0, 1);
    }
    $this->currentPage = $page;
    return $page;
  }
}
?>
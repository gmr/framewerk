<?php // $Id: tPageNav.inc 914 2007-08-06 17:45:17Z gmr $
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
 * Page Navigation Object
 *
 * Standard Page Navigation Widget for managing pages of lists
 *
 * @author     Gavin M. Roy <gavinmroy@gmail.com>
 * @link       http://framewerk.org
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2005-2007 the Framewerk Development Group
 * @version    $Rev: 914 $
 * @package    Toolkit
 * @subpackage Widgets
 * @uses       fObject
 */

class tPageNav extends fObject
{

  /**
   * @param Object fMain parent
   */
  function __construct()
  {
    $this->fMain = fMain::getInstance();

    // Setup our valid entities
    $this->validEntity("items");
    $this->validEntity("itemsPerPage");
    $this->validEntity("currentPage");
    $this->validEntity("baseURI");
    $this->validEntity("offset");
    $this->validEntity("displayPages");
    $this->validEntity("pages");
    $this->validEntity('appendURL');

    // Set some defaults
    $this->itemsPerPage  = 25;
    $this->displayPages  = 7;
    $this->pages = 0;
    $this->currentPage   = $this->getPageNumber();
  }

  /**
   * Return a link
   *
   * @param string $page
   * @param string $title
   * @param string $label
   * @return string
   */
  function returnLink($page, $title = NULL, $label = NULL)
  {
    if ( $title == NULL ) $title = $page;
    if ( $label == NULL ) $label = $page;

    // strip off extra trailing slashes
    $this->baseURI = rtrim($this->baseURI, '/');

    // No need to hyperlinkt he current page
    if ( $page == $this->currentPage )
    return "$page ";

    $appendURL = "";
    if ( strlen($this->appendURL) )
    $appendURL = "?" . $this->appendURL;

    return "<a href=\"" . $this->baseURI . "/$page/$appendURL\" title=\"{!i18n:$title}\">{!i18n:$label}</a> ";
  }

  /**
   * Display our Page List
   *
   * @return string Page List
   */
  function display($short = false, $linkAllPages = false)
  {
    // How many pages do we have?
    $this->getPages();

    // Return nothing if we only have one page
    if ( $this->pages == 1 ) return;

    // Set our previous and next page
    $previous = intval($this->currentPage - 1);
    $next     = intval($this->currentPage + 1);

    // Output debugging info about how we're building the page
    $debug = fDebug::getInstance();
    $debug->add("TPageNav: " . $this->items . " Items", 4);
    $debug->add("TPageNav: " . $this->itemsPerPage . " Items per page", 4);
    $debug->add("TPageNav: " . $this->currentPage . " is the current page", 4);
    $debug->add("TPageNav: " . $this->pages . " Pages", 4);

    $output = "";
    $midpoint = intval(intval($this->displayPages) / 2);

    // Show the first Page Option
    if ( !$short && ( intval( $this->currentPage - $midpoint) > 1 ) && ( $this->pages > $this->displayPages ) )
    $output .= $this->returnLink(1, "First Page", "&laquo; First Page") . "... ";


    // Show the Previous Page Option
    if ( !$short && $this->currentPage > 1 )
    $output .= $this->returnLink($previous, "Previous Page", "&laquo;");

    // Get our starting and ending #
    $start = 1;
    if ( $this->pages > $this->displayPages )
    if ( intval($this->currentPage) - $midpoint > 1 )
    $start = intval($this->currentPage) - $midpoint;

    // Find the last possible page
    $end = $this->pages + 1;
    if ( $this->pages > ( $start + $midpoint ) )
    $end = $start + intval($this->displayPages);
    if ( $end > $this->pages ) $end = $this->pages + 1;

    if ( ( $end - $this->currentPage ) <= $midpoint ) $start -= ( $midpoint - ( $end - $this->currentPage ) );
    if ( $start < 1 ) $start = 1;


    // Make our links
    for ( $y = $start; $y < $end; $y++ )
    $output .= $this->returnLink($y);


    // Show the Next Page Option
    if ( !$short && $next <= $this->pages )
    $output .= $this->returnLink($next, "Next Page", "&raquo;");

    // Show the Last Page option
    if ( ( intval($this->pages - $this->currentPage) > $midpoint ) && ( $this->pages > $this->displayPages ) )
    {
      if ( !$short )
      $output .=  "... " . $this->returnLink($this->pages, "Last Page", "Last Page &raquo;");
      else
      $output .=  ".. " . $this->returnLink($this->pages, "Last Page", "&raquo;");
    }

    return $output;
  }

  /**
   * Return the current page number
   *
   * @return int Page Number
   */
  function getPageNumber()
  {
    // Strip off the last / if it's there
    $uri = $_SERVER['REQUEST_URI'];
    $uri = rtrim($uri, "/");
    $data = explode("/", $uri);
    $last = array_pop($data );
    $page = 1;
    $this->getPages();
    if ( is_numeric($last) && ( $last <= $this->pages ) )
    $page = $last;
    fDebug::getInstance()->add("Parsed current page number as $page", 5);
    return $page;
  }

  /**
   * Get the Page Count
   *
   * @return int Page Count
   */
  function getPages()
  {
    $this->pages = ceil(intval($this->items) /  intval($this->itemsPerPage));
    return $this->pages;
  }

  /**
   * Get the count offset
   *
   * @return int Offset
   */
  function getOffset()
  {
    $this->getPages();
    $offset = intval($this->currentPage * $this->itemsPerPage) - $this->itemsPerPage;
    return $offset;
  }

}
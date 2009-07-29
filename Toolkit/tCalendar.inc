<?php // $Id: tCalendar.inc 914 2007-08-06 17:45:17Z gmr $
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
 * Calendar object
 *
 * @author     Joe Hansche <madcoder@gamesurge.net>
 * @link       http://framewerk.org
 * @since      2005-07-24
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2005-2007 the Framewerk Development Group
 * @version    $Rev: 914 $
 * @package    Toolkit
 * @subpackage Widgets
 */

class tCalendar
{
  /* Presentation variables */
  protected $display; // View type [month|week]
  protected $view;    // View detail [small|large]

  /* Date variables */
  protected $today; //getdate() of today
  protected $active;//getdate() of viewport date
  protected $start; //getdate() of starting date
  protected $end;   //getdate() of ending date
  protected $numDays;

  /* XML variables */
  protected $dom;  // DOM document
  protected $root; // Root element

  /* Internal variables */
  protected $registry;
  protected $processed = false;

  function __construct($display = "month", $view = "small", $activeDate = null)
  {

    $this->registry = new fRegistry("tCalendar");
    if ( $this->registry->showDatesOutsideMonth !== false && $this->registry->showDatesOutsideMonth !== true )
    $this->registry->showDatesOutsideMonth = true;

    /* Setup XML document root */
    $this->dom = new DOMDocument("1.0");
    $this->dom->formatOutput = true;
    $this->root = $this->dom->appendChild($this->dom->createElement("calendar"));
    $this->root->setAttribute("display", $display);
    $this->root->setAttribute("view", $view);

    $this->display = $display;
    $this->view = $view;

    $this->today = getdate();

    /* Set active date */
    if ( $activeDate !== null && ($t = strtotime($activeDate)) > 0 )
    {
      $this->active = getdate($t);
    }
    else if ( is_array($activeDate) && isset($activeDate['mday']) )
    $this->active = $activeDate;
    else
    $this->active = getdate(strtotime("00:00:00"));

    /* Set start/end based on active date and display */
    if ( $display == "month" )
    {
      $this->start = getdate(mktime(0, 0, 0, $this->active['mon'], 1, $this->active['year']));
      $this->end = getdate(mktime(0, 0, 0, $this->active['mon'] + 1, 0, $this->active['year']));
      $this->numDays = $this->end['mday'];

      $this->root->setAttribute("title", $this->active['month'] . " " . $this->active['year']);
    } else if ( $display == "week" ) {
      $this->start = getdate(strtotime("Last Sunday", $this->active[0] + 3600*24));
      $this->end = getdate(strtotime("Saturday", $this->active[0]));
      $this->numDays = 7;

      $this->root->setAttribute("title", strftime("%x", $this->start[0]) . " - " . strftime("%x", $this->end[0]));
    }
  }

  function process()
  {

    // Start on sunday
    if ( $this->start['wday'] > 0 )
    $d = getdate(strtotime("Last Sunday", $this->start[0]));
    else
    $d = $this->start;

    do
    {
      if ( $d['wday'] == 0 && $d['mon'] <= $this->end['mon'] )
      {
        $weekNode = $this->root->appendChild($this->dom->createElement("week"));
        $weekNode->setAttribute("num", date("W", $d[0]));
      }
      $dayNode = $weekNode->appendChild($this->dom->createElement("day"));
      //  $dayNode->setAttribute("timestamp", $d[0]);
      $dayNode->setAttribute("mday", $d['mday']);
      $dayNode->setAttribute("mon", $d['mon']);
      $dayNode->setAttribute("year", $d['year']);
      $dayNode->setAttribute("wday", $d['wday']);
      $dayNode->setAttribute("yday", $d['yday']);
      $dayNode->setAttribute("title", strftime("%x", $d[0]));

      if ( date("Ymd", $d[0]) === date("Ymd", $this->today[0]) )
      $dayNode->setAttribute("today", true);
      if ( $d['mon'] !== $this->active['mon'] )
      {
        $dayNode->setAttribute("outside", true);
        echo $this->registry->showDatesOutsideMonth;
        if ( !($this->registry->showDatesOutsideMonth) )
        $dayNode->setAttribute("hidden", true);
      }

      if ( $temp = $this->getEvents($d) )
      {
        $dayNode->appendChild($temp);
      }

      $d = getdate(strtotime("tomorrow", $d[0]));
      // Continue until end of month and week
    } while($d[0] <= $this->end[0] || $d['wday'] > 0);

    $this->processed = true;
  }

  private function getEvents($d)
  {
    $count = 0;
    $events = $this->dom->createElement("events");
    if ( date("Ymd", $d[0]) === date("Ymd", $this->today[0]) )
    {
      $count++;
      $tempNode = $events->appendChild($this->dom->createElement("event"));
      $tempNode->setAttribute("type", "CalendarInfo");
      $tempNode->appendChild($this->dom->createTextNode("Today!"));
    }

    if ( $count > 0 )
    return $events;
    else return false;
  }

  function getXML()
  {
    if ( !$this->processed )
    $this->process();
    return $this->dom->saveXML();
  }

  function getXHTML()
  {
    $xml = $this->getXML();
    return TTheme::transformXML(TTheme::xslFile("tCalendar"), $xml);
  }

}
?>
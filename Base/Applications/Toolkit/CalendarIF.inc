<?php // $Id: CalendarIF.inc 919 2007-08-06 18:34:41Z dallas $
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
   * Calendar Interface.
   *
   * @since      2005-07-15
   * @author     Joe Hansche <madcoder@gamesurge.net>
   * @link       http://framewerk.org
   * @license    http://opensource.org/licenses/bsd-license.php BSD License
   * @copyright  Copyright 2005-2007 the Framewerk Development Group
   * @package    Applications
   * @subpackage Base
   * @uses       fApplication
   * @version    $Rev: 919 $
   */

class CalendarIF extends fApplication 
{
  function block()
  {
    $cal = new fCalendar("month", "small");
    //XXX: use the calendar title as the block title
    $this->blockTitle = "";
    return $cal->getXHTML() . "<div><a href='/calendar/'>View Calendar</a></div>";
  }
  
  function execute()
  {
    $this->pageName("{!i18n:Calendar of Events}");
    $this->fTheme->addObjectCSSFile('fCalendar');
    $cal = new fCalendar("month", "small");
    $xml = $cal->getXML();
    $this->output($xml, true, 'fCalendar');
    $this->output('<hr />');
    
    //XXX: This is for debugging only:
    $this->output(nl2br(htmlentities($cal->getXML())));
    
    return true;
  }
 
}
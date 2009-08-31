<?php // $Id: xpath.inc 826 2007-03-28 23:43:57Z gmr $
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
  * Framewerk 2.0 XPath Routines.
  *
  * Routines for setting simplexml node values and types based upon an xpath
  *
  * @since      2005-01-04
  * @author     Joe Hansche <madCoder@gamesurge.net>
  * @version    $Rev: 826 $
  * @license    http://opensource.org/licenses/bsd-license.php BSD License
  * @copyright  Copyright 2005-2007 the Framewerk Development Group
  * @package    Engine
  * @subpackage Library
  */

function SetXPath($xml, $xpath, $value)
{
  $temp = $this->DrillXPath($xml, $xpath);
  $temp['type'] = gettype($value);
  $temp[0] = $value;
}

function DrillXPath($xml, $xpath)
{
  $temp = $xml;
  $xpath = preg_replace("/^[\/]+/", "", $xpath);
  $parts = split("/", $xpath);
  foreach($parts as $key)
  {
    if ( isset($temp->$key) )
    {
      $this->parent->debug("Drilling $key", 1);
      $temp = $temp->$key;
    } else return $this->Out->error("$key");
  }
  return $temp;
}
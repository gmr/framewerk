<?php // $Id: ticks.inc 930 2007-09-09 18:04:34Z gmr $
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
  * Tick debugging functions
  *
  * This program will check Configuration/registry.xml to see if ticks are enabled
  * us turned on, and if so it will register the functions to use it
  *
  * @author     Gavin M. Roy <gavinmroy@gmail.com>
  * @author     Joe Hanshe <madcoder@framewerk.org>
  * @link       http://framewerk.org
  * @license    http://opensource.org/licenses/bsd-license.php BSD License License
  * @copyright  Copyright 2004-2007 the Framewerk Development Group
  * @version    $Rev: 930 $
  * @package    Engine
  * @subpackage Core
  * @since      2005-09-07
  */

// Load the registry file
$xml = @simplexml_load_file('Configuration/registry.xml');
if ( !is_object($xml) )
  die('Fatal Error: ticks are enabled but the registry file can not be read.');

// Get the ticks node
$ticksNode = $xml->xpath("/registry/node[@name='ticks']");

// Check to see if ticks are enabled
if ( $ticksNode[0]->enabled )
{
  $temp = "debugTick" . UCWords($ticksNode[0]->method);
  if ( is_callable($temp) )
  {
    $foo = register_tick_function($temp); // file, print
    if ( $temp == 'debugTickFile' )
    {
      $filename = rtrim($ticksNode[0]->directory, '/') . '/' . uniqid() . '.ticklog';
      $fp = fopen($filename, 'w');
      fputs($fp, "New Request: " . $_SERVER['REQUEST_URI'] . "\n\n");
    }
    declare(ticks=2);
  }
}

function debugTickPrint()
{
  static $tickCalls = 0;
  $temp = debug_backtrace();
  $tickCalls++;
  $call = $temp[1];
  $file = explode('/', $call['file']);
  $file = $file[Count($file) - 1];
  $output = "Tick #$tick_calls: $SERVER[REQUEST_URI] $file $call[line] $call[class] $call[function]<br />\n";
  echo $output;
}

function debugTickFile()
{
  global $fp;
  static $tickCalls = 0;

  $temp = debug_backtrace();
  $tickCalls++;
  $call = $temp[1];
  $file = explode('/', $call['file']);
  $file = $file[Count($file) - 1];
  if ( !isset($call['class']) ) $call['class'] = 'Unknown';
  if ( !isset($call['file']) ) $call['file'] = 'Unknown';
  if ( !isset($call['line']) ) $call['line'] = 'Unknown';
  $output = "Tick #$tickCalls: [ $_SERVER[REQUEST_URI] ] - $file $call[line] - $call[class] - $call[function]\n";
  fputs($fp, $output);
}

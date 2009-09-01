<?php // $Id$
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
 * Class Information
 *
 * Uses reflection to pull header comments
 *
 * @author     Gavin M. Roy <gavinmroy@gmail.com>
 * @link       http://framewerk.org
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2007 the Framewerk Development Group
 * @since      2007-02-27
 * @version    $Rev: 0 $
 * @package    Engine
 * @subpackage Core
 */

class tClassInfo
{
  private $values;

  /**
   * Constructor
   *
   * Pass in the class name (string) or ReflectionClass to get key/value pairs for
   *
   * @param string|ReflectionClass $class
   */
  function __construct($class)
  {
    if ($class instanceof ReflectionClass)
      $reflection = $class;
    else
      $reflection = new ReflectionClass($class);
    
    // Get our comments
    $comments = $reflection->getDocComment();
    
    // Explode the comments into an array via the linefeed
    $lines = explode("\n", $comments);

    // Setup our value array and set the first few entries based upon hard line format values
    $this->values = array();
    $this->values['name'] = trim(ltrim($lines[1], ' * '));
    $this->values['desc'] = trim(ltrim($lines[3], ' * '));

    // Loop through the lines and parse out the key/value pairs
    foreach ( $lines as $line )
    {
      // Get the output of a parsed line
      $x = $this->parseLine($line);
      // If the output is an array, merge the arrays
      if ( is_array($x) ) $this->values = array_merge($this->values, $x);
    }
  }

  /**
   * Break out the key/value pair from a comment line
   *
   * @param String $line
   * @return Array
   */
  function parseLine($line)
  {
    $keyword = '';
    $mode = '';
    $value = '';
    for ( $y = 0; $y < strLen($line); $y++ )
    {
      switch ( $mode )
      {
        case 'keyword':
          if ( $line[$y] != ' ' )
          {
            $keyword .= $line[$y];
          } else {
            $mode = 'value';
            $started = false;
          }
          break;
        case 'value':
          $value .= $line[$y];
          break;
        default:
          if ( $line[$y] == '@' ) $mode = 'keyword';
          break;
      }
    }

    // If the keyword is version, strip out the svn Rev: format info
    if ( $keyword == 'version' )
    {
      $temp = explode(':', $value);
      $value = trim(rtrim($temp[1], '$'));
    }

    // If we have a keyword, return the array
    if ( strlen($keyword) ) return array($keyword => trim($value));
  }

  // Get a variable from the class info value array
  function __get($variable)
  {
    return $this->values[$variable];
  }

}
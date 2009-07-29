<?php // $Id$

/**
 * Function/Method Information
 *
 * Uses reflection to pull header comments
 *
 * @author     Gavin M. Roy <gavinmroy@gmail.com>
 * @author     Dallas Gutauckis <dgutauckis@myyearbook.com>
 * @since      2007-07-23
 * @package    Engine
 * @subpackage Core
 */

class fFunctionInfo
{
  private $values = array('return' => array());

  /**
   * Constructor
   *
   * Pass in the ReflectionFunction, or ReflectionMethod object to get key/value pairs for
   *
   * @param ReflectionFunction|ReflectionMethod $object
   */
  function __construct($function)
  {
    // If they pass us a Reflection[Function/Method] object, we can use that to getDocComment
    if ($function instanceof ReflectionFunction || $function instanceof ReflectionMethod)
      $reflection = $function;
   
    // Get our comments
    $comments = $reflection->getDocComment();
    
    // Explode the comments into an array via the linefeed
    $lines = explode("\n", $comments);

    // Setup our value array and set the first few entries based upon hard line format values
    $this->values = array();
    $this->values['desc'] = trim(ltrim($lines[1], ' * '));

    // Loop through the lines and parse out the key/value pairs
    foreach ( $lines as $line )
    {
      $this->parseLine($line);
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
    switch ($keyword)
    {
      case 'version':
        $temp = explode(':', $value);
        $value = trim(rtrim($temp[1], '$'));
      break;
      case 'param':
        $temp = explode(' ', $value,3);
        $stdClass = new stdClass();
        $stdClass->type = $temp[0];
        $stdClass->name = substr($temp[1],1);
        $stdClass->description = $temp[2];
        $this->values['param'][$stdClass->name] = $stdClass;
      case 'return':
        $temp = explode(' ', $value,3);
        $stdClass = new stdClass();
        $stdClass->type = $temp[0];
        $stdClass->name = substr($temp[1],1);
        $stdClass->description = $temp[2];
        $this->values['return'] = $stdClass;
      break;
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
?>

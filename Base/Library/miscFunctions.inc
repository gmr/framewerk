<?php // $Id: miscFunctions.inc 919 2007-08-06 18:34:41Z dallas $
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
 * Misc Functions.
 *
 * Misc routines that dont fit anywhere else.  This is for complete
 * Framewerk wide functions.
 *
 * @author     Gavin M. Roy <gavinmroy@gmail.com>
 * @author     Joe Hansche <madcoder@gamesurge.net>
 * @link       http://framewerk.org
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2005-2007 the Framewerk Development Group
 * @version    $Rev: 919 $
 * @package    Engine
 * @subpackage Library
 * @since
 */


/**
 * Returns a random value
 *
 * Usually used in the format {@random:min:max}
 * @param array $values
 * @return string
   */
function random($values = NULL)
{
  srand(microtime(true));
  $x = strval(rand($values[0],$values[1]));
  return $x;
}

/**
 * Return the current year
 *
 * Returns the current year in the YYYY format
 * @return string
 */
function currentYear()
{
  return strftime("%Y", time());
}

/**
 * Return the current year
 *
 * Returns the current year in the YYYY format
 * @return string
 */
function currentTimestamp()
{
  return strftime("%X %x", time());
}

/**
 * Get a Language List
 *
 * Returns an array of languages available on the site
 * @return array
 */
function getLanguages()
{
  $framewerk = fMain::getInstance();
  $list = array();
  $temp = $framewerk->configuration->i18n->languages->available->option;
  foreach ( $temp AS $key => $value )
  {
    $list[] = $value;
  }
  return $list;
}

/**
 * Block Variable Handler
 *
 * Called by the Parser on {@block:...}.  Executes the Interface method that generates a Block
 * @return HTML representation of the block
 */
function callBlockInterface($array)
{

  $obj = new $array[0];
  if ( !method_exists($obj, "Block") )
  {
    trigger_error("Block executed for invalid interface " . $array[0], E_USER_WARNING);
    return "";
  }

  // Pass in a parameter if it exists

  $data = $obj->block($array);
  if ( $data )
  {
    $template = <<<TEMPLATE
      <div class="block">
       <h1>{blockTitle}</h1>
       {blockContent}
      </div>
TEMPLATE;

    //$template = fTheme::getInstance()->config->theme->blockTemplate;
    $output = str_replace('{blockContent}', $data, $template);
    $output = str_replace('{blockInterface}', $array[0], $output);
    if ( $obj->blockTitle )
    $output = str_replace('{blockTitle}', $obj->blockTitle, $output);
    return $output;
  }
  else return "";
}

/**
 * Internet Explorer check
 *
 * Returns 'msie' if MSIE is found in the user agent
 * @return string
 */
function returnMSIE()
{
  if ( strstr($_SERVER['HTTP_USER_AGENT'], "MSIE") ) return "msie";
  return;
}

/**
 * Variable incrementor
 *
 * Function that takes an array in 4 parts: 0: key, 1: max, 2: step, 3: initial val
 * increments the variable and returns the result
 * @param array
 * @return int
 */
function returnIncrementedValue($values)
{
  // Set our initial values and such
  $store = fExecutionStore::getInstance();
  $increment = $store->getValue('riv', $values[0]);
  if ( !$increment )
  {
    $increment = array();

    if ( !is_int($values[1]) ) $values[1] = 2;
    if ( !is_int($values[2]) ) $values[2] = 1;
    if ( !is_int($values[3]) ) $values[3] = 0;

    $increment['value']   = $values[3];
    $increment['initial'] = $values[3];
    $increment['step']    = $values[2];
    $increment['max']     = $values[1];
  }

  // Add our step value to the variable
  $increment['value'] = $increment['value'] + $increment['step'];

  // If we're greater than the max return it to the initial state plus 1
  if ( $increment['value'] > $increment['max'] )
  {
    $increment['value'] = $increment['initial'] + $increment['step'];
  }

  $store->setValue('riv', $values[0], $increment);

  // Return the value
  return $increment['value'];
}

/**
 * Block variable list
 *
 * Returns parser variables in order for blocks defined in Configuration/blocks.xml
 * @return string of blocks
 */
function returnBlockVariables()
{
  $xml = simplexml_load_file("Configuration/blocks.xml");
  $output = '';
  $array = array();
  foreach($xml->item AS $item)
  {
    $option = "";
    if ( !isset($item['disabled']) || $item['disabled'] == "false" || $item['disabled'] == "no" )
    {
      if ( isset($item['option']) ) $option = ":" . $item['option'];
      if ( !isset($item['sort']) ) $item['sort'] = 5; // XXX: default sort value?
      if ( !isset($item['permission']) || fSession::getInstance()->user->checkPermission($item['permission']) )
      $array[intval($item['sort'])][] = "{@block:$item$option}";
    }
  }
  // Sort by index (key)
  ksort($array);
  foreach($array as $sortlevel)
  {
    $output .= join("", $sortlevel);
  }
  return $output;
}

/**
 * Returns a formatted time
 * Syntax:
 *   ftime:<unix timestamp>[:<strftime format>]
 *
 * @param timestamp $timestamp
 * @return string
 */
function ftime($args = NULL)
{
  $timestamp = $args[0];
  $format = fMain::getInstance()->configuration->site->locale['ftime'];
  if ( !$format )
  {
    $format = '%x %X %Z';
  }
  if ( $timestamp == NULL || !is_numeric($timestamp) || $timestamp < 0 )
  {
    $timestamp = time();
  }
  if ( isset($args[1]) )
  {
    $format = $args[1];
  }
  return strftime($format, $timestamp);
}

/**
 * Returns the UNIX system load level
 *
 * This is for exiting out of Framewerk if the load level is too high
 *   * @return int Load Level
 */
function returnSystemLoad()
{
  // Get the system load via the UNIX uptime command
  $fp = popen("uptime", "r");
  $input = fgets($fp, 1024);
  pclose($fp);
  $data = explode(" ", $input);
  return Trim(substr($data[Count($data) - 3], 0, strlen($data[Count($data) - 3]) - 1));
}

/**
 * Returns the first string with a valid from an array
 *
 * An example of its use would be the variable {@firstString:{@memnu_name}:{@page_name}}
 * @param String $params
 * @return String
 */
function iff( $params )
{
  list ( $condition, $trueValue, $falseValue ) = $params;
  if ( strlen(trim($condition)) == 0 || $condition == 'Undefined' )
  {
    return $falseValue;
  }
  return $trueValue;
}

/**
 * Format bool based on type passed to it
 *
 * Returns bool formatted similar to type
 * @param bool
 * @param string
 * @return string
 */
function boolType($bool, $type)
{
  if(is_bool($type))
  {
    return $bool ? true : false;
  }

  if(is_string($type))
  {
    if(is_numeric($type))
    return $bool ? '1' : '0';
    return $bool ? 't' : 'f';
  }
  return $bool;
}

/**
 * Figure out if the passed value is one of a few true values
 *
 * Returns (bool)true on 'true' values, (bool)false on rest.
 * @param string
 * @param string
 * @return string
 */
function isTrue($string='', $type=false)
{
  switch($string)
  {
    case 1:
    case '1':
    case 't':
    case 'true':
      return boolType(true, $type);
    default:
      return boolType(false, $type);
  }
}

/**
 * Generate a unique ID prefixed with this "SysID"
 * @param string $SysID 2-character System ID
 * @return string 10-character unique ID
 * @static
 */
function generateID($SysID)
{
  if ( strlen($SysID) != 2 )
  {
    trigger_error("GenerateID(): SysID is not 2 characters", E_USER_WARNING);
    return "";
  }
  $rnd_id = crypt(uniqid(rand(),1));
  $rnd_id = strip_tags(stripslashes($rnd_id));
  $rnd_id = str_replace(array(".","/"),array("",""),$rnd_id);
  $rnd_id = strrev($rnd_id);
  $rnd_id = substr($rnd_id,0,8);
  $rnd_id = $SysID . $rnd_id;
  return $rnd_id;
}

function ismatch($params)
{
  if ( $params[0] == $params[1] )
  return $params[2];
}

function server($params)
{
  return $_SERVER[$params[0]];
}

function parser_rtrim($params)
{
  $out = rtrim($params[1], $params[0]);
  return $out;
}

/**
 * Used for quick debugging. Will dump any arguments passed into
 * fOutput and htmlentities'ize and <pre>'ize it
 *
 * @param mixed Variable to dump
 * @param bool Replace { and } with equivelant character entities
 */
function fecho( $var, $noparse = false )
{
  $output = fOutput::getInstance();

  // Start buffering and preparing for output
  ob_start();
  var_dump($var);
  $string = ob_get_clean();
  $string = htmlentities( $string );

  // Replace {}s so it doesn't get parsed by the parser
  if ( $noparse )
  {
    $string = str_replace('{', '&#123;', $string);
    $string = str_replace('}', '&#125;', $string);
  }

  // Append/prepend pre tags
  $string = '<pre class="fecho">' . $string . '</pre><br />';

  // Output
  $output->appendContent( $string );
}

function removeSpaces($variables)
{
  // Only return the first parameter
  return str_replace(' ', '_', $variables[0]);
}

function parseIncludeXML( &$xml )
{
  $xml = preg_replace_callback( '/<xi:include[^>]*>/m', 'pixcb', $xml );
}

function pixcb( $matches )
{
  preg_match( '/href=[^ ]*/', $matches[0], $hm );
  $href = substr( substr( $hm[0], 6 ), 0, -1 );
  list( $filePath, $xpointer ) = explode( '#', $href, 2 );
  $xml = file_get_contents( $filePath );
  return $xml;
}

function mybEncode( $input )
{
  return base64_encode( $input[0] );
}

/**
 * Finally, a single duration function!
 *
 * @param int $seconds The number of seconds to calculate for
 * @param array $format array( 1 => array( 'second', 'seconds' ), 60 => array( 'mySingularMinuteName', 'myPluralMinuteName' ) )
 * @param array $momentRule array( 'seconds' => 5, 'text' => 'about 5 seconds' ) 
 * This allows the developer to specify what the 'moment' rule is. If the $seconds passed in is <= 5 (in this example) the passed 'text' is used instead of calculating the time. 
 * @return string formatted output
 */
function duration( $seconds, $format = null, $unitSeparator = ' ', $momentRule = false )
{
  if ( ! $format )
  {
    $format = array( 31536000 => array( ' year', ' years' ),
                     604800 => array( ' week', ' weeks' ), 
                     86400 => array( ' day', ' days' ), 
                     3600 => array( ' hour', ' hours' ), 
                     60 => array( ' minute', ' minutes' ), 
                     1 => array( ' second', ' seconds' ), 
                   );
  } else {
    krsort( $format );  
  }

  if ( $seconds <= $momentRule['seconds'] || $seconds <= 0 )
  {
    if ( ! $momentRule['text'] )
    {
      return 'a moment';
    }
    return $momentRule['text'];
  }
  
  $duration = '';
  foreach ( $format as $multiple => $nouns )
  {
    $value = $seconds / $multiple;
    if ( $value >= 1 )
    {
      $duration .= $unitSeparator . floor( $value ) . $nouns[intval((floor($value)!=1))];
      $seconds = $seconds % $multiple;
    }
  }
  $duration = substr( $duration, strlen( $unitSeparator ) );
  return $duration;
}

/**
 * A function for breaking words into multiple lines
 * 
 * This function does not take anything like HTML tags into account
 *
 * @param string $input
 * @param int $maxWordWidth
 * @return string
 */
function wordBreak( $input, $maxWordWidth = 8 )
{
  $maxWordWidth = intval( $maxWordWidth );
  if ( strlen( $input ) > $maxWordWidth && $input )
  {
    $matches = array();
    preg_match_all( '/[^ ]{' . $maxWordWidth . ',}/', $input, $matches );
    if ( count( $matches > 0 ) )
    {
      foreach ( $matches as $match )
      {
        $match = $match[0];
        $matchLen = strlen( $match );
        for ( $i = 0; $i < $matchLen; $i += $maxWordWidth )
        {
          $adjusted .= substr( $match, $i, 8 ) . '<wbr />';
        }
        $adjusted = substr( $adjusted, 0, -7 );
        $input = str_replace( $match, $adjusted, $input );
      }
    }
  }
  return $input;
}

/**
 * Turn any number into it's word or words representation
 *
 * Copied from http://slightlymore.co.uk/cmwc/blog/index.php/archives/view/number-to-words-function-in-php
 * 
 * @author Dallas Gutauckis <dgutauckis@myyearbook.com>
 * 
 * @param int $number
 * @return string
 */
function numberToWords ($number)
{
  $words = array ('zero',
      'one',
      'two',
      'three',
      'four',
      'five',
      'six',
      'seven',
      'eight',
      'nine',
      'ten',
      'eleven',
      'twelve',
      'thirteen',
      'fourteen',
      'fifteen',
      'sixteen',
      'seventeen',
      'eighteen',
      'nineteen',
      'twenty',
      30=> 'thirty',
      40 => 'fourty',
      50 => 'fifty',
      60 => 'sixty',
      70 => 'seventy',
      80 => 'eighty',
      90 => 'ninety',
      100 => 'hundred',
      1000=> 'thousand');
 
  if (is_numeric ($number))
  {
    $number = (int) round($number);
    if ($number < 0)
    {
      $number = -$number;
      $number_in_words = 'minus ';
    }
    if ($number > 1000)
    {
      $number_in_words = $number_in_words . numberToWords(floor($number/1000)) . " " . $words[1000];
      $hundreds = $number % 1000;
      $tens = $hundreds % 100;
      if ($hundreds > 100)
      {
        $number_in_words = $number_in_words . ", " . numberToWords ($hundreds);
      }
      elseif ($tens)
      {
        $number_in_words = $number_in_words . " and " . numberToWords ($tens);
      }
    }
    elseif ($number > 100)
    {
      $number_in_words = $number_in_words . numberToWords(floor ($number / 100)) . " " . $words[100];
      $tens = $number % 100;
      if ($tens)
      {
        $number_in_words = $number_in_words . " and " . numberToWords ($tens);
      }
    }
    elseif ($number > 20)
    {
      $number_in_words = $number_in_words . " " . $words[10 * floor ($number/10)];
      $units = $number % 10;
      if ($units)
      {
        $number_in_words = $number_in_words . numberToWords ($units);
      }
    }
    else
    {
      $number_in_words = $number_in_words . " " . $words[$number];
    }
    return $number_in_words;
  }
  return false;
}

/**
 * Allows us to return server variables from the dictionary
 */
function getServerVariable($variable)
{
  return $_SERVER[$variable[0]];
}

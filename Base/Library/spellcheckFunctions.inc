<?php // $Id: spellcheckFunctions.inc 826 2007-03-28 23:43:57Z gmr $
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
 * Spellcheck Functions.
 *
 * Spell Checking Functions
 *
 * @author     Joe Hansche <madcoder@gamesurge.net>
 * @link       http://framewerk.org
 * @since      2005
 * @copyright  Copyright 2005-2007 the Framewerk Development Group
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @version    $Rev: 826 $
 * @package    Engine
 * @subpackage Library
 */

function spellcheck($string)
{
  $words = preg_split('/[\W]+?/',$string);
  $misspelled = $return = array();

  /*
  $config = pspell_config_create('en');
  pspell_config_personal($config, 'Utilities/custom.pws');
  $int = pspell_new_config($config);
  pspell_add_to_personal($int, "GameSurge");
  pspell_save_wordlist($int);
  */
  $int = pspell_new('en');

  foreach ($words as $value) {
    if (!pspell_check($int, $value)) {
      $misspelled[] = $value;
    }
  }
  foreach ($misspelled as $value) {
    $return[$value] = pspell_suggest($int, $value);
  }
  return $return;
}

function spellcheck_format($string, $words, $format = '<span class="spellingError">%s</span> [Suggestion(s): <span class="spellingSuggestions">%s</span>]')
{
  $i = 0;
  $replace = array();
  foreach($words as $word => $suggest)
  {
    $suggest = array_splice($suggest, 0, 3);
    $replace[++$i] = sprintf($format, $word, join(", ", $suggest));
    $string = preg_replace("/(\W*)$word(\W*)/", "\\1@word$i\\2", $string);
  }
  for( $i; $i>=0; $i--)
  {
    $string = str_replace("@word$i", $replace[$i], $string);
  }
  return $string;
}

function spellcheck_replace_suggestions($string, $words)
{
  $i = 0;
  $replace = array();
  foreach($words as $word=>$suggest)
  {
    $replace[++$i] = $suggest[0];
    $string = preg_replace("/(\W*)$word(\W*)/", "\\1@word$i\\2", $string);
  }
  for( $i; $i>=0; $i--)
  {
    $string = str_replace("@word$i", $replace[$i], $string);
  }
  return $string;
}

<?php // $Id: fi18nXMLLoader.inc 938 2007-09-10 21:12:37Z gmr $
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
 * Internationalization (i18n) Data Loader XML Implementation.
 *
 * Reads the current i18n dictionary from Configuration/i18n.xml
 *
 * @author     Gavin M. Roy <gavinmroy@gmail.com>
 * @link       http://framewerk.org
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2005-2007 the Framewerk Development Group
 * @version    $Rev: 938 $
 * @package    Engine
 * @subpackage Core
 * @uses       fi18nLoader
 */

class fi18nXMLLoader extends fi18nLoader
{
  /**
   * Loads Data into an array of key-language-condition holding the values
   *
   * @return array Array of i18n data
   */
  function loadData()
  {
    return false;
    fDebug::getInstance()->add("Loading XML Data", 4);

    // Load the data file
    $registry = new fRegistry('fi18n');
    $file = new fFile($registry->loader->file);
    $data = simplexml_load_string($file->read());
    if ( intval($data['version']) < 2.0 ) throw new Exception("Invalid i18n.xml file format version.");
    $dict = array();
    foreach ( $data->key as $key )
    {
      $y = array();
      $name = StrToLower($key['name']);
      foreach ( $key->value as $value )
      {
        $cnd = "none";
        foreach ( $value->attributes() AS $attribute=>$val )
        {
          switch ( StrToLower($attribute) )
          {
            case "language":
              $lang = StrToLower(strval($val));
              if ( !isset($y[$lang]) ) $y[$lang] = 0;
              break;
            case "condition":
              $cnd = strval($val);
              break;
            default:
              trigger_error("Invalid i18n value attribute \"$attribute\" for key \"$name.\"", E_USER_WARNING);
              break;
          }
          $trn = strval($value);
          $dict[$name][$lang][$y[$lang]]['Value'] = $trn;
          $dict[$name][$lang][$y[$lang]]['Condition'] = $cnd;
          $y[$lang]++;
        }
      }
    }
    return $dict;
  }

  function rehashKey()
  {
    return filemtime($file);
  }
  
}

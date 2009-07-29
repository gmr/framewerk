<?php // $Id: fi18n.inc 945 2007-12-20 21:10:24Z madcoder $
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
 * Internationalization (i18n) Engine
 *
 * This object is responsible maintaining the internal i18n information
 * and managing functions related to it.
 *
 * @author     Gavin M. Roy <gavinmroy@gmail.com>
 * @link       http://framewerk.org
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2005-2007 the Framewerk Development Group
 * @version    $Rev: 945 $
 * @package    Engine
 * @subpackage Core
 * @uses       fMainSingletonObject
 */

class fi18n extends fSingletonObject
{
  static private $instance = NULL;

  protected $fMain;
  protected $fDebug;
  protected $fProfile;
  protected $fRegistry;
  private   $loader;
  private   $i18nTable;
  private   $language;
  private   $defaultLanguage;

  /**
   * Constructor
   *
   * Sets internal variables and loads data
   */
  protected function __construct()
  {
    // Set our objects tht we use
    $this->fMain   = fMain::getInstance();
    $this->fDebug = fDebug::getInstance();
    $this->fProfile = fProfile::getInstance();

    $this->fRegistry = new fRegistry('fi18n');

    // Get the Default Language
    foreach ( $this->fRegistry->languages->language AS $language )
    {
      if ( $language->default == true )
      {
        $this->language = strToLower(strval($language->code));
        $this->defaultLanguage = $this->language;
        $this->fDebug->add('Default language set to: ' . $this->defaultLanguage, 5);
        break;
      }
    }

    // Load our loader class
    $class = "fi18n" . StrToUpper($this->fRegistry->loader->type) . "Loader";
    $this->fDebug->add('Setting i18n Loader to: ' . $class, 4);
    $this->fProfile->profile("i18nLoad");
    $loader = new $class();
    $this->i18nTable = $loader->loadData();
    fProfile::getInstance()->profile("i18nLoad", "i18n Data Loaded");
  }

  /**
   * Get the current or new instance
   *
   * @return Object instance of this object
   */
  static function getInstance()
  {
    if ( !isset(self::$instance) )
      self::$instance = new fi18n();
    return self::$instance;
  }

  /**
   * Set the current working language
   *
   * @param string $lang lanugage
   */
  function setLanguage($lang)
  {
    $this->language = $lang;
  }

  /**
   * Parse internationalization markup
   *
   * @param string $cskey the content key
   * @param string $values passed in values for conditional evaluation
   * @return string parsed output
   */
  function parse($cskey, $values = "none")
  {
    $output = "";
    $key = StrToLower($cskey);
    $this->fDebug->add("Parse: received \"$key\", \"$values\"", 6);

    if ( strstr($values, ",") )
    {
      $cond = explode(",", $values);
    } else {
      $cond[] = $values;
    }

    /** Parse for our sessions default language looking for values **/

    // If the key doesn't exist in our table, use the key as the output.
    if ( !isset($this->i18nTable[$key]) )
    {
      return $cskey;
    }

    // Check our selected language
    if ( isset($this->i18nTable[$key][$this->language]) )
    {
      $nodes = $this->i18nTable[$key][$this->language];
      for ( $y = 0; $y < Count($nodes); $y++ )
      {
        if ( $nodes[$y]['Condition'] == 'none' )
        {
          return $nodes[$y]['Value'];
        } elseif ( $this->evaluateCondition($nodes[$y]['Condition'], $cond))
        {
          return $nodes[$y]['Value'];
        }
      }
    }

    // Check the default language
    if ( isset($this->i18nTable[$key][$this->defaultLanguage]) )
    {
      $nodes = $this->i18nTable[$key][$this->defaultLanguage];
      for ( $y = 0; $y < Count($nodes); $y++ )
      {
        if ( $nodes[$y]['Condition'] == 'none' )
        {
          return $nodes[$y]['Value'];
        } elseif ( $this->evaluateCondition($nodes[$y]['Condition'], $cond))
        {
          return $nodes[$y]['Value'];
        }
      }
    }

    return $cskey;
  }

  function evaluateCondition($calc, $cond)
  {
    // Reverse order, so that, e.g., $11 doesn't get replaced with "$cond[1]1"
    for ( $z = Count($cond) - 1; $z >= 0; $z-- )
    {
      $str = "\$" . ( $z + 1);
      $calc = str_replace($str, $cond[$z], $calc);
    }
    $calc = str_replace("&amp;", "&", $calc);
    $calc = str_replace("&lt;", "<", $calc);
    $calc = str_replace("&gt;", ">", $calc);
    $r = eval("if ( $calc ) return true;");
    if ( $r == false ) $r = 0;
    $this->fDebug->add("Parse: evaluating conditional: ($calc) == $r");
    return $r;
  }
  
  /**
   * Get a navigation language list
   *
   * @return string
   */
  function getLanguageList()
  {
    // Create a new XML document for the language list
    $dom = new DOMDocument('1.0', 'UTF-8');
    $i18n = $dom->createElement('i18n');
    $dom->appendChild($i18n);
    $languages = $dom->createElement('languages');
    $languages->setAttribute('current', $this->language);
    $i18n->appendChild($languages);
    foreach ( $this->fRegistry->languages->language AS $language )
    {
      $node = $dom->createElement('language');
      $node->setAttribute('code', $language->code);
      $node->setAttribute('display', $language->name);
      $languages->appendChild($node);
    }

    // Return the XSL transformed language list
    $fTheme = fTheme::getInstance();
    $xslFile = $fTheme->xslFile($this);
    echo $dom->saveXML();
    return html_entity_decode($fTheme->transformXML($xslFile, $dom->saveXML()));
  }

  /**
   * Check to see if the requested language is valid
   *
   * @param string $languageIn
   * @return bool
   */
  function isValidLanguage($languageIn)
  {
    foreach ( $this->fRegistry->languages->language AS $language )
    {
      if ( ( $languageIn == $language->code ) || 
           ( $languageIn == $language->name ) )
        return true;
    }
    return false;
  }
}
?>

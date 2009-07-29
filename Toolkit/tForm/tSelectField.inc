<?php // $Id: fSelectField.inc 881 2007-06-14 01:51:06Z gmr $
############################################################################
#    Copyright (C) 2004-2007 by the Framewerk Development Group            #
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
 * Select Form Field Object
 *
 * This is a Select class for a form field
 * Usable options:
 *  - init_label: Label for the first item in the drop-down list.  Default: '-- Choose One --'
 *  - hide_first: If bool(true), will supress the first item (not recommended for 'required' items)
 *  - required: If enabled, the user must choose one of the items (an error occurs if the selected value is '')
 *
 * @since      2004-09-14
 * @author     Joe Hansche <madCoder@gamesurge.net>
 * @version    $Rev: 881 $
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2004-2007 the Framewerk Development Group
 * @uses       tFormField
 * @package    Toolkit
 * @subpackage Form
 */

class tSelectField extends tFormField
{
  private $items;

  /**
   * @param string $name
   * @param string $label
   * @param mixed $items
   * @param array $options
   * @return TSelectField
   */
  function __construct($name, $label, $items, $options = NULL)
  {
    parent::__construct($name, $label, NULL, $options);
    $this->type = "tSelectField";
    $this->items = $items;
    
    // Pre-fill the selected value from post data
    if ( isset( $_REQUEST[$name] ) && ! isset( $this->options['selected'] ) )
    {
      $this->options['selected'] = $_REQUEST[$name];
    }
    
    if ( !isset($this->options['init_label']) )
    {
      $this->options['init_label'] = "{!i18n:Choose One}";
    }
    if ( !isset($this->options['multiple']) )
    {
      $this->options['multiple'] = 'false';
    }
    if ( !isset($this->options['size']) )
    {
      $this->options['size'] = 1;
    }
    if ( $this->options['array_key'] == true )
    {
      $this->options['arrayKey'] = true;
      $this->options['array_key'] = null;
      fWarn::getInstance()->add('Deprecated fSelectField attribute: array_key used instead of arrayKey');
    }
  }

  /**
   * @return DOM node
   */
  function buildOutput()
  {
    $dom = new DOMDocument('1.0', 'UTF-8');
    $parent = parent::buildOutput();
    $parent = $dom->importNode($parent, true);
    if ( is_array($this->items) || is_object($this->items) )
    {
      if ( $this->items instanceOf PDOStatement )
      {
        $data = $this->items->FetchAll(PDO::FETCH_NUM);
        $this->items = $data;
        foreach( $this->items as $item )
        {
          if ( !isset($item[1]) ) $item[1] = $item[0];
          $temp = $parent->appendChild($dom->createElement("option"));
          $temp->setAttribute("label", $item[1]);
          $temp->appendChild($dom->createTextNode($item[0]));
          $temp->setAttribute("selected", intval($this->options['selected'] == $item[0]));
        }
      } else {
        foreach($this->items as $key => $item)
        {
          if  ( ( $this->options['arrayKey'] == true ) || ( is_object($this->items) ) )
          {
            if ( ( !$this->options['arrayKey']  ) && ( isset($this->options['valueAttribute']) ) )
            {
              $temp = $parent->appendChild($dom->createElement("option", $item[$this->options['valueAttribute']]));
              if ( strlen(trim($item[$this->options['valueAttribute']])) )
                $temp->setAttribute("selected", intval($this->options['selected'] == $item[$this->options['valueAttribute']]));
            } else {
              $temp = $parent->appendChild($dom->createElement("option", strval($key)));
              $temp->setAttribute("selected", intval($this->options['selected'] == $key));
            }
            $temp->setAttribute("label", $item);
          } else {
            if ( is_array($item) )
            {
              foreach($item AS $k => $v )
              {
                $temp = $parent->appendChild($dom->createElement("option", $v));
                $temp->setAttribute("label", $v);
                $temp->setAttribute("selected", intval($this->options['selected'] == $v));
              }
            } else {
              $temp = $parent->appendChild($dom->createElement("option"));
              $temp->setAttribute("label", $item);
              $temp->appendChild($dom->createTextNode($item));
              $temp->setAttribute("selected", intval($this->options['selected'] == $item));
            }
          }
        }
      }
    }

    return $parent;
  }
}
?>

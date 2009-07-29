<?php // $Id: tFormField.inc 881 2007-06-14 01:51:06Z gmr $
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
 * Abstract Form Field Object
 *
 * This is an abstract class for a form field
 *
 * @since      2004-09-01
 * @author     Joe Hansche <madCoder@gamesurge.net>
 * @author     Gavin M. Roy <gavinmroy@gmail.com>
 * @copyright  2004-2007 Framewerk Development Group
 * @version    $Rev: 881 $
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @see        tForm
 * @package    Toolkit
 * @subpackage Form
 */

abstract class tFormField
{
  protected $fDebug, $name, $label, $options, $regex, $type, $class;
  public $validationError;

  /**
   * Constructor
   *
   * @param string $name
   * @param string $label
   * @param string $value
   * @param string $options
   */
  function __construct($name, $label, $value = NULL, $options = NULL)
  {
    $tExecutionStore = tExecutionStore::getInstance();
    $this->fDebug = fDebug::getInstance();
    
    $names = $tExecutionStore->getValue('tForm', 'names');
    if ( is_array($names) && $name != '' )
    {
      if ( in_array($name, $names) )
      {
        throw new Exception('The field name "' . $name . '" already exists in the tForm object stack.  You must select another.');
      }
    }

    $this->fDebug->add('Creating new tFormField: ' . $name, 8);
    $this->label = $label;
    $this->name = $name;
    $this->options = $options;
    $this->type = 'Unextended';
    $this->validationError = $this->name . ' is invalid.';
    
    // Set our value if we pass it in, otherwise override it with POST/GET variable values
    $tFormMethod = $tExecutionStore->getValue('tForm', 'method');
    if ( !trim($value) )
    {
      if ( isset($_POST[$name]) )
        $value = $_POST[$name];
      if ( ( $tFormMethod == 'GET' ) && ( isset($_GET[$name]) ) )
      {
        $value = $_GET[$name];
      }
    }
    $this->value = $value;
  }

  /**
   * Return a domNode for this field
   *
   * @return domNode
   */
  function buildOutput()
  {
    $dom = new DOMDocument('1.0', 'UTF-8');
    $field = $dom->createElement('field');
    if ( !isset($this->class) )
    {
      $field->setAttribute('class', $this->type);
    } else {
      $field->setAttribute('class', $this->class);
    }
    $field->setAttribute('id', $this->name);
    $field->setAttribute('label', $this->label);
    $field->setAttribute('name', $this->name);
    $field->setAttribute('type', $this->type);
    if ( is_array($this->options) )
    {
      foreach ( $this->options AS $name=>$value )
      {
        $field->setAttribute($name, $value);
      }
    }
    $field->appendChild($dom->createCDATASection($this->value));
    return $field;
  }

  /**
   * Return the field name
   *
   * @return string
   */
  function getName()
  {
    return $this->name;
  }

  /**
   * Remove Framewerk variables if needed
   *
   * @param bool $cleanFramewerkTags
   */
  function sanitize($cleanFramewerkTags)
  {
    if ( $cleanFramewerkTags == true )
    {
      $this->value = preg_replace('/({(@|%|!i18n:)([^{}]*((?1)[^{}]*)*)})/', $this->value);
    }
  }

  /**
   * Set the value
   *
   * @param string Value
   */
  function setValue($value)
  {
    $this->value = $value;
  }

  /**
   * Validate the value against a regex if it's set
   *
   * @return bool
   */
  function validate()
  {
    if ( $this->options['required'] && $this->value == '' )
    {
      $this->validationError = $this->label . ' is required!';
      fWarn::getInstance()->add('Validation failed for ' . $this->name . ', field is required.');
      return false;
    }
    if ( isset($this->regex) )
    {
      $return = preg_match($this->regex, $this->value);
      if ( !$return )
        $this->fDebug->add('Could not validate "' . $this->value . '" against "' . $this->regex . '"', 1);
      return $return;
    }
    return true;
  }
}
?>

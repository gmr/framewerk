<?php // $Id: tDateField.inc 881 2007-06-14 01:51:06Z gmr $
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
 * Date Form Field Object
 *
 * This is a date class for a form field
 *
 * @since      2004-09-15
 * @author     Gavin Roy <gavinmroy@gmail.com>
 * @version    $Rev: 881 $
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2004-2007 the Framewerk Development Group
 * @uses       tFormField
 * @package    Toolkit
 * @subpackage Form
 */

class tDateField extends tFormField
{
  // This ofrmat is required, to change, the regex below needs to be changed
  private $format = 'yyyy-mm-dd';
  
  /**
   * @param string $name
   * @param string $label
   * @param mixed $value
   * @param array $options
   */
  function __construct($name, $label, $value = NULL, $options = NULL)
  {
    
    // Set the tFormField constructor
    parent::__construct($name, $label, $value, $options);
    
    // Set some default parameters
    // Set our value to our format if it's not set
    if ( ( !$value ) && ( !$this->value ) ) $this->value = $this->format;

    $this->type = "tDateField";
    $this->options['size'] = 10 + ($this->options['range'] ? 14 : 0);
    $this->options['maxlength'] = 10 + ($this->options['range'] ? 14 : 0);
    if ( $this->options['range'] )
    {
      $this->regex = '/^((\d{4})-(\d{2})-(\d{2})) to ((\d{4})-(\d{2})-(\d{2}))$/';
    } else {
      $this->regex = '/^(\d{4})-(\d{2})-(\d{2})$/';
    }
    $this->validationError = 'You have entered an invalid date format for ' . $label;

    // Add the required plugins
    $jQuery = fjQuery::getInstance();
    $jQuery->addPlugin('jquery.ui.datepicker');
   
   $opts = new stdClass();
   $opts->changeFirstDay = false;
   $opts->dateFormat = 'yy-mm-dd';
   $opts->mandatory = $this->options['required'] == true;
   $opts->rangeSelect = $this->options['range'] == true;
   $opts->rangeSeparator = ' to ';
   $opts->defaultDate = '+0';
   $opts->closeAtTop = false;
   $opts = json_encode($opts);
   // Add the jQuery init code
   $init = <<<INIT
$('input#$name').datepicker($opts).focus(function(){if ( $(this).val() == 'yyyy-mm-dd' ){ $(this).val(''); return; }}).blur(function(){if ( $(this).val() == '' ) { $(this).val('yyyy-mm-dd'); }});
INIT;
    $jQuery->addInit($init);
    
    // Add our CSS file
//    fTheme::getInstance()->addObjectCSSFile('date_input');
  }
 
}
?>

<?php // $Id: tPhoneField.inc 14357 2008-07-14 19:43:31Z gmr $
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
 * Phone Form Field Object
 *
 * This is a phone class for a form field
 *
 * @since      2008-07-07
 * @author     Joe Hansche <jhansche@myyearbook.com>
 * @version    $Rev: 14357 $
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2004-2007 the Framewerk Development Group
 * @uses       tFormField
 * @package    Toolkit
 * @subpackage Form
 */

class tPhoneField extends tFormField
{
  /**
   * @param string $name
   * @param string $label
   * @param mixed $value
   * @param array $options
   */
  function __construct($name, $label, $value = NULL, $options = array() )
  {
    if ( ! isset( $options['ext'] ) )
    {
      $options['ext'] = false;
    } else if ($options['ext'])
    {
      $options['class'] = 'tPhoneField tPhoneField_ext';
    }
    
    
    // Call the tFormField constructor
    parent::__construct($name, $label, $value, $options);
   
    $this->type = "tPhoneField";
    $this->validationError = 'You have entered an invalid phone number for ' . $label;
  }
  
  function buildOutput()
  {
    static $jqDone = 0;
    if ( $jqDone == 0 )
    {
      $jqDone = 1;
      $fjQuery = fjQuery::getInstance();
      $fjQuery->addPlugin('phoneInput');
      $fjQuery->addInit('$("input.tPhoneField").phoneInput();');
    }
    return parent::buildOutput();
  }
}
<?php // $Id: tRadioField.inc 881 2007-06-14 01:51:06Z gmr $
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
 * Radio Button Form Field Object
 *
 * This is a radio button class for a form field
 *
 * @since      09/14/2004
 * @author     Joe Hansche <madCoder@gamesurge.net>
 * @version    $Rev: 881 $
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2005-2007 the Framewerk Development Group
 * @uses       tFormField
 * @package    Toolkit
 * @subpackage Form
 */

class tRadioField extends tFormField
{

  /**
   * @param string $name
   * @param string $label
   * @param mixed $value
   * @param array $options
   * @return fRadioField
   */
  function __construct($name, $label, $value = 1, $options = NULL)
  {
    parent::__construct($name, $label, $value);
    $this->type = "tRadioField";
    if ( is_array($options) )
      $this->options = $options;

    // If it appears in the form results, that means it was selected when
    // submitted, so make it checked by default
    if ( isset($_POST[$name]) && strlen($_POST[$name]) > 0 && $_POST[$name] == $value )
      $this->options['checked'] = 1;
    else if ( isset($_GET[$name]) && strlen($_GET[$name]) > 0 && $_GET[$name] == $value )
      $this->options['checked'] = 1;
  }

  /**
     * @return string
     */
  function process()
  {
    $this->setAttribute("value", $this->value);
    $this->setAttribute("rlabel", 1);
    $this->node->appendChild($this->dom->createTextNode($this->label));
  }

}

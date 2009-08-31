<?php // $Id: fPasswordField.inc 873 2007-06-13 16:11:20Z gmr $
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
 * Password Form Field Object
 *
 * This is a password class for a form field
 *
 * @since      2004-09-12
 * @author     Gavin Roy <gavinmroy@gmail.com>
 * @version    $Rev: 873 $
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2004-2007 the Framewerk Development Group
 * @uses       tFormField
 * @package    Toolkit
 * @subpackage Form
 */

class tPasswordField extends tTextField
{
  /**
   * @param string $name
   * @param string $label
   * @param string $value
   * @param array $options
   * @return fPasswordField
   */
  function __construct($name, $label, $value = NULL, $options = NULL)
  {
    parent::__construct($name, $label, $value, $options);
    $this->type = "tPasswordField";
  }

  function validate()
  {
    eval("\$array = \$_" . $_SERVER['REQUEST_METHOD'] . ";");

    if ( isset($this->options['confirm']) && strlen($this->options['confirm']) > 0 )
    {
      if ( $array[$this->name] !== $array[$this->options['confirm']] )
      {
        $this->setError("Passwords must match");
        return "Passwords do not match";
      }
    }
    return true;
  }
}
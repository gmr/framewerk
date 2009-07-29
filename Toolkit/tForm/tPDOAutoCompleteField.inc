<?php // $Id: tPDOAutoCompleteField.inc 881 2007-06-14 01:51:06Z gmr $
############################################################################
#    Copyright (C) 2006-2007 by the Framewerk Development Group            #
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
 * Autocomplete field like Google Suggest type functionality tied to a PDO query object
 *
 * @since      2006-10-28
 * @author     Fred Ghosn <fredghosn@gmail.com>
 * @version    $Rev: 881 $
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2006-2007 the Framewerk Development Group
 * @uses       tFormField
 * @package    Toolkit
 * @subpackage Form
 */

class tPDOAutoCompleteField extends tFormField
{
  /**
   * @param string $name
   * @param string $label
   * @param string $value
   * @param array $options
   * @return fPDOAutoCompleteField
   */
  function __construct($name, $label, $value = NULL, $options = NULL)
  {
    $this->type = 'AutoComplete';
    if ( $value === NULL )
    {
      parent::__construct($name, $label);
    } else {
      parent::__construct($name, $label, $value);
    }
    if ( is_array($options) )
    {
      $this->options = $options;
    }
    $registry = new fRegistry('tPDOAutoCompleteXML');
    $table = md5($this->options['table'] . $registry->pdoObfuscator);
    $field = md5($this->options['field'] . $registry->pdoObfuscator);
    $jQuery = fjQuery::getInstance();
    $jQuery->addPlugin('iutil');
    $jQuery->addPlugin('iautocompleter');
    $jQuery->addPlugin('pdo');
    $jQuery->addInit('$("input#' . $name . '").PDOAutoCompleter({t: "' . $table . '", f: "' . $field . '"});');
  }
}
?>

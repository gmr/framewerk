<?php  // $Id: tAutoCompleteField.inc 888 2007-06-14 02:43:28Z gmr $
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
 *  Autocomplete field like Google Suggest type functionality
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

class tAutoCompleteField extends tFormField
{
  /**
   * @param string $name
   * @param string $label
   * @param string $value
   * @param array $options
   * @return fAutoCompleteField
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
    $registry = new fRegistry('tAutoCompleteXML');
    $table = md5($options['table'] . $registry->pdoObfuscator);
    $field = md5($options['field'] . $registry->pdoObfuscator);
    $jQuery = fjQuery::getInstance();
    $jQuery->addPlugin('iautocompleter');
    $init = <<<INIT
      \$('#$name').Autocomplete(
        {
          source: '/tAutoCompleteXML/?t=$table&f=$field',
          autofill: true,
          helperClass: 'autocompleter',
          selectClass: 'selectAutocompleter',
          minchars: 1
        }
      );
INIT;
    $jQuery->addInit($init);
    if ( is_array($options) )
    {
      $this->options = $options;
    }
  }
}
?>

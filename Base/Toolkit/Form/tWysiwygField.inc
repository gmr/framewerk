<?php // $Id: tWysiwygField.inc 873 2007-06-13 16:11:20Z gmr $
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
 * Wysiwyg Form Field Object
 *
 * This is a Wysiwyg class for a form field using FCKEditor.  One day we should abstract
 * this even more to make it editor agnostic
 *
 * @since      2006-07-18
 * @author     Gavin M. Roy <gavinmroy@gmail.com>
 * @version    $Rev: 873 $
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2006-2007 the Framewerk Development Group
 * @uses       tFormField
 * @package    Toolkit
 * @subpackage Form
 */

class tWysiwygField extends tFormField
{
  /**
   * @param string $name
   * @param string $label
   * @param string $value
   * @param array $options
   * @return TWysiwygField
   *
   * Available options:
   *  - rows
   *  - cols
   *  - style
   */
  function __construct($name, $label, $value = NULL, $options = NULL)
  {
    parent::__construct($name, $label, $value, $options);
    $this->type = "tWysiwygField";
    if ( !isset($this->options['editor']) )
    {
      try
      {
        $registry = new fRegistry('tWysiwygField');
        $this->options['editor'] = $registry->editor;
        unset($registry);
      } catch ( Exception $e )
      {
        unset($this->options['editor']);
      }
    }
    if ( !isset($this->options['theme']) )
    {
      $this->options['theme'] = 'default';
    }
    if ( !isset($this->options['rows']) )
    {
      $this->options['rows'] = 20;
    }
    if ( !isset($this->options['cols']) )
    {
      $this->options['cols'] = 80;
    }
    if ( ( $this->options['nolabel'] == true ) || ( $this->options['toplabel'] == true ) )
    {
      $this->options['colspan'] = 2;
    }
    switch ( $this->options['editor'] )
    {
      // Add the tinymce info to the tinymce management object
      case 'FCKeditor':
        $fckclass = $_SERVER['DOCUMENT_ROOT'].(fMain::getInstance()->configuration->site->relativePath).'/Base/JavaScript/FCKeditor/fckeditor.php';
        if ( is_file($fckclass) )
        {
          include_once($fckclass);
        } else {
          throw new Exception('Error accessing FCKeditor files.');
        }
        break;
      case 'tiny_mce':
        $fTinyMCE = tTinyMCE::getInstance();
        $fTinyMCE->addElement($this->name, $this->options['theme']);
        $fTinyMCE->buildHeader();
        break;
      default:
        throw new Exception('Unknown WYSIWYG Editor Type: ' . var_export($this->options['editor'],true) . '. Check XML/registry.xml //tWysiwygField/editor/.');
    }
  }

  /**
   * @return string
   */
  function buildOutput()
  {
    switch ( $this->options['editor'] )
    {
      // Add the tinymce info to the tinymce management object
      case 'FCKeditor':
        $editor = new FCKeditor($this->name) ;
        $editor->Value = $this->value;
        $editor->BasePath = strval(fMain::getInstance()->configuration->site->baseURI) . 'JavaScript/FCKeditor/';

        if ( isset($this->options['width']) )
          $editor->Width = $this->options['width'];
        if ( isset($this->options['height']) )
          $editor->Height = $this->options['height'];
        $this->value = $editor->CreateHtml();
        break;
    }
    return parent::buildOutput();
  }
}

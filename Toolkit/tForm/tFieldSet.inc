<?php // $Id: tFieldSet.inc 18484 2008-10-09 23:01:08Z jhansche $
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
 * Form Container class
 *
 * This class is responsible for handling tForm sub-objects
 *
 * @since      2006-10-27
 * @author     Gavin M. Roy <gavinmroy@gmail.com>
 * @version    $Rev: 18484 $
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2006-2007 the Framewerk Development Group
 * @uses       tFormField
 * @package    Toolkit
 * @subpackage Form
 */
class tFieldSet extends tFormField
{
  protected $class, $id, $legend, $objects;
  private $tExecutionStore;

  /**
   * Constructor
   *
   * @param String Legend for fieldset
   * @param String CSS class
   */
  function __construct( $legend = NULL, $class = NULL )
  {
    parent::__construct();
    $this->fDebug->add('Creating new tFieldSet: ' . $legend, 8);
    $this->tExecutionStore = tExecutionStore::getInstance();
    $this->objects = array();
    $this->class = $class;
    $this->legend = $legend;
  }

  /**
   * Add to our stack of objects
   *
   * @param object $object
   * @return object
   */
  function add( tFormField $object )
  {
    if ( is_object($object) )
    {
      $name = $object->getName();
      if ( !isset($this->objects[$name]) )
      {
        $this->objects[$name] = $object;
        return $object;
      }
      throw new Exception('Name: ' . $name . ' already exists in the tForm object stack.  Unable to add object to tForm stack: ' . $name);
    } else {
      $name = strval($object);
    }
    throw new Exception('Unable to add object to tForm stack: ' . $name);
  }

  /**
   * Return a dom node contain this and sub-objects
   *
   * @return domNode
   */
  function buildOutput()
  {
    $dom = new DOMDocument('1.0', 'UTF-8');
    $container = $dom->createElement('fieldset');
    $container->setAttribute('legend', $this->legend);
    $container->setAttribute('class', $this->class);
    $container->setAttribute('type', 'fieldset');
    foreach($this->objects AS $name=>$object )
    {
      $this->fDebug->add('Adding ' . $name . ' to the tFieldSet DOM object.', 8);
      $node = $dom->importNode($object->buildOutput(), true);
      $container->appendChild($node);
    }
    return $container;
  }

  /**
   * Return the class
   *
   * @return string
   */
  function getClass()
  {
    return $this->class;
  }

  /**
   * Return the id
   *
   * @return string
   */
  function getId()
  {
    return $this->id;
  }

  /**
   * Clean up our submitted data if we dont want framewerk variables entered in fields
   * Calls sub-object santize functions
   */
  function sanitize()
  {
    if ( $this->allowFramewerkVariables == false )
    {
      foreach ( $this->objects AS $name=>$object )
      {
        $this->fDebug->add('Sanitizing the input from: ' . $name, 8);
        $object->sanitize(true);
      }
    }
  }

  /**
   * Override our default class
   *
   * @param string $newClass
   */
  function setClass($newClass = 'tForm')
  {
    $this->class = $newClass;
  }

  /**
   * Set the id
   *
   * @param string $id
   */
  function setId($id)
  {
    $this->id = $id;
  }

  /**
   * Validate our form
   *
   * @return bool
   */
  function validate()
  {
    $validated = true;
    foreach ( $this->objects AS $name=>$object )
    {
      $this->fDebug->add('Validating the input from: ' . $name, 8);
      $validated = $object->validate();
      if ( !$validated )
      {
        $this->validationError = $object->validationError;
        break;
      }
    }
    return $validated;
  }
}
?>

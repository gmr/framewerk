<?php // $Id: tFormContainer.inc 881 2007-06-14 01:51:06Z gmr $
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
 * @version    $Rev: 881 $
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2006-2007 the Framewerk Development Group
 * @uses       tFormField
 * @package    Toolkit
 * @subpackage Form
 */
class tFormContainer extends tFormField
{
  protected $class, $id, $name, $type, $objects;
  private $tExecutionStore;

  /**
   * Constructor
   *
   * @param unknown_type $type
   * @param unknown_type $class
   */
  function __construct($type='div', $class = NULL)
  {
    parent::__construct();
    $tExecutionStore = tExecutionStore::getInstance();
    $this->fDebug = fDebug::getInstance();
    $name = generateID('TF');
    $names = $tExecutionStore->getValue('tForm', 'names');
    if ( is_array($names) )
    {
      if ( in_array($name, $names) )
      {
        throw new Exception('The field name "' . $name . '" already exists in the tForm object stack.  You must select another.');
      }
    }
    $this->fDebug->add('Creating new tFormContainer: ' . $name, 8);
    $this->tExecutionStore = tExecutionStore::getInstance();
    $this->objects = array();
    $this->class = $class;
    $this->name = $name;
    $this->type = $type;
  }

  /**
   * Add to our stack of objects
   *
   * @param object $object
   * @return object
   */
  function add($object)
  {
    if ( is_object($object) )
    {
      $name = $object->getName();
      if ( !isset($this->objects[$name]) )
      {
        $names = $this->tExecutionStore->getValue('tForm', 'names');
        if ( !is_array($names) ) $names = array();
        if ( !in_array($name, $names) )
        {
          $names[] = $name;
          $this->tExecutionStore->setValue('tForm', 'names', $names);
          $this->fDebug->add('Adding ' . $name . ' to the tForm object stack', 6);
          $this->objects[$name] = $object;
          return $object;
        }
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
    $container = $dom->createElement('container');
    $container->setAttribute('name', $this->name);
    $container->setAttribute('class', $this->class);
    $container->setAttribute('type', $this->type);
    $container->setAttribute('id', $this->id);
    foreach($this->objects AS $name=>$object )
    {
      $this->fDebug->add('Adding ' . $name . ' to the tFormContainer DOM object.', 8);
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
      if ( !$validated ) break;
    }
    return $validated;
  }
}
?>

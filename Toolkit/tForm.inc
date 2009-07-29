<?php // $Id: tForm.inc 941 2007-09-19 20:57:07Z madcoder $
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
  * Form Handling Object
  *
  * This object is responsible for handling Form generation
  *
  * @author     Joe Hansche <madCoder@GameSurge.net>
  * @author     Gavin M. Roy <gavinmroy@gmail.com>
  * @link       http://framewerk.org
  * @license    http://opensource.org/licenses/bsd-license.php BSD License
  * @copyright  Copyright 2005-2007 the Framewerk Development Group
  * @version    $Rev: 941 $
  * @package    Toolkit
  * @subpackage Form
  * @uses       fObject
  * @since      2005/09/10
  */

  class tForm extends fObject
  {
    private $action, $class, $enctype, $id, $method, $name, $dom, $objects, $showLabels, $fTheme, $tExecutionStore;
    public $defaultValues; // @todo Remove this, for legacy purposes
    protected $variables = false;

    /**
     * tForm Constructor
     * @param string $method
     * @param string $action
     * @param string $name
     * @param string $enctype
     * @return tForm
     */
    function __construct($method = 'post', $action = NULL, $name = 'tForm', $enctype = 'application/x-www-form-urlencoded')
    {
      parent::__construct();
      $this->fTheme = fTheme::getInstance();
      $this->tExecutionStore = tExecutionStore::getInstance();

      // Validate our options coming in
      $method = strToLower($method);
      if ( !in_array($method, array('post', 'get')) )
      {
        throw new Exception('Invalid form method passed to tForm constructor: ' . $method);
      }
      $enctype = strToLower($enctype);
      if ( !in_array($enctype, array('application/x-www-form-urlencoded','multipart/form-data')))
      {
        throw new Exception('Invalid form encoding type passed to tForm constructor: ' . $enctype);
      }

      // Set our action or the default of our current uri
      if ( $action != NULL )
      {
        $this->action = $action;
      } else {
        $this->action = "{@siteURI}";
      }

      // Set our other variables
      $this->class      = 'tForm';
      $this->dom        = new DOMDocument('1.0', 'UTF-8');
      $this->enctype    = $enctype;
      $this->method     = $method;
      $this->name       = $name;
      $this->showLabels = 1;

      $this->tExecutionStore->setValue('tForm', 'method', $this->method);
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
        if ( !isset($this->objects[$name]) || $name == '' )
        {
          $names = $this->tExecutionStore->getValue('tForm', 'names');
          if ( !is_array($names) ) $names = array();
          if ( !in_array($name, $names) || $name == '' )
          {
            if ( $name != '' )
              $names[] = $name;
            $this->tExecutionStore->setValue('tForm', 'names', $names);
            $this->fDebug->add('Adding ' . $name . ' to the tForm object stack', 6);
            if ( $name != '' )
              $this->objects[$name] = $object;
            else
              $this->objects[] = $object;
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
     * Override our default turned off framewerk vars or toggle it
     *
     * @param bool $bool
     */
    function allowFramewerkVariables($bool = true)
    {
      $this->variables = $bool;
    }

    /**
     * Return our XHTML output by building our DOM Document and transforming the XML with tForm.xsl
     *
     * @return string
     */
    function buildOutput()
    {
      $dom = new DOMDocument('1.0', 'UTF-8');
      $tForm = $dom->createElement('tForm');
      $dom->appendChild($tForm);
      $tForm->setAttribute('enctype', $this->enctype);
      $tForm->setAttribute('class', $this->class);
      $tForm->setAttribute('action', $this->action);
      $tForm->setAttribute('method', $this->method);
      $tForm->setAttribute('showLabels', $this->showLabels);
      $tForm->setAttribute('name', $this->name);
      $tForm->setAttribute('id', $this->name);
      foreach($this->objects AS $name=>$object )
      {
        $this->fDebug->add('Adding ' . $name . ' to the tForm DOM object.', 8);
        $node = $dom->importNode($object->buildOutput(), true);
        $tForm->appendChild($node);
      }
      $this->fTheme->addObjectCSSFile($this);
      if ( $this->fTheme->addObjectJSFile($this) )
      {
        $jQuery = fjQuery::getInstance();
        $jQuery->addInit('if ( "tFormInit" in window ) tFormInit();');
      }
      $this->fDebug->add($dom->saveXML(), 4);
      $this->fProfile->profile('tForm xslt');
      $xml = $this->fTheme->transformXML($this->fTheme->xslFile('tForm'), $dom->saveXML());
      $this->fProfile->profile('tForm xslt', 'tForm XSLT');
      return $xml;
    }

    /**
     * Return the form action
     *
     * @return string
     */
    function getAction()
    {
      return $this->action;
    }

    /**
     * Return the form class
     *
     * @return string
     */
    function getClass()
    {
      return $this->class;
    }

    /**
     * Return the form display label toggle
     *
     * @return name
     */
    function getDisplayLabels()
    {
      return $this->showLabels;
    }

    /**
     * Return the form enctype
     *
     * @return string
     */
    function getEnctype()
    {
      return $this->enctype;
    }

    /**
     * Return the form method
     *
     * @return string
     */
    function getMethod()
    {
      return $this->method;
    }

    /**
     * Return the form name
     *
     * @return name
     */
    function getName()
    {
      return $this->name;
    }

    /**
     * Remove a form object from the stack
     *
     * @param object $object
     */
    function remove($object)
    {
      if ( is_object($object) )
      {
        $name = $object->getName();
        if ( isset($this->objects[$name]) )
        {
          $this->fDebug->add('Removing ' . $name . ' from the tForm object stack', 6);
          $names = $this->tExecutionStore->getValue('tForm', 'names');
          $new = array();
          foreach ( $names AS $n)
          {
            if ( $name != $n ) $new[] = $n;
          }
          $this->tExecutionStore->setValue('tForm', 'names', $new);
          unset($this->objects[$name]);
          return true;
        }
      }
      $this->fDebug->add('Could not remove the requested object from the tForm stack', 6);
      return false;
    }

    /**
     * Remove all of the objects from the tForm stack
     */
    function removeAll()
    {
      $this->fDebug->add('tForm deleting all fields and values', 8);
      foreach ( $this->objects AS $name=>$object )
        $this->remove($object);
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
     * Override our default action
     *
     * @param string $newName
     */
    function setAction($newAction = NULL)
    {
      // Set our action or the default of our current uri
      if ( $newAction != NULL )
      {
        $this->action = $newAction;
      } else {
        $this->action = "{@siteURI}";
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
     * Override our default display labels
     *
     * @param string $newShowLabels
     */
    function setDisplayLabels($newShowLabels = true)
    {
      $this->showLabels = 0;
      if ( $newShowLabels == true )
        $this->showLabels = 1;
    }

    /**
     * Override our default enctype
     *
     * @param string $newEnctype
     */
    function setEnctype($newEnctype = 'application/x-www-form-urlencoded')
    {
      $newEnctype = StrToLower($newEnctype);
      if ( !in_array($newEnctype, array('application/x-www-form-urlencoded','multipart/form-data')))
      {
        throw new Exception('Invalid form encoding type passed to tForm constructor: ' . $enctype);
      }
      $this->enctype = $newEnctype;
    }

    /**
     * Override the method
     *
     * @param string $newMethod
     */
    function setMethod($newMethod = 'POST')
    {
      $newMethod = StrToUpper($newMethod);
      if ( !in_array($newMethod, array('POST', 'GET')) )
      {
        throw new Exception('Invalid form method passed to tForm constructor: ' . $method);
      }
      $this->method = $newMethod;
      $this->tExecutionStore->setValue('tForm', 'method', $this->method);
    }

    /**
     * Override our default name
     *
     * @param string $newName
     */
    function setName($newName = 'tForm')
    {
      $this->name = $newName;
    }

    /**
     * Clear out the set values for all of our child form objects
     * Useful after a POST
     */  
    function clearValues()
    {
      foreach ( $this->objects AS $name=>$object )
      {
        $this->fDebug->add('Clearing value for: ' . $name, 8);
        $object->setValue('');
      }
    }

    /**
     * Check to see if the form was submitted or if we're on the data entry side
     *
     * @return bool
     */
    function submitted()
    {
      $this->method = StrToLower($this->method);
      if ( StrToLower($_SERVER['REQUEST_METHOD']) == $this->method )
      {
        if ( $this->method == 'post' )
        {
          $this->fDebug->add('Form appears to be submitted.', 4);
          return true;
        }
        $vars = $_GET;
        foreach ( $this->objects AS $name=>$object )
        {
          if ( isset($vars[$name]) )
          {
            $this->fDebug->add('Form appears to be submitted.', 4);
            return true;
          }
        }
      }
      $this->fDebug->add('Form Not Submitted', 4);
      return false;
    }

    /**
     * Validate our form
     *
     * @return bool
     */
    function validate()
    {
      $messages = fMessages::getInstance();
      $validated = true;
      foreach ( $this->objects AS $name=>$object )
      {
        $this->fDebug->add('Validating the input from: ' . $name, 8);
        $result = $object->validate();
        if ( $result != true )
        {
          $validated = false;
          $messages->add(new fErrorMessage($object->validationError));
        }
      }
      $this->fDebug->add('Form validation results:' . $validated, 8);
      return $validated;
    }

    /**
     * Aliases to allow older apps to work
     * @todo remove once core apps are updated
     */
    function warn($type) { $this->fWarn->add('You are using deprecated tForm behavior: ' . $type); }
    function addField($field) { $this->warn('addField'); $this->add($field); }
    function makeForm() { $this->warn('makeForm'); return $this->buildOutput(); }
  }
?>

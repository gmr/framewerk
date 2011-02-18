<?php // $Id:fRegistry.inc 876 2007-06-13 16:22:22Z gmr $
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
 * Registry Management Object
 *
 * This object is to allow for module programmers to easily store and retrieve module specific settings.
 *
 * @author     Gavin M. Roy <gavinmroy@gmail.com>
 * @link       http://framewerk.org
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2005-2007 the Framewerk Development Group
 * @version    $Rev:876 $
 * @package    Engine
 */

class fRegistry
{
  private $name, $node;

  /**
   * Constructor
   *
   * @param unknown_type $node
   */

  function __construct($node = null)
  {
    // Make sure $node isn't null
    if ( is_null($node) )
    {
      throw new fHTTPException(501, 'You must specify an application when using the fRegistry class.');
    }

    // If the $node passed in is
    if ( is_object($node) )
    {
      $node = get_class($node);
    }

    // Set the class name for later
    $this->name = $node;

    // Load the XML document
    $file = new fFile('Configuration/registry.xml');
    $xml = simplexml_load_string($file->read());

    // Make sure it is an object and if not, throw an exception
    if ( !is_object($xml) )
    {
      throw new fHTTPException(501, 'registry.xml could not be loaded.');
    }

    // Check the version of the file and make sure it matches the new registry/node format
    if ( intval($xml['version'] != 3 ) )
    {
      throw new fHTTPException(501, 'Invalid registry.xml version: ' . $this->registry['version']);
    }

    // Get our base node for the application
    $baseNode = $xml->xpath('/registry/node[@name="' . $node . '"]');

    if ( count($baseNode) == 0 )
    {
      throw new Exception('Looking for a registry node ('.$node.') that does not exist in registry.xml!');
    }

    // Give ourselves a handle to our current fRegistryNode
    $this->node = new fRegistryNode($baseNode[0]);
  }

  /**
   * Destructor
   *
   * When we're destroying the object, save out any updates if the document has changed at all
   */
  function __destruct()
  {
    if ( $this->node->isChanged() )
    {
      // Build our DOMDOcument and load the contents of the registry file
      $dom = new DOMDocument('1.0', 'UTF-8');
      $dom->formatOutput = true;
      $file = new fFile('Configuration/registry.xml');
      $dom->loadXML($file->read());

      // Get our registry node
      $registry = $dom->getElementsByTagName('registry')->item(0);

      // Build the new tree for the class that called us
      $tree = $this->node->buildDOMTree($dom);

      // Find the old node
      $xpath = new DOMXPath($dom);
      $entries = $xpath->query('/registry/node[@name="' . $this->name . '"]');

      // Replace the old node with the new one
      $registry->replaceChild($tree, $entries->item(0));

      // Try and save the document
      if ( !$dom->save('Configuration/registry.xml')  )
      {
        // Add an engine warning
        $this->fWarn->add('fRegistry could not save the updated registry.xml file.');
      }
    }
  }

  /**
   * Get a fRegistryNode
   *
   * @param string $key
   * @return fRegistryNode
   */
  function __get($key)
  {
    return $this->node->$key;
  }

  /**
   * Set a fRegistryNode value
   *
   * @param string $key
   * @param any $value
   */
  function __set($key, $value)
  {
    $this->node->$key = $value;
  }

  /**
   * Check to see if a node is set
   *
   * @param string $key
   * @return bool
   */
  function __isset($key)
  {
    return isset($this->node->$key);
  }

  /**
   * Unset a fRegistryNode
   *
   * @param string $key
   */
  function __unset($key)
  {
    unset($this->node->$key);
  }

}

/**
 * fRegistryNode class for containing data for the entiry registry node tree
 * @todo revisit the casting code, it's not working 100% 2007-08-07 gmr
 */
class fRegistryNode implements Iterator
{
  public $_attr = array(), $_changed = false, $_value, $_values = array();
  public $_iter_ptr = 0;

  /**
   * Object constructor
   *
   * @param simplexml_element $node
   */
  function __construct(simplexml_node $node)
  {

    if ( !is_object($node) )
      throw new Exception('Empty node passed to fRegistryNode');

    // Loop through and process the child nodes
    foreach ( $node->children() AS $cname => $child )
    {
      // If we've already set a node then it needs to be an array
      if ( isset($this->_values[$cname]) )
      {
        // If it's not already an array, make it one
        if ( !is_array($this->_values[$cname]) )
        {
          $this->_values[$cname] = array($this->_values[$cname]);
        }
        // Assign a new child node to the array
        $this->_values[$cname][] = new fRegistryNode($child);
      } else {
        // Assign a new child node
        $this->_values[$cname] = new fRegistryNode($child);
      }
    }

    // Set the attributes for this item
    foreach ( $node->attributes() AS $attribute=>$value )
    {
      $this->_attr[$attribute] = strval($value[0]);
    }

    // Force our value to the specified data type, default to string
    switch(  $node['type'] )
    {
      case 'bool':
        $this->_attr['type'] = 'bool';
        $this->_value = ( intval($node) == 1 );
        break;
      case 'int':
        $this->_attr['type'] = 'int';
        $this->_value = intval($node);
        break;
      case 'constant':
        $this->_attr['type'] = 'constant';
        $this->_value = constant($node);
        break;
      case 'string':
      default:
        if ( strlen(trim($node)) )
        {
          $this->_attr['type'] = 'string';
          $this->_value = strval($node);
        }
        break;
    }

    // Destroy the values array if it's not used
    if ( Count($this->_values) == 0 ) unset($this->_values);
  }

  /**
   * Get the values or attributes of the node, or child nodes
   *
   * @param string $key
   * @return any
   */
  public function __get($key)
  {
    // If our values entry for this node is an array, build a value array of output
    if ( !isset($this->_values[$key]) ) return false;
    if ( is_array($this->_values[$key]) && sizeof($this->_values[$key]) > 0 )
    {
      $out = array();
      foreach ( $this->_values[$key] AS $value )
      {
        if ( strlen(trim($value->_value)) || get_class($value->_value) == 'fRegistryNode' )
        {
          $out[] = $value->_value;
        }
      }

      // If we actually have items in our array, return it
      if ( Count($out) > 0 ) return $out;
    }

    if ( $this->_values[$key] instanceOf fRegistryNode && is_array($this->_values[$key]->_values) && $this->_values[$key]->_attr['type'] == 'collection' )
    {
      return array($this->_values[$key]);
    }

    // Return the value if it's set
    //XXX: can't use strlen() here in case _value is bool(false), e.g.
    if ( isset($this->_values[$key]->_value) )
    {
      return $this->_values[$key]->_value;
    }

    // If we have an attribute named what we requested, return that
    if ( strlen(trim($this->_attr[$key])) > 0 )
    {
      return $this->_attr[$key];
    }

    // Return the node for recursion
    return $this->_values[$key];
  }

  /**
   * Set a value for this node
   *
   * @param string $key
   * @param any $value
   */
  public function __set($key, $value)
  {
    // Explicitly set the types if the type attribute is set
    switch ( $this->_values[$key]->_attr['type'] )
    {
      case 'bool':
        if ( $value )
        {
          $this->_values[$key]->_value = true;
        } else {
          $this->_values[$key]->_value = 0;
        }
        break;
      case 'int':
        $this->_values[$key]->_value = intval($value);
        break;
      default:
        $this->_values[$key]->_value = $value;
        break;
    }
    $this->_values[$key]->_changed = true;
  }

  /**
   * Check to see if a child node or value is set
   *
   * @param string $key
   * @return bool
   */
  public function __isset($key)
  {
    if ( isset($this->_values[$key]) || isset($this->_value) )
      return true;
    return false;
  }

  /**
   * Unset a child node or value
   *
   * @param string $key
   */
  public function __unset($key)
  {
    if ( isset($this->_value) )
      unset($this->_value);
    if ( isset($this->_values[$key]) )
      unset($this->_values[$key]);
  }

  /**
   * Recursively check this and all child nodes to see if a value has changed
   *
   * @return bool
   */
  function isChanged()
  {
    // If this node has changed no use going deeper, return true
    if ( $this->_changed ) return true;

    // Loop through the children and call this function
    if ( is_array($this->_values) )
    {
      foreach ( $this->_values AS $value )
      {
        // If it's an array of of nodes
        if ( is_array($value) )
        {
          foreach ( $value AS $arrayValue )
            if ( $arrayValue->isChanged() ) return true;
        } else {
          // If it's just a single node
          if ( $value->isChanged() ) return true;
        }
      }
    }

    // Nothing triggered a true, so return a fale
    return false;
  }

  /**
   * Build a DOM tree from this node down
   *
   * @param DOMDocument $dom
   * @param DOMElement $element
   * @return DOMElement
   */
  function buildDOMTree($dom, $element = NULL)
  {
    // If we didn't pass in an element, make a top level node element
    if ( !isset($element) )
      $element = $dom->createElement('node');

    // Loop through and see the attributes
    foreach ( $this->_attr AS $name=>$value )
      $element->setAttribute($name, $value);

    // If our value is set, create a CDATA or text section
    if ( isset($this->_value) )
    {
      // If we have elements that will break XML use CDATA
      if ( preg_match('/[<>&]/m', $this->_value) )
      {
        $data = $dom->createCDATASection($this->_value);
        $element->appendChild($data);
      } else {
        // Otherwise just use a normal text node
        $data = $dom->createTextNode($this->_value);
        $element->appendChild($data);
      }
    }

    // Loop through the child nodes and recursively build them
    if ( Count($this->_values) )
    {
      foreach ( $this->_values AS $name=>$node )
      {
        // Is it a multi-node array?
        if ( is_array($node) )
        {
          // Loop through the array nodes and build the tree
          foreach ( $node AS $anode )
          {
            $subNode = $dom->createElement($name);
            $anode->buildDOMTree($dom, $subNode);
            $element->appendChild($subNode);
          }
        } else {
          // Build the tree under this node
          $subNode = $dom->createElement($name);
          $node->buildDOMTree($dom, $subNode);
          $element->appendChild($subNode);
        }
      }
    }

    // Return this node
    return $element;
  }

  function next()
  {
    $this->_iter_ptr++;
  }

  function rewind()
  {
    $this->_iter_ptr = 0;
  }

  function key()
  {
    $temp = array_keys($this->_values);
    return $temp[$this->_iter_ptr];
  }

  function current()
  {
    return $this->__get($this->key());
  }

  function valid()
  {
    $temp = array_keys($this->_values);
    return isset($temp[$this->_iter_ptr]);
  }

}

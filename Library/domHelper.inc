<?php
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
 * Dom Functions
 *
 * These functions are designed to make working with the DOM easier
 *
 * @author     Gavin M. Roy <gavinmroy@gmail.com>
 * @link       http://framewerk.org
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @since      2005-08-11
 * @copyright  Copyright 2005-2007 the Framewerk Development Group
 * @version    $Rev: 826 $
 * @package    Engine
 * @subpackage Library
 */

/**
 * appendDomNode
 *
 * Creates new Dom node and sets its values or appends data values on a dom node
 *
 * @param object  Dom Document
 * @param object/string Dom Node or new Dom node name
 * @param array Attributes array of name/value array pairs
 * @param object Dom node to append as a child node of this node
 * @param text Text to set as a node value
 * @param text Comment for the node
 * @return object Dom Node
 */
function appendDomNode($document, $node, $attribs = NULL, $child = NULL, $text = NULL, $comment = NULL)
{
  // If the $node is a string and not a dom object, make a new element
  if ( ( !is_object($node) ) && ( is_string($node) ) )
  $node = $document->createElement($node);

  // If there are attributes append them
  if ( is_array($attribs) )
  foreach($attribs as $name=>$aval)
  {
    $attrib = $document->createAttribute($name);
    $value = $document->CreateTextNode($aval);
    $attrib->appendChild($value);
    $node->appendChild($attrib);
    unset($attrib);
  }

  // If we have a child node, append it
  if ( is_object($child) )
  $node->appendChild($child);

  // If we have a text value to add to this node, do so
  if ( !is_null($text) )
  {
    // If we have chars not allowed by xml, add the text as a cdata node otherwise as a text node
    if ( preg_match("/[<>&]/", $text) )
    $value = $document->createCDATASection($text);
    else
    $value = $document->CreateTextNode($text);
    $node->appendChild($value);
  }

  // If we have a comment add it
  if ( !is_null($comment) )
  {
    $value = $document->createComment($comment);
    $node->appendChild($value);
  }

  // Return our node object
  return $node;
}

/**
 * pdo2dom
 *
 * Returns DOM document from PDO data object retreived from PDOStatement::fetchAll(PDO::FETCH_OBJ)
 *
 * @param Object PDO Data Result
 * @return Object Dom Document
 */
function pdo2dom($dataObject, $root = 'pdoResult', $returnDoc = true)
{
  $document = new DOMDocument('1.0', 'UTF-8');
  $pdoresult = $document->createElement($root);
  foreach ( $dataObject AS $row )
  {
    $domRow = appendDomNode($document, 'row');
    foreach ( $row AS $key=>$value )
    {
      $domRow = appendDomNode($document, $domRow, NULL, appendDomNode($document, $key, NULL, NULL, $value));
    }
    $pdoresult = appendDomNode($document, $pdoresult, NULL, $domRow);
  }
  if ( $returnDoc == true )
  {
    $document->appendChild($pdoresult);
    return $document;
  }
  return $pdoresult;
}

/**
 * Take a key/value array and turn it into an XML document
 *
 * @param DOMDocument $dom
 * @param DOMNode $parent
 * @param Array $array
 */
function array2dom($dom, $parent, $array)
{
  foreach ( $array AS $key=>$value )
  {
    $element = $dom->createElement($key);
    $parent->appendChild($element);
    if ( is_array($value) )
    {
      array2dom($dom, $element, $value);
    } else {
      if ( strstr($value, '<?xml') )
      {
        $newdoc = DOMDocument::loadXML($value);
        $element->appendChild($dom->importNode($newdoc->firstChild, true));
      } else {
        $element->appendChild($dom->createTextNode($value));
      }
    }
  }
}
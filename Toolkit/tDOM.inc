<?php // $Id: tDOM.inc 914 2007-08-06 17:45:17Z gmr $
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
 * DOMDocument helper class
 *
 * Defines functions for generating DOM Documents
 *
 * @author     Brad Brizendine <briz@glyphix.com>
 * @link       http://www.glyphix.com/
 * @license    BSD
 * @since      2006-01-10
 * @copyright  Copyright 2006-2007 Framewerk Development Group
 * @version    $Rev: 914 $
 * @package    Toolkit
 * @subpackage    XML
 */

class tDOM extends fObject
{

  // xml version
  private $version = '1.0';
  // xml encoding
  private $encoding = 'UTF-8', $lastNode;
  // the DOM document: make it public for use outside this class
  public $doc;

  // data type constants
  const LAST = 'ATTACH_TO_LAST_NODE';
  const END = 'ATTACH_TO_END';

  /** Constructor
   * Instantiating this object requires at least a Root value.
   * It creates a DOM Document and generates a root node.
   * If root appears to be a filepath, it loads that file.
   * If root is not a path and starts with < ? xml, it attempts to load the string as xml.
   * If root is not a path and starts with < / html >, it attempts to load the string as html.
   * It returns itself when done.
   *
   * @param String node name
   * @param String text value for node
   * @param Array key=>value list
   * @param String xml encoding, defaults to UTF-8
   * @param String xml version, defaults to 1.0
   * @returns object TDOM
   */
  public function __construct( $root = null, $textValue = null, $attributes = null, $encoding = 'UTF-8', $version = '1.0' )
  {
    parent::__construct();

    // root is required
    if( !$root )
    {
      $this->fWarn->add(get_class($this) .' Missing root or not a string: must be either a path or a node name');
      return false;
    }

    switch(true){

      // if it's an object...
      case is_object($root):
        $this->fDebug->add(get_class($this) .' Loading an object: ' .$root, 6);
        $this->newDoc();
        if( !$this->loadObject($root) )
        {
          $this->fWarn->add(get_class($this) .' Unable to load object');
          return false;
        }
        break;

      // if it's a path ... unix or windows ... or if the provided string points to a file
      case substr($root,0,1) == DIRECTORY_SEPARATOR || preg_match('/^[a-zA-Z]{1}:/',$root) || is_file($root):
        $this->fDebug->add(get_class($this) .' Loading a file: ' .$root, 6);
        $this->newDoc();
        // Suppress warnings and errors...
        if( !@$this->doc->load($root) )
        {
          $this->fDebug->add(get_class($this) .' Unable to load file: ' .$root, 6);
          return false;
        }
        break;

      // if it's xml ... use is_numeric() because strpos can return a zero
      case is_numeric(strpos($root,'<?xml')):
        $this->newDoc();
        // Suppress warnings and errors...
        if( !@$this->doc->loadXML($root) )
        {
          $this->fWarn->add(get_class($this) .' Unable to load xml string', 6);
          return false;
        }
        break;

      // if it's html ... use is_numeric() because strpos can return a zero
      case is_numeric(strpos($root,'</html>')) || is_numeric(strpos($root,'</HTML>')):
        $this->newDoc();
        // Suppress warnings and errors...
        if( !@$this->doc->loadHTML($root) )
        {
          $this->fWarn->add(get_class($this) .' Unable to load html string', 6);
          return false;
        }
        break;

      // otherwise, just create a document
      default:
        $this->fDebug->add(get_class($this) .' Creating a new document called ' .$root, 6);
        $this->newDoc();
        // can't use the append() function here since it depends on having a root node
        $this->doc->appendChild( $this->createNode( $root, $textValue, $attributes ) );
    }

    // Set encoding and version
    $this->setEncoding( $encoding );
    $this->setVersion( $version );

    return $this;
  }

  /**
   * loadObject
   *
   * Turns a PHP native object into xml
   *
   * @param Object the object to convert to xml
   * @param Object the node to attach to ... this is used by the function itself, do not pass anything for this value
   * @param String the name of the top-level element
   * @return Object the final xml DOM document
   */
  private function loadObject( $obj = null, $node = null, $name = 'root' )
  {
    if( !$obj )
    {
      $this->fDebug->add(get_class($this) .' Missing object in importObject()', 6);
      return false;
    }

    //
    if( !is_object($node) )
    {
      // can't use the append() function here since it depends on having a root node
      $node = $this->doc->appendChild( $this->createNode($name) );
    }

    // grab all object vars
    if( is_object($obj) ){
      $props = get_object_vars($obj);
    }
    if( is_array($obj) ){
      $props = $obj;
    }
    // if we have no props, return nothing
    if( !is_array($props) && !count($props) ){
      return false;
    }

    // run through all properties
    foreach( $props as $k => $v )
    {
      // don't want numeric element names
      if( is_numeric($k) )
      {
        $k = 'item';
      }
      // use 1's and 0's for boolean
      if( gettype($v) == 'boolean' )
      {
        // if v is zero, turn it into a string
        if( !$v )
        {
          $v = 0 .' ';
        }
      }
      // if it's not a string, recurse
      if( is_object($v) || is_array($v) )
      {
        // create the holder node and send it the results of recursing
        $this->loadObject( $v, $this->append( $node, $k ) );
      }else{
        $this->append( $node, $k, $v );
      }
    }

    return $node;
  }

  /**
   * setEncoding
   *
   * Sets xml encoding for this document.
   *
   * @param String encoding
   * @return boolean
   */
  public function setEncoding( $encoding = 'UTF-8' )
  {
    $this->encoding = $encoding;

    // If we already have a doc, set it
    if( $this->doc )
      $this->doc->encoding = $this->encoding;

    return true;
  }

  /**
   * setVersion
   *
   * Sets xml version for this document.
   *
   * @param String version
   * @return boolean
   */
  public function setVersion( $version = '1.0' )
  {
    $this->version = $version;

    // If we already have a doc, set it
    if( $this->doc )
      $this->doc->version = $this->version;

    return true;
  }

  /**
   * newDoc
   *
   * Creates a new DOMDocument with appropriate version and encoding,
   * then sets several DOM specific attributes.
   *
   * @return object
   */
  private function newDoc()
  {
    $this->doc = new DOMDocument( $this->version, $this->encoding );
    $this->doc->preserveWhiteSpace  = false;
    $this->doc->formatOutput        = true;
    $this->doc->standalone          = true;
    $this->doc->XMLStandalone       = true;
    $this->doc->substituteEntities  = true;
    return $this->doc;
  }

  /**
   * xml
   *
   * Generates xml string
   *
   * @return string
   */
  public function xml()
  {
    return $this->doc->saveXML();
  }

  /**
   * append
   *
   * Adds node(s) to the DOMDocument based on the type of the information passed in.
   *
   * @param Mixed location in the current doc to attach
   * @param Mixed information to be included
   * @param String if item is a string, this is the value it will be set with
   * @param Array key=>value list
   * @return boolean
   */
  public function append( $where = self::END, $item = null, $textValue = null, $attributes = null )
  {
    // Must have something in item
    if( !$item )
    {
      $this->fWarn->add(get_class($this) .' Missing item: must be a string or object');
      return false;
    }

    // Set up appendTo
    switch(true)
    {
      // where is an object, so just use it
      case is_object($where):
        $appendTo = $where;
        break;

      // Append to the latest node
      case $where === self::LAST:
        if( is_object($this->lastNode) ){
          $appendTo = $this->lastNode;
        } else {
          $appendTo = $this->doc->documentElement;
        }
        break;

      // Append to end of this doc.
      case $where === self::END:
        $appendTo = $this->doc->documentElement;
        break;

      // It's an xpath, find the node and use it
      case gettype($where) == 'string':
        if( $tmp = $this->get($where,null,'dom') ){
          $appendTo = $tmp->item(0);
        }else{
          $appendto = $this->doc->documentElement;
        }
        break;

      // If nothing else, append to end of doc root
      default:
        $appendTo = $this->doc->documentElement;
    }

    // Determine what to do with item
    switch( gettype($item) )
    {

      // It's a simple string, add it as a node.
      case 'string':
        $result = $this->appendNode( $item, $textValue, $attributes, $appendTo );
        break;

      // It's an array, add it recursively
      case 'array':
        // If it's a numerically-indexed array of objects,
        // treat it as if it's the result of a SimpleXML xpath.
        if( is_array($item) && is_object($item[0]) )
        {
          for( $i = 0; $i < count($item); $i++ )
          {
            $this->appendElement( $item[$i], $appendTo );
          }
          $result = $appendTo;
        } else {
          $result = $this->appendArray( $item, $textValue, $attributes, $appendTo );
        }
        break;

      case 'object':

        switch(get_class($item))
        {
        // If it's the results of a DOM xpath, add each item as an element.
        // Otherwise, treat it as a full DOM document.
          case 'DOMNodeList':
            foreach( $item as $tmp )
            {
              $this->appendElement( $tmp, $appendTo );
            }
            $result = $appendTo;
            break;

          case 'SimpleXMLElement':
            $result = $this->appendElement( $item, $appendTo );
            break;

          default:
            $result = $this->appendDoc( $item, $appendTo);
        }
        break;

      default:
        $this->fDebug->add(get_class($this) .' Item type of ' .gettype($item) .' not supported', 6);
        return false;
    }

    // record lastNode, and return result
    if( is_object($result) )
    {
      $this->lastNode = $result;
    }
    return $result;
  }

  /**
   * appendNode
   *
   * Adds a single node to the current document.
   *
   * @param String name of the node
   * @param String text value of the new node
   * @param Array key=>value list
   * @param Object the node to which to attach the node
   * @return boolean
   */
  private function appendNode( $nodeName = null, $textValue = null, $attributes = null, $appendTo = null )
  {
    // Create the new node
    $node = $this->createNode( $nodeName, $textValue, $attributes );
    // If no appendTo, append to the end of the current doc
    if( !is_object($appendTo) )
    {
      $appendTo = $this->doc->documentElement;
    }
    $appendTo->appendChild( $node );
    return $node;
  }

  /**
   * appendArray
   *
   * Recursively adds an array to the current document.
   *
   * @param Array array to add
   * @param String node name to use for numerically-indexed arrays
   * @param Array key=>value list
   * @param Object the node to which to attach the node
   * @return boolean
   */
  private function appendArray( $array = null, $nodeName = 'entry', $attributes = null, $appendTo = null )
  {
    // Must have an array
    if( !(is_array($array) && count($array)) )
    {
      $this->fWarn->add(get_class($this) .' Missing data: must be an array');
      return false;
    }
    foreach( $array as $key => $val )
    {
      // If it's numeric, use nodeName.
      if( is_numeric($key) )
      {
        $key = $nodeName;
      }
      // Create the new node...can't use createNode() since we only want the element without values here
      $node = $this->doc->createElement( $this->sanitizeName($key) );
      // If there's no value and there are children nodes, remove them (to get rid of any empty text nodes)
      if( !$val && $node->hasChildNodes() )
      {
        $node->removeChild( $node->firstChild );
      }
      // If we have attributes...
      if( $attributes )
      {
        $this->addAttributes( $node, $attributes );
      }
      // If this value is an array, recurse
      if( is_array($val) )
      {
        $this->appendArray( $val, $nodeName, null, $node );
      } else {
        $node = $this->createNode( $key, $val, $attributes );
      }
      // Finally, perform the append
      if( $appendTo !== null )
      {
        if( is_object($node) )
        {
          $appendTo->appendChild( $node );
        }
      } else {
        $this->doc->documentElement->appendChild( $node );
      }
    }
    return $node;
  }

  /**
   * appendDoc
   *
   * Combines two docs.
   *
   * @param Object the document to add to this one
   * @param Object the spot in this document at which to append
   * @return boolean
   */
  private function appendDoc( $newDoc, $appendTo = null )
  {
    // Check for TDOM vs just DOM
    if( is_object($newDoc->documentElement) )
    {
      $incoming = $newDoc->documentElement;
    } else {
      $incoming = $newDoc->doc->documentElement;
    }
    // If no incoming doc, we have an error
    if( !$incoming )
    {
      $this->fWarn->add(get_class($this) .' Incoming document is invalid');
      return false;
    }
    // import the incoming doc
    $tmp = $this->doc->importNode( $incoming, true );
    if( !is_object($appendTo) )
    {
      $appendTo = $this->doc->documentElement;
    }
    // append it
    $appendTo->appendChild( $tmp );
    return $appendTo;
  }

  /**
   * appendElement
   *
   * Imports a supplied node into the current doc.
   *
   * @param Object the incoming element
   * @param Object the node to which to attach
   * @return boolean
   */
  private function appendElement( $incoming = null, $appendTo = null )
  {
    if( !$incoming )
    {
      $this->fWarn->add(get_class($this) .' Incoming document is invalid');
      return false;
    }

    // incoming must be DOM, not Simple
    if( substr_count(get_class($incoming),'Simple') )
    {
      $incoming = dom_import_simplexml( $incoming );
    }
    $tmp = $this->doc->importNode( $incoming, true );
    if( !is_object($appendTo) )
    {
      $appendTo = $this->doc->documentElement;
    }
    // append it
    $appendTo->appendChild( $tmp );
    return $appendTo;
  }

  /**
   * createNode
   *
   * Creates a node with an optional value and attributes.
   *
   * @param String the name of the node
   * @param String the value of the node
   * @param Array key=>value list
   * @return Object new node
   */
  private function createNode( $nodeName = null, $textValue = null, $attributes = null )
  {
    if( !$nodeName || !is_string($nodeName) )
    {
      $this->fWarn->add(get_class($this) .' Missing information: nodeName must be a string');
      return false;
    }

    // Create the holding node.
	// cast to string in case we get an object.
	if( preg_match("/[<>&]/", strval($textValue)) )
	{
		$node = $this->doc->createElement( $this->sanitizeName($nodeName) );
		$tmp = $this->doc->createCDATASection( $textValue );
		$node->appendChild( $tmp );
	} else {
		$node = $this->doc->createElement( $this->sanitizeName($nodeName), $this->escapeText($textValue) );
	}
    // If there's not value, remove the text child, which DOM creates by default
    if( !$textValue )
    {
      $node->removeChild( $node->firstChild );
    }
    $this->addAttributes( $node, $attributes );
    return $node;
  }

  /**
   * addAttributes
   *
   * Adds attributes to the supplied node.
   *
   * @param Object the node
   * @param Array key=>val list
   * @return Object supplied node
   */
  private function addAttributes( $node = null, $attributes = null )
  {
    // attach attributes if we have them
    if( is_array($attributes) )
    {
      foreach( $attributes as $attName => $attValue )
      {
        $att = $this->doc->createAttribute( $this->sanitizeName($attName) );
        $val = $this->doc->createTextNode( $this->escapeText($attValue) );
        $att->appendChild( $val );
        $node->appendChild( $att );
        unset( $att );
        unset( $val );
      }
    }
    return $node;
  }

  /**
   * replaceNode
   *
   * Imports another doc, then replaces the specified node in this doc with it.
   *
   * @param Object the new TDOM object
   * @param Object the node to be replaced
   * @param String an xpath to be run on this document which identifies the node to be replaced
   * @return Boolean
   */
  public function replaceNode( $replaceWith = null, $node = null, $xpath = null )
  {
    // Must have an object for replaceWith and either node or xpath
    if( !is_object($replaceWith) || !($node || $xpath) )
    {
      $this->fWarn->add(get_class($this) .' Missing information: replaceWith must be an object and either node or xpath is required');
      return false;
    }
    // If we have an xpath, set Node to the query results
    if( $xpath )
    {
      $tmp = $this->get( $xpath, null, 'dom' );
      $node = $tmp->item(0);
    }
    // Now perform the replacement
    $tmp = $this->doc->importNode( $replaceWith->doc->documentElement, true );
    if( !$node->parentNode )
    {
      $this->fDebug->add(get_class($this) .' parentNode of this node is invalid or missing, appending to this doc', 6);
      $this->doc->documentElement->appendChild( $tmp );
    } else {
      $node->parentNode->replaceChild( $tmp, $node );
    }
    return true;
  }

  /**
   * setStyleSheet
   *
   * Adds the xml-stylesheet processing instruction at the top of this document.
   *
   * @param String relative path to the XSL file
   * @return boolean
   */
  public function setStyleSheet( $path = null )
  {
    if( !$path )
    {
      $this->fWarn->add(get_class($this) .' Missing information: path must be a string');
      return false;
    }
    $instr = $this->doc->createProcessingInstruction( 'xml-stylesheet', 'href="' .$path .'" type="text/xsl"' );
    $this->doc->insertBefore( $instr, $this->doc->documentElement );
    return true;
  }

  /**
   * escapeText
   *
   * Returns values suitable for inclusion in a UTF-8 document.
   * If the document will not be UTF-8, it just returns the value, untouched.
   *
   * @param String value to be escaped
   * @return escaped value
   */
  private function escapeText( $str = null ){
    // If we're encoding to UTF-8, escape MS Windows-1252 characters, then run iconv on it.
    // Otherwise, just return what we got.
    if( strtolower($this->encoding) == 'utf-8' ){
      // Note: your editor must be set to ISO-8859-1 or UTF-8, or the replaceMap array below will no longer work.
      // You should see actual characters in every other key below.
      $map = array(
        "&"     => '&amp;',     // 2 'must be' escaped chars in xml
        "<"     => '&lt;',
        "•"     => '&#x2022;',  // bullet: ascii 149
        "\x95"  => '&#x2022;',
        " "     => '&#160;',    // non-breaking space: ascii 160
        "\xA0"  => '&#160;',
        "©"     => '&#169;',    // copyright: ascii 169
        "\xA9"  => '&#169;',
        "®"     => '&#174;',    // registered: ascii 174
        "\xAE"  => '&#174;',
        "™"     => '&#x2122;',	// trademark: ascii 153
        "\x99"  => '&#x2122;',
        "…"     => '&#x2026;',	// ellipsis: ascii 133
        "\x85"  => '&#x2026;',
        "‘"     => '&#x2018;',	// opening single quote: ascii 145
        "\x91"  => '&#x2018;',
        "’"     => '&#x2019;',	// closing single quote: ascii 146
        "\x92"  => '&#x2019;',
        "“"     => '&#x201C;',	// opening double quote: ascii 147
        "\x93"  => '&#x201C;',
        "”"     => '&#x201D;',	// closing double quote: ascii 148
        "\x94"  => '&#x201D;',
        "–"     => '&#x2013;',	// en dash: ascii 150
        "\x96"  => '&#x2013;',
        "—"     => '&#x2014;',	// em dash: ascii 151
        "\x97"  => '&#x2014;'
        );
      // First, run the translation for map.
      $temp = strtr( $str, $map );
      // Finally, run iconv to take care of any remaining encoding issues.
      return iconv( 'ISO-8859-1', 'UTF-8//TRANSLIT', $temp );
    }
    return $str;
  }

  /**
   * sanitizeName
   *
   * Returns a string valid for use as a node name or attribute name.
   *
   * @param String value to sanitize
   * @return String new name
   */
  private function sanitizeName( $name = null )
  {
    // If it's not a valid node name, sanitize it.
    $from = array(
      '/',
      '%',
	  '$',
	  '(',
	  ')',
	  '{',
	  '}'
      );
    $tmp = trim( str_replace($from,'_',$name) );
    // If trimming it results in nothing, call it entry.
    if( !$tmp ){
      $tmp = 'entry';
    }
    // Names must begin with a letter. If it doesn't, prepend it with an underscore.
    if( preg_match( "/[^a-z]+/i", substr($tmp,0,1) ) && substr($tmp,0,1) != '_' )
    {
      $this->fDebug->add(get_class($this) .' Key name (' .$name .') is not valid, returning _' .$tmp, 6);
      $tmp = '_' .$tmp;
    }
    return $tmp;
  }

  /**
   * get
   *
   * Performs an xpath on this doc and returns the results in the specified format.
   *
   * @param String xpath
   * @param String the type of result to return: xml (raw xpath results), value (force xpath results down to a single value), array (convert xpath results to an array)
   * @param String the engine to use: 'dom' (DOM) or simple (SimpleXML)
   * @return Mixed
   */
  public function get( $xpath = null, $returning = 'xml', $engine = 'simple' )
  {
    if( !$xpath )
    {
      $this->fWarn->add(get_class($this) .' Missing information: xpath must be a string');
      return false;
    }

    // start with what we have
    $doc = $this->doc;
    // Convert Doc to the appropriate format
    $thisEngine = strtolower(get_class($doc));

    // if what we have is dom and we want simple...
    if( $thisEngine == 'domdocument' && $engine == 'simple' )
    {
      if( !($doc = @simplexml_import_dom($this->doc)) ){
        $this->fWarn->add(get_class($this) .' Unable to simplexml_import_dom() using ' .$this->xml());
      }
    }
    // if what we have is simple and we want dom...
    if( $thisEngine == 'simplexmldocument' && $engine == 'dom' )
    {
      if( !($doc = @dom_import_simplexml($this->doc)) ){
        $this->fWarn->add(get_class($this) .' Unable to dom_import_simplexml() using ' .$this->xml());
      }
    }

    // Now call the appropriate method
    if( $engine == 'simple' )
    {
      return $this->getSimple( $doc, $xpath, $returning );
    }
    if( $engine == 'dom' )
    {
      return $this->getDOM( $doc, $xpath, $returning );
    }
  }

  /**
   * getSimple
   *
   * Performs an xpath on this doc and returns the results in the specified format.
   *
   * @param Object the document on which to run the xpath
   * @param String xpath
   * @param String the type of result to return: xml (raw xpath results), value (force xpath results down to a single value), array (convert xpath results to an array)
   * @return Mixed
   */
  public function getSimple( $doc = null, $xpath = null, $returning = 'xml' )
  {
    if( !is_object($doc) )
    {
      $this->fWarn->add(get_class($this) .' Missing doc: must be a SimpleXML object');
      return false;
    }

    // Exec xpath
    $xresult = $doc->xpath( $xpath );
    if( !$xresult )
    {
      // we got a false result, which is an error
      if( gettype($xresult) == 'boolean' )
      {
        $this->fWarn->add(get_class($this) .' Unable to execute xpath query: ' .$xpath);
        return false;
      }
    }

    // Return the requested format
    switch( $returning )
    {
      case 'value':
        // If we have something at the first branch of an array...
        if( $xresult[0] )
        {
          return strval($xresult[0]);
        } else {
          return strval($xresult);
        }
        break;

      case 'array':
        return $this->toArray( $xresult );

      default:
        return $xresult;
    }
  }

  /**
   * getDOM
   *
   * Performs an xpath on this doc and returns the results in the specified format.
   *
   * @param Object the document on which to run the xpath
   * @param String xpath
   * @param String the type of result to return: xml (raw xpath results), value (force xpath results down to a single value), array (convert xpath results to an array)
   * @return Mixed
   */
  private function getDOM( $doc = null, $xpath = null, $returning = 'xml' )
  {
    if( !is_object($doc) )
    {
      $this->fWarn->add(get_class($this) .' Missing doc: must be a DOMDocument object');
      return false;
    }

    // Exec xpath
    $x = new DOMXPath( $doc );
    $xresult = $x->query( $xpath );
    if( !$xresult )
    {
      // we got a false result, which is an error
      if( gettype($xresult) == 'boolean' )
      {
        $this->fWarn->add(get_class($this) .' Unable to execute xpath query: ' .$xpath);
        return false;
      }
    }

    // Return the requested format
    switch( $returning )
    {
      case 'value':
        // If we have an item list...
        if( $xresult->item(0) ){
          // and the first item has a value
          if( $xresult->item(0)->nodeValue )
          {
            return $xresult->item(0)->nodeValue;
          } else {
            return $xresult->item(0);
          }
        } else {
          return strval($xresult);
        }
        break;

      case 'array':
        foreach( $xresult as $item )
        {
          $r[] = $this->toArray( $item );
        }
        return $r;
        break;

      default:
        return $xresult;
    }
  }

  /**
   * toArray
   *
   * Determines the engine of the supplied doc (simple or dom) and calls the appropriate *2array() method.
   *
   * @param Object the node / document to convert
   * @return Array
   */
  public function toArray( $doc = null )
  {
    // doc can be an object or a numerically-indexed array of simplexml objects, in the case of simple xpath results
    if( !(is_object($doc) || is_object($doc[0])) )
    {
      $this->fWarn->add(get_class($this) .' Missing doc: must be an object');
      return false;
    }

    // Determine the doc engine: simplexml or dom
    switch(true)
    {
      case substr_count(get_class($doc),'DOM'):
        // If we have an xpath result...
        if( get_class($doc) == 'DOMNodeList' )
        {
          foreach( $doc as $item )
          {
            $tmp[] = $this->dom2array($item);
          }
          return $tmp;
        } else {
          return $this->dom2array($doc);
        }
        break;

      case substr_count(get_class($doc),'Simple'):
      case substr_count(get_class($doc[0]),'Simple'):
        return $this->simplexml2array($doc);
        break;

      default:
        $this->fWarn->add(get_class($this) .' Unable to determine xml engine: ' .get_class($doc));
        return false;
    }
  }

  /**
   * dom2array
   *
   * Converts a DOM document to an array.
   *
   * @param Object the node / document to convert
   * @param Array the generated array
   * @return Array
   */
  private function dom2array( $node = null, &$array = null )
  {
    $ptr = &$array;
    // If we don't have a dom, set it to this object's document
    if( !$node )
    {
      $node = $this->doc;
    }
    // Get the first child, we'll go to the next sibling at the end of this loop
    $node = $node->firstChild;
    while( !is_null($node) )
    {
      unset($attlist);
      switch( $node->nodeType )
      {
        case XML_CDATA_SECTION_NODE:
        case XML_TEXT_NODE:
          $ptr = $node->nodeValue;
          break;
        // It's an element, which can contain other nodes...text values are also nodes inside an element
        case XML_ELEMENT_NODE:
          if( $node->hasAttributes() )
          {
            foreach( $node->attributes as $i => $att )
            {
              $attlist[ $att->name ] = $att->value;
            }
          }
          // If we have more than one level of children,
          // force the array to be numeric so that we can have more than one branch with the same nodeName.
          // Otherwise, just add it as a simple text value.
          if( $node->firstChild->firstChild )
          {
            $ptr =& $array[ $node->nodeName ][];
            if( $attlist )
            {
              $array[ $node->nodeName ][ count($array[$node->nodeName]) - 1 ]['@'] = $attlist;
            }
          } else {
            if( $attlist )
            {
              $array[ $node->nodeName ]['@'] = $attlist;
            }
            $ptr =& $array[ $node->nodeName ];
          }
          break;
      }
      if( $node->hasChildNodes() )
      {
        $this->dom2array( $node, $ptr );
      }
      $node = $node->nextSibling;
    }
    return $array;
  }

  /**
   * simplexml2array
   *
   * Converts a SimpleXML document to an array.
   *
   * @param Object the node / document to convert
   * @return Array
   */
  private function simplexml2array( $doc )
  {
    // If we have a SimpleXMLElement, generate attribute array and then grab object vars at this level.
    if( get_class($doc) == 'SimpleXMLElement' )
    {
      $att = $doc->attributes();
      if( $att )
      {
        foreach( $att as $key => $val )
        {
          if ($val) $attributes[$key] = strval($val);
        }
      }
      // Save the doc to a tmp var for use later
      $tmp = $doc;
      $doc = get_object_vars($doc);
    }
    // If we have an array, process it.
    if( is_array($doc) )
    {
      // If it's an array but the count is zero, it's CDATA, so return the tmp var.
      if( count($doc) == 0 )
      {
        return strval($tmp);
      }
      // Otherwise, recurse
      foreach( $doc as $key => $val )
      {
        // One of the simplexml object vars is @attributes...skip that in favor of a single '@' node
        if( $key != '@attributes' )
        {
          $r[$key] = $this->simplexml2array( $val );
        }
      }
      // Add attributes
      if( isset($attributes) ) $r['@'] = $att;
      return $r;
    }
    // As a last resort, return a string.
    return strval($doc);
  }

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
  static function appendDomNode($document, $node, $attribs = NULL, $child = NULL, $text = NULL, $comment = NULL)
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
  static function pdo2dom($dataObject, $root = 'pdoResult')
  {

    $document = new DOMDocument();
    $pdoresult = $document->createElement($root);
    foreach ( $dataObject AS $row )
    {
      $domRow = appendDomNode($document, 'row');
      foreach ( $row AS $key=>$value )
      $domRow = appendDomNode($document, $domRow, NULL, appendDomNode($document, $key, NULL, NULL, $value));
      $pdoresult = appendDomNode($document, $pdoresult, NULL, $domRow);
    }
    return $pdoresult;
  }

}

?>

<?php // $Id: tMenu.inc 6691 2007-10-18 19:04:19Z jhansche $
############################################################################
#    Copyright (C) 2007 by the Framewerk Development Group                 #
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
 * tMenu
 *
 * This file contains the tMenu object.
 */
/**
 * Menu Display Object.
 *
 * This object is responsible for generating site menu output from the definitions
 * in Configuration/menu.xml and can be accessed via the system variable {@menu}.  Behavior
 * is dictated by the configuration directives in Configuration/registry.xml
 *
 * @author     Gavin M. Roy <gavinmroy@gmail.com>
 * @link       http://framewerk.org
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2007 the Framewerk Development Group
 * @version    $Rev: 6691 $
 * @package    Engine
 * @subpackage Core
 * @since      2006-03-15
 * @todo       Add submenu support
 */

class tMenu extends fObject
{
  private $fTheme, $fSession, $uri, $xmldoc;

  /**
   * Initialize our variables and load the site.xml file
   */
  function __construct()
  {
    parent::__construct();
    $this->fSession = fSession::getInstance();
    $this->fTheme = fTheme::getInstance();
    $this->xmldoc = $this->fMain->site;
    $this->uri = rtrim( $_SERVER['REQUEST_URI'], '/' );
  }

  /**
   * Function that is called by the parser that will return the menu
   *
   * @param array $params
   * @return string
   */
  function buildOutput($params)
  {
    $this->processNodes($this->xmldoc);
    $this->fDebug->add('Making the XML Menu from site.xml', 4);
    $file = $this->fTheme->xslFile('tMenu');
    $this->fDebug->add('Attempting to transform XML Menu from: ' . $file, 4);
    $xml = $this->makeMenuXML($params);
    echo $xml;
    $output = $this->fTheme->transformXML($file, $xml);
    return $output;
  }

  /**
   * Process a condition by running it through the parser then evaluating it
   *
   * @param string $condition
   * @return bool
   */
  function conditionCheck($condition)
  {
    $parser = fParser::getInstance();
    $condition = 'if ( ' . $parser->parse(strval($condition)) . ' ) return true; return false;';
    $this->fDebug->add('Evaluating: ' . $condition, 4);
    $ret = eval($condition);
    return $ret;
  }

 /**
  * Turn the site.xml document into a Menu XML document for XSL transformation
  *
  * @return string HTML for menu
  */
  function makeMenuXML($start = NULL)
  {
    $dom = new DOMDocument('1.0', 'UTF-8');
    $dom->formatOutput = true;
    $menu = $dom->createElement('menu');
    $dom->appendChild($menu);
    if ( substr($start, 0, 1) == "!" )
    {
      $select = $dom->createAttribute('select');
      $select->appendChild($dom->createTextNode(substr($start, 1, strlen($start) - 1)));
      $menu->appendChild($select);
      $start = NULL;
    }

    if ( isset($start) )
    {
      $this->fDebug->add('Building submenu for title: ' . $start, 4);
      $base = $this->xmldoc->xpath('descendant-or-self::key[title = \'' . $start . '\']');
      foreach ( $base AS $key )
      {
        $this->processMenuNode($dom, $menu, $key, rtrim(strval($this->fMain->configuration->site->baseURI), '/'));
      }
      $base = $this->xmldoc->xpath('descendant-or-self::link[title = \'' . $start . '\']');
      foreach ( $base AS $link )
      {
        $this->processMenuNode($dom, $menu, $link, rtrim(strval($this->fMain->configuration->site->baseURI), '/'));
      }
    } else {

      foreach ( $this->xmldoc->key AS $key )
      {
        $this->processMenuNode($dom, $menu, $key, rtrim(strval($this->fMain->configuration->site->baseURI), '/'));
      }

      foreach ( $this->xmldoc->link AS $link )
      {
        $this->processMenuNode($dom, $menu, $link, rtrim(strval($this->fMain->configuration->site->baseURI), '/'));
      }
    }
    return $dom->saveXML();
  }

  /**
   * Recursive function to process menu items
   *
   * @param DOMDocument $dom
   * @param DOMElement $parent
   * @param SimpleXMLElement $node
   */
  function processMenuNode($dom, $parent, $node, $baseURI = NULL)
  {
    if ( strval($node['visible']) == '1' )
    {

      $menuItemConfig = NULL;
      if ( isset($node->tMenu) )
      {
        foreach ( $node->tMenu as $tMenuNode )
        {
          if ( ! isset($tMenuNode['condition']) || $this->conditionCheck($tMenuNode['condition']) )
          {
            $menuItemConfig = $tMenuNode;
            break;
          }
        }
      }

      $item = $dom->createElement('item');

      if ( $node['nosubmenu'] == '1' )
      {
        $item->setAttribute('preventSubmenu', '1');
      }

      $url = $dom->createAttribute('url');
      if ( isset($node['url']) )
      {
        $url->appendChild($dom->createTextNode($node['url']));
      } else {
        if ( isset($node['service']) )
        {
          
        }
        $url->appendChild($dom->createTextNode($baseURI . $node['uri']));
      }
      $item->appendChild($url);

      // Override node div class
      if ( $menuItemConfig !== NULL && isset($menuItemConfig['class']) )
      {
        $item->setAttribute('class', $menuItemConfig['class']);
      }

      // Override node title
      if ( $menuItemConfig !== NULL && isset($menuItemConfig['title']) )
      {
        $item->setAttribute('title', $menuItemConfig['title']);
      } else if ( $node->title )
      {
        $item->setAttribute('title', $node->title);
      }


      // Check to see if it should be selected
      $currentURI = ( $node['uri'] ) ? ( rtrim( $baseURI . $node['uri'], '/' ) ) : ( rtrim( $node['url'], '/' ) );

      // Note a node as selected if the uri's match
      if ( $currentURI == $this->uri )
      {
        $selected = $dom->createAttribute('selected');
        $selected->appendChild($dom->createTextNode('1'));
        $item->appendChild($selected);
      }

      // Check to see if it's a member of a group
      if ( isset($node['group']) )
      {
        $grp = $this->xmldoc->xpath("//group[@name='" . $node['group'] . "']");
        $xpath = new DOMXPath($dom);
        if ( isset($grp[0]->title) )
        {
          $groupTitle = $grp[0]->title;
        } else {
          $groupTitle = $node['group'];
        }
        $nodeList = $xpath->query("//menu/item/descendant-or-self::*[@title = '" . $groupTitle . "']");
        if ( $nodeList->length == 0 )
        {
          $group = $dom->createElement('item');
          $title = $dom->createAttribute('title');
          $group->appendChild($title);

          $temp = $dom->createAttribute('url');
          $temp->appendChild($dom->createTextNode($baseURI . $grp[0]->uri));
          $group->appendChild($temp);

          if ( isset($grp[0]->link) )
          {
            $temp = $dom->createAttribute('link');
            $temp->appendChild($dom->createTextNode($grp[0]->link));
            $group->appendChild($temp);
          }

          // Override the title if it's set
          if ( isset($grp[0]->title) )
          {
            $title->appendChild($dom->createTextNode($grp[0]->title));
          } else {
            $title->appendChild($dom->createTextNode($node['group']));
          }

          // if our testuri and our group uri match then mark it as selected
          $turi = rtrim($baseURI . $grp[0]->uri, '/');
          if ( ( $this->uri == $turi ) || ( $groupTitle == fOutput::getInstance()->getPageCategory() ) )
          {
            $selected = $dom->createAttribute('selected');
            $selected->appendChild($dom->createTextNode('1'));
            $group->appendChild($selected);
          }
          $parent->appendChild($group);
          $group->appendChild($item);
        } else {
          $nodeList->item(0)->appendChild($item);
        }
      } else {
        $parent->appendChild($item);
      }

      if ( isset($node->key) )
      {
        foreach ( $node->key AS $key)
        {
          $this->processMenuNode($dom, $item, $key, $currentURI);
        }
      }

      if ( isset($node->link) )
      {
        foreach ( $node->link AS $link)
        {
          $this->processMenuNode($dom, $item, $link, $currentURI);
        }
      }
    }
  }

  /**
   * Process a SimpleXML based site node toggling visiblity based upon condition, permission, and host
   *
   * @param SimpleXML Node $node
   */
  function processNode($node)
  {
    if ( strval($node['visible']) == '1' )
    {
      // If we have a permission set, check it
      if ( isset($node['permission']) )
      {
        if ( $this->fSession->fUser instanceOf fUser )
        {
          $node['visible'] = $this->fSession->fUser->checkPermission(strval($node['permission']));
        } else {
          if ( $node['permission'] != 'All' )
          {
            $node['visible'] = false;
          }
        }
      }

      // If there is a condition, process it
      if ( isset($node['condition']) )
      {
        $node['visible'] = $node['visible'] && $this->conditionCheck(strval($node['condition'][0]));
      }

      // If we have a hostname see if we have a match
      if ( isset($node['host']) )
      {
        if ( strToLower($node['host']) != strToLower($_SERVER['SERVER_NAME']) )
        {
          $node['visible'] = false;
        }
      }

      // Process Keys
      foreach ( $node->key AS $key )
      {
        $this->processNode($key);
      }

      // Process Links
      foreach ( $node->link AS $link )
      {
        $this->processNode($link);
      }

      // Process Actions
      foreach ( $node->action AS $action )
      {
        $this->processNode($action);
      }
    }
  }

  /**
   * Process a SimpleXML document
   *
   * @param SimpleXML Document $nodes
   */
  function processNodes($nodes)
  {
    foreach ( $nodes AS $node )
    { 
      $this->processNode($node);
    }
  }
}

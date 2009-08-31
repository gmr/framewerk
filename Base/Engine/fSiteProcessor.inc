<?php // $Id: fSiteProcessor.inc 945 2007-12-20 21:10:24Z madcoder $
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
 * Site Processor Class.
 *
 * This class processes the site.xml document and performs the appropriate actions based
 * upon selected keys and the various attributes and nodes per key.
 *
 * @author     Gavin M. Roy <gavinmroy@gmail.com>
 * @link       http://framewerk.org
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2006-2007 the Framewerk Development Group
 * @version    $Rev: 945 $
 * @since      2006-02-16
 * @package    Engine
 * @subpackage Core
 * @todo       Implement the host and include attributes for key and action
 */

class fSiteProcessor
{
  protected $fDebug, $fMain, $fOutput, $fSession;
  private $uri, $xmlDoc, $class, $uriDepth = 0;

  /**
   * Construct the fSiteProcessor Object
   *
   * @param SimpleXMLObject $sitexml
   */
  function __construct($sitexml)
  {
    $this->fDebug = fDebug::getInstance();
    $this->fMain = fMain::getInstance();
    $this->fOutput = fOutput::getInstance();
    $this->fSession = fSession::getInstance();
    $this->xmlDoc = isset($sitexml) ? $sitexml : NULL;
  }

  /**
   * Comments would be nice here, eh? What does this function actually do?
   */  
  function processKey($node, $depth = 0)
  {
    // If we're this deep, we're deep in it
    if ( $depth > 100 ) return false;

    if ( !isset($gmrcount) ) $gmrcount =0;
    $this->fDebug->add('Processing Key at depth: ' . $depth, 4);
    $process = false;
    $processed = false;

    // Get the URI we want to use
    $uri = '';
    for ( $i = $depth; $i < count($this->uri); $i++ )
    {
      $uri .= '/' . $this->uri[$i];
    }
    
    // Get the current domain via registry
    $currentDomain = $this->fMain->configuration->site->URL;

    if ( $uri == '') 
    {
      $uri = '/';
    }

    // Run xpath to find matching URI nodes
    $xml = $node->xpath( 'key[starts-with("' . $uri . '", @uri)]' );

    // Sort the keys
    usort( $xml, array( 'fSiteProcessor', '_arrangeKeyNodes' ) );
    
    foreach ( $xml as $key )
    {
      if ( isset($key['uri']) )
      {
        $count = count(explode('/', trim($key['uri'], '/')));
        $uri = '';

        for ( $y = $depth; $y < ( $count + $depth); $y++ )
        {
          $uri .= '/' . $this->uri[$y];
        }
        
        // Process and see if our URI's match
        if ( $uri == $key['uri'] )
        {
          $this->fDebug->add('URI Match on &ldquo;' . $uri . '&rdquo;', 8);
          $process = true;

          // If there is a condition, process it
          if ( isset($key['condition']) )
          {
            if ( ! $process = $this->conditionCheck( strval( $key['condition'] ) ) )
            {
              if ( isset($key->conditionFailure) )
              {
                $process = $this->conditionFailure($key);
              }
            }
          }

          // If we have a permission set, check it
          if ( isset($key['permission']) )
          {
            if ( $this->fSession->fUser instanceOf fUser )
            {
              if ( $this->fSession->fUser->checkPermission(strval($key['permission'])) !== true )
              {
                throw new fHTTPException(403, 'You do not have permission to access this key [' . $key['permission'] . ']');
              }
            } else {
              if ( $key['permission'] != 'All' )
              {
                throw new fHTTPException(403, 'You do not have permission to access this key [' . $key['permission'] . ']');
              }
            }
          }

          // If we have a hostname see if we have a match
          if ( isset($key['host']) )
          {
            if ( strtolower($key['host']) != strtolLower($_SERVER['SERVER_NAME']) )
            {
              $process = false;
            }
          }  

          // Process any sub-keys if we have them
          $response = false;
          if ( isset($key->key) && ( $process == true ) )
          {
            $response = $this->processKey($key, ($depth + $count));
            if ( ( $process == false ) && ( $response == true ) ) $process = true;
          }

          // Force SSL if required
          if ( intval($key['force-ssl']) == 1 && $_SERVER['SERVER_PORT'] != 443 )
          {
            header('Location: https://' . $_SERVER['HTTP_HOST'] . $_SERVER['REQUEST_URI']);
            exit();
          }

          // If we have permission and match conditions process the request
          if ( ( $process == true ) && ( $response == false ) )
          {
            $this->setURIValues(($depth + $count));

            // Process Content if there is any
            foreach ($key->content AS $content)
            {
              switch ( strToLower($content['type']) )
              {
                case 'string':
                  $this->fOutput->appendContent(strval($content));
                  $processed = true;
                  break;
                default:
                  trigger_error('Invald Content Type &ldquo;' . $key->content['type'] . '&rdquo;', E_USER_WARNING);
                  break;
              }
            }

            // Set the title if there is one
            if ( isset($key->class) )
            {
              $this->class = strval($key->class);
            }

            // Set the title if there is one
            if ( isset($key->title) )
            {
              $this->fOutput->setPageName(strval($key->title));
            }

            // Override our theme if we need to
            if ( isset($key->theme) )
            {
              fTheme::getInstance()->override(strval($key->theme));
            }

            // Set the pageCategory if there is a group
            if ( isset($key->category) )
            {
              $this->fOutput->setPageCategory(strval($key->category));
            }

            // Process any actions if we have them
            if ( ( isset($key->action) ) && ( $response != true ) )
            {
              $response = $this->processAction($key, ($depth + $count));
              if ( ( $process == false ) && ( $response == true ) ) $process = true;
            }

            // If we didn't process a sub key or action successfully, try and see if we can execute a function on this class
            if ( $response != true )
            {
              if ( strlen($this->class) )
              {
                $this->fDebug->add('Setting class to use to &ldquo;' . $this->class . '&rdquo;', 8);
                $className =  $this->class;
                fMain::getInstance()->lastInterface = $className;
                // Check to see if we have a function and execute it if so, otherwise try and execute the default function
                if ( isset($action->function) )
                {
                  $functionName = strval($action->function);
                  $obj = new $className();
                  $this->fDebug->add('Calling ' . $className . '->' . $functionName . '(' . print_r($this->fSession->uriValues, true) . ')', 8);
                  $processed = $obj->$functionName($this->fSession->uriValues);
                } elseif ( method_exists($className, 'execute') ) {
                  $obj = new $className();
                  $this->fDebug->add('Calling ' . $className . '->execute(' . print_r($this->fSession->uriValues, true) . ')', 8);
                  $processed = $obj->execute($this->fSession->uriValues);
                }
              }
            }
          }
        }
      }
      // Break if our xpaths dont match anymore or if we've processed a result
      if ( ( $processed == true ) || ( $key['uri'] != $uri ) )
        break;
    }

    return $process;
  }

  function processAction($node, $depth = 0)
  {
    $this->fDebug->add('Processing Action at depth: ' . $depth, 8);
    $this->uriDepth = $depth;

    // Loop through the actions that were passed in
    foreach ( $node->action AS $action )
    {
      $process = false;
      $count = 0;
      if ( isset($action['uri']) )
      {
        $count = count(explode('/', ltrim($action['uri'], '/')));
        $uri = "";
        for ( $y = $depth; $y < ( $count + $depth); $y++ )
          $uri .= '/' . $this->uri[$y];

        // Process and see if our URI's match
        if ( $uri == $action['uri'] )
          $process = true;

      } else
      $process = true;

      // Set URI Values
      $this->setURIValues(($depth + $count));

      // If we still can process, check our permissions and conditions
      if ( $process == true )
      {
        // Check the permission
        if ( isset($action['permission']) )
        {
          if ( $this->fSession->fUser instanceOf fUser )
          {
            if ( $this->fSession->fUser->checkPermission(strval($action['permission'])) !== true )
            {
              throw new fHTTPException(403, 'You do not have permission to access this action [' . $action['permission'] . ']');
            }
          } else {
            if ( $action['permission'] != 'All' )
            {
              throw new fHTTPException(403, 'You do not have permission to access this action [' . $action['permission'] . ']');
            }
          }
        }

        // If there is a condition, process it
        if ( isset($action['condition']) )
          if ( !$process = $this->conditionCheck(strval($action['condition'])) )
            if ( isset($action->conditionFailure) )
              $process = $this->conditionFailure($action);

        // If we have a hostname see if we have a match
        if ( isset($key['host']) )
          if ( StrToLower($key['host']) != StrToLower($_SERVER['SERVER_NAME']) )
            $process = false;
      }

      // If we can process, do so
      if ( $process == true )
      {
        // Set the title if there is one
        if ( isset($action->title) )
          $this->fOutput->setPageName($action->title);

        // Set the pageCategory if there is a group
        if ( isset($action->group) )
          $this->fOutput->setPageCategory($action->group);

        // Override our theme if we need to
        if ( isset($action->theme) )
          fTheme::getInstance()->override(strval($action->theme));

        // If class is set instantiate it
        if ( isset($action->class) )
          $this->class = strval($action->class);

        // If we have a class set run the function if it's there
        $processed = false;
        if ( isset($this->class) )
        {
          $className = $this->class;
          $obj = new $className();
          $className = $this->class;
          if ( isset($action->function) )
          {
            $functionName = strval($action->function);
            $this->fDebug->add('Calling ' . $className . '->' . $functionName . '(' . print_r($this->fSession->uriValues, true) . ')', 8);
            $processed = $obj->$functionName($this->fSession->uriValues);
          }
        }

        // If we have sub-actions try and perform them
        if ( isset($action->action) && ( $processed != true ) )
          $processed = $this->processAction($action, ($depth + $count));
      }

      // Break if our xpaths dont match anymore or if we've processed a result
      if ( ( $processed == true ) || ( $action['uri'] != $uri ) )
      break;
    }
    return $processed;
  }

  function process($uri)
  {
    $temp = explode('/', trim($uri, '/'));
    $this->uri = array();
    foreach ( $temp AS $value )
    {
      if ( strlen(Trim($value)) )
      {
        $this->uri[] = $value;
      }
    }

    return $this->processKey($this->xmlDoc);
  }

  function conditionCheck($condition)
  {
    $parser = fParser::getInstance();
    $condition = 'if ( ' . $parser->parse($condition) . ' ) return true;';
    $this->fDebug->add('Evaluating: ' . $condition, 8);
    $ret = @eval($condition);
    return $ret;
  }

  function conditionFailure($key)
  {
    if ( isset($key->conditionFailure) )
    {
      foreach ( $key->conditionFailure->perform AS $failure )
      {
      	if ( $failure['parse'] == 1 )
      	{
      	  $parser = fParser::getInstance();
      	  $failure[0] = $parser->parse( $failure[0] );
      	}
      	
        switch ( StrToLower($failure['type']) )
        {
          case 'error':
            fMessages::getInstance()->add(new fErrorMessage(strval($failure)));
            $this->fSession->save();
            $process = false;
            break;
          case 'string':
            $this->fOutput->appendContent(strval($failure));
            $process = false;
            break;
          case 'redirectlogin':
            $failure = fSiteProcessor::siteURI('TLogin') . '?redirect=/' . join('/', $this->uri);
            // fall through to redirect:
          case 'redirect':
            Header('Location: ' . strval($failure));
            exit();
          case 'fail':
            $process = false;
            break;
          default:
            throw new Exception('Unknown Condition Failure Type: ' . strval($failure['type']));
        }
      }
    }
    return $process;
  }

  /**
   * Rebuild our fSession Keyword and URI Values based upon our execution attempts
   */
  function setURIValues($depth)
  {
    // Set our uriValues
    $uriValues = array();
    for ( $y = $depth; $y < count($this->uri); $y++ )
    {
      if ( strlen($this->uri[$y]) )
      {
        $uriValues[] = $this->uri[$y];
      }
    }
    $this->fDebug->add('Setting fSession uriValues to: ' . print_r($uriValues, true), 8);
    $this->fSession->uriValues = $uriValues;

    // Set the session keyword
    $keyword = "";
    for ( $y = 0; $y < ($depth); $y++ )
      $keyword .= $this->uri[$y] . '/';
    $keyword = rtrim($keyword, '/');
    $this->fDebug->add('Setting fMain keyword to &ldquo;' . $keyword . '&rdquo;', 8);
    $this->fMain->keyword = $keyword;
  }

  /**
   * Arranges key nodes by string length
   *
   * @param string $a
   * @param string $b
   */
  private static function _arrangeKeyNodes($a, $b)
  {
    if ( !isset($a['uri']) )
      return 1;
    if ( $a['uri'] == $b['uri'] )
      return 0;
    if ( strlen($a['uri']) > strlen($b['uri']) )
      return -1;
    else
      return 1;
  }

  /**
   * Takes site.xml key options and works backwards to generate the uri's leading to the matched key
   * Options left blank will be skipped
   *
   * @param string $class
   * @param string $function
   * @param string $title
   * @param string $group
   */
  public function siteURI($class, $function, $title, $group)
  {
      // Make this use static accessors so that we can call fSiteProcessor::siteURI()
    if ( !isset($class) && !isset($function) && !isset($title) && !isset($group) )
      return strval(fMain::getInstance()->configuration->site->baseURI) . fMain::getInstance()->keyword;

    $xml = fMain::getInstance()->site;
    $xpath = '//key[@uri]';
    if ( isset($class) )
      $xpath .= "[class='" . $class . "']";
    if ( isset($function) )
      $xpath .= "[function='" . $function . "']";
    if ( isset($title) )
      $xpath .= "[title='" . $title . "']";
    if ( isset($group) )
      $xpath .= "[@group='" . $group . "']";
    $results = $xml->xpath($xpath);
    if ( !sizeof($results) )
      return false;
    $uri     = array();
    $uri[]   = strval($results[0]['uri']);
    if ( sizeof($xml->xpath($newPath = $xpath . '/..')) )
      $uri = self::siteURIParent($xml, $newPath, $uri);
    return join('', array_reverse($uri));
  }

  /**
   * Private recursive function to generate an array of uri's leading to the given xpath
   *
   * @param object $xml
   * @param string $xpath
   * @param string $uri
   */
  private static function siteURIParent($xml, $xpath, $uri)
  {
    $parent = $xml->xpath($xpath);
    $uri[]  = strval($parent[0]['uri']);
    if ( sizeof($xml->xpath($newPath = $xpath . '/..')) )
      $uri = self::siteURIParent($xml, $newPath, $uri);
    return $uri;
  }

  static function returnClassURI($className)
  {
    $template = <<<TEMPLATE
<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
 <xsl:output encoding="ISO-8859-15" omit-xml-declaration="yes" method="xml" version="1.0" indent="yes" />
 <xsl:template match="/site">
   <xsl:for-each select="//key/class[.='tSoapServer']">
    <xsl:for-each select="ancestor-or-self::key">
     <xsl:value-of select="@uri" />
    </xsl:for-each>
   </xsl:for-each>
 </xsl:template>
</xsl:stylesheet>
TEMPLATE;
    $xsl = DOMDocument::loadXML($template);
    $proc = new XSLTProcessor();
    $proc->importStyleSheet($xsl);
    $dom = new DOMDocument('1.0', 'UTF-8');
    $temp = $dom->importNode(dom_import_simplexml(fMain::getInstance()->site), true);
    $dom->appendChild($temp);
    $result = $proc->transformToXml($dom);
    return trim($result);
  }
}
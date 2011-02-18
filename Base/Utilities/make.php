#!/usr/local/bin/php
<?php // $Id: make.php 906 2007-08-04 22:33:21Z fred $
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
 * Developer Basic Application Utility
 *
 * This program creates a basic Application automatically based on given options.
 *
 * @author     Fred Ghosn <fredghosn@gmail.com>
 * @link       http://framewerk.org
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2006-2007 the Framewerk Development Group
 * @since      2006-11-04
 * @package    Applications
 * @subpackage Utilities
 * @version    $Rev: 906 $
 */

  define('NEWLINE', "\n");
  define('VERSION', '1.1');

  // Gather our arguements into a nice little array ;-)
  $options            = array();
  $options['with']    = array();
  $options['without'] = array();

  unset($argv[0]);
  foreach( $argv as $arg )
  {
    if ( substr($arg, 0, 2) == '--' )
    {
      $arg = substr($arg, 2);
      if ( strpos($arg, '=') !== false )
      {
        $tmp = explode('=', $arg, 2);
        $options[$tmp[0]] = $tmp[1];
      } elseif ( strpos($arg, '-') !== false ) {
        $tmp = explode('-', $arg, 2);
        $options[$tmp[0]][$tmp[1]] = true;
      } else {
        $options[$arg] = true;
      }
    }
  }

  // Return help document
  if ( $options['help'] )
  {
    $message = <<<HELPMESSAGE
Usage: make.php <Application Name> [options]

Options:
--type=<type>         Set the type of Application
--theme=<name>        Name of theme to create default theme object files in
--ladder=<table>      Generate a basic 'Lazy Add Edit Remove' Object
--keyword=<keyword>   Keyword to add to site.xml document

--with-dom            Instance DOMDocument
--with-registry       Instance fRegistry for the Application settings

--without-js          Disable creation of default JavaScript Object File
--without-css         Disable creation of default CSS Object File
--without-dom         Disable DOMDocument Instance when using Application
                      type of "xml"
--without-xsl         Disable XSL output when using DOM in a regular
                      interface

--version             Print version information and exit
--help                Print help (this message) and exit


HELPMESSAGE;
    die($message);
  }

  // Return utilities version
  if ( $options['version'] )
  {
    die('Application Generator Version ' . VERSION . '.' . NEWLINE);
  }

  // Function for printing messages
  function msg($msg = '', $exit = false)
  {
    if ( $msg != '' )
    {
      print('[Framewerk] :: ' . $msg . NEWLINE);
    }
    if ( $exit )
    {
      exit();
    }
    return true;
  }

  // Make sure framewerk Applications directory exists
  if ( !file_exists('Applications') )
  {
    msg('(Fatal Error) -> Missing Framewerk Applications Directory.', true);
  }

  // Make sure framewerk configuration exists
  if ( !file_exists('Configuration/registry.xml') )
  {
    msg('(Fatal Error) -> Missing Framewerk "registry.xml" file.', true);
  }

  // Load configuration document
  if ( !$xml = simplexml_load_file('Configuration/registry.xml') )
  {
    msg('(Fatal Error) -> Invalid "registry.xml" file.', true);
  }

  // Make sure this is a valid Application name
  if ( !preg_match('/^[a-z][0-9a-z]+$/i', ($applicationName = $argv[1])) )
  {
    msg('(Fatal Error) -> Invalid Application name supplied.', true);
  }

  // Check if an Application with this name already exists
  if ( file_exists('Applications/' . $applicationName . '.inc') )
  {
    msg('(Fatal Error) -> There is already an Application existing with that name.', true);
  }

  // If no type was specified and a ladder exists, we'll assume this is going to be an admin application
  if ( !strlen($options['type']) && strlen($options['ladder']) )
  {
    $options['type'] = 'admin';
  }

  // Decide what type of Application this is and create some shortcuts
  $isXML = false;
  $isAdmin = false;
  $useXSL = false;

  switch( strtolower($options['type']) )
  {
    case 'admin':
      $isAdmin = true;
      $applicationType = 'fAdminApplication';
      break;
    case 'xml':
      $isXML = true;
      $applicationType = 'fXMLApplication';
      break;
    default:
      $applicationType = 'fApplication';
  }

  msg('Initialize Application of Type "' . $applicationType . '"');

  // Start of Application template
  $classTemplate = <<<TEMPLATE
<?php
/**
 * $applicationName
 *
 * This is some default Application description text.
 *
 * @author     My Name <myname@mydomain.com>
 * @link       http://www.mywebsite.com
 * @license    Unknown
 * @copyright  Unknown
 * @package    Engine
 * @subpackage Applications
 * @uses       $applicationType
 */

class $applicationName extends $applicationType
{
  /**
   * This function is executed when the Application is requested.
   *
   * @returns  (boolean)
   */
  public function execute()
  {

TEMPLATE;

  // Set page name template
  $classTemplate .= <<<TEMPLATE
    // Set page name
    \$this->pageName('$applicationName');

TEMPLATE;

  if ( $isAdmin )
  {
    // Set view permission template
    $classTemplate .= <<<TEMPLATE

    // Set the permission for viewing this Application
    \$this->requirePermission('Administrator');

TEMPLATE;
  }

  // Add a basic registry template
  if ( $options['with']['registry'] )
  {
    $classTemplate .= <<<TEMPLATE

    // Load registry settings for this Application
    // To access registry values,  simply use: \$appRegistry->valuesKey;
    \$appRegistry = new fRegistry('$applicationName');

TEMPLATE;
  }

  // Add CSS Application Object File
  if ( !$options['without']['css'] )
  {
    msg('Adding CSS Application Object File');

    $classTemplate .= <<<TEMPLATE

    // Add Application CSS File
    \$this->fTheme->addObjectCSSFile('$applicationName');

TEMPLATE;
  }

  // Add JavaScript Application Object File
  if ( !$options['without']['js'] )
  {
    msg('Adding JavaScript Application Object File');

    $classTemplate .= <<<TEMPLATE

    // Add Application JavaScript File
    \$this->fTheme->addObjectJSFile('$applicationName');

TEMPLATE;
  }

  if ( ($isXML && !$options['without']['dom']) || $options['with']['dom'] )
  {
    msg('Creating Basic DOM Structure');

    // Create basic DOM structure
    $classTemplate .= <<<TEMPLATE

    // Instance DOM document
    \$dom = new DOMDocument('1.0');
    \$dom->formatOutput = true;
    \$dom->preserveWhiteSpace = false;

    // Create root document node
    \$rootNode = \$dom->createElement('$applicationName');

    // Output XML

TEMPLATE;

    if ( $isXML || $options['without']['xsl'] )
    {
      $classTemplate .= '    $this->output($rootNode);' . NEWLINE;
    } else {
      $useXSL = true;
      $classTemplate .= '    $this->output($rootNode, true, \'' . $applicationName . '\');' . NEWLINE;
    }
  }

  // Admin related Application template code
  if ( $isAdmin )
  {
    // If a Lazy Add Edit Remove was specified generated some basic Ladder code
    if ( strlen($options['ladder']) )
    {
      msg('Adding Lazy Add Edit Remove Instance');

      //fixme: use Registry('tPDO') instead
      $pdo = new PDO($xml->PDO->dsn, $xml->PDO->username, (isset($xml->PDO->password) ? $xml->PDO->password : NULL));
      $query = $pdo->prepare('SELECT f.attname as field FROM pg_tables t JOIN pg_class c ON c.relname = t.tablename JOIN pg_attribute f ON c.oid = f.attrelid WHERE relkind = \'r\' and f.attnum > 0 AND t.schemaname = \'public\' AND t.tablename = :table;');
      $query->bindParam(':table', $options['ladder']);
      $query->execute();

      $classTemplate .= <<<TEMPLATE

    // Instance a copy of 'Lazy Add Edit Remove' for the table '{$options['ladder']}' and send the output
    // You can control and expand upon the Ladder Output by setting meta options on the fields.  Some basic
    // meta options are included as examples; however, no meta options are required for a Ladder to function.
    \$fLadder = new fLadder('{$options['ladder']}');
    \$fLadder->name = '$applicationName - {$options['ladder']}';

TEMPLATE;

      if ( $data = $query->fetchAll(PDO::FETCH_OBJ) )
      {
        foreach( $data as $field )
        {
          $classTemplate .= '    $fLadder->meta[\'' . $field->field . '\'][\'label\'] = \'' . ucfirst($field->field) . '\';' . NEWLINE;
        }
      }

      $classTemplate .= <<<TEMPLATE
    \$this->output(\$fLadder->buildOutput());

TEMPLATE;
    }
  }

  // End of Application
  $classTemplate .= <<<TEMPLATE

    return true;
  }
}
?>
TEMPLATE;

  // Try to write the Application, if it can't be written exit and don't create anything else
  if ( !file_put_contents('Applications/AddOns/' . $applicationName . '.inc', $classTemplate) )
  {
    msg('(Fatal Error) -> There was an error writing the Application Template,  check permissions and try again.', true);
  }

  // Figure out what theme should be used for creating JS/CSS empty templates
  if ( strlen($options['theme']) )
  {
    $theme = $options['theme'];
  } elseif ( $isAdmin ) {
    $theme = strval($xml->site->theme->administration);
  } else {
    $theme = strval($xml->site->theme->default);
  }

  // Strip slashes from keyword
  $options['keyword'] = trim($options['keyword'], '/');

  // If keyword is set try and add site.xml entry
  if ( strlen($options['keyword']) )
  {
    // Instance new DOMDocument
    $dom = new DOMDocument();
    $dom->preserveWhiteSpace = false;
    $dom->formatOutput = true;
    $dom->load('Configuration/site.xml');

    // Instance XPath
    $xpath = new DOMXPath($dom);
    $root = $xpath->query('/site')->item(0);

    if ( $isAdmin )
    {
      $adminURI = '/admin'; // @todo  automatically figure this out...
      $result = $xpath->evaluate('count(/site/key[@uri="' . $adminURI . '"]/key[@uri="/' . $options['keyword'] . '"])');
    } else {
      $result = $xpath->evaluate('count(/site/key[@uri="/' . $options['keyword'] . '"])');
    }
    if ( $result > 0 )
    {
      msg('The keyword you have specified already exists,  site.xml will not be created.');
    } else {
      msg('Adding site.xml keyword');

      // Create basic key element
      $key = $dom->createElement('key');
      $key->setAttribute('uri', '/' . $options['keyword']);
      $key->setAttribute('visible', '1');

      // If this is an Admin Application set default permission to 'Administrator'
      if ( $isAdmin )
      {
        $key->setAttribute('permission', 'Administrator');
      }

      // Set title and class key nodes
      $key->appendChild($dom->createElement('title', $applicationName));
      $key->appendChild($dom->createElement('class', $applicationName));

      // Decide where the key should be entered
      if ( $isAdmin )
      {
        // Append the new key as a child of the admin 'key' node
        $xpath->query('/site/key[@uri="'.$adminURI.'"]')->item(0)->appendChild($key);
      } else {
        // Insert the new key before the last 'key' node in the document.
        $last = $xpath->query('/site/key[last()]');
        $root->insertBefore($key, $last->item(0));
      }

      // Save site.xml
      if ( $dom->save('Configuration/site.xml') === false )
      {
        msg('Could not add specified keyword to "site.xml" document.  Check permissions.');
      }
    }
  }

  if ( !file_exists('Themes/' . $theme) )
  {
    msg('The theme "' . $theme . '" does not exist.');
  } else {
    if ( !$options['without']['js'] )
    {
      if ( !file_put_contents('Themes/' . $theme . '/JavaScript/' . $applicationName . '.js', '/* ' .  $applicationName . ' - JavaScript Object File */' . NEWLINE) === false )
      {
        msg('Unable to write JavaScript file.');
      }
    }
    if ( !$options['without']['css'] )
    {
      if ( file_put_contents('Themes/' . $theme . '/CSS/' . $applicationName . '.css', '/* ' .  $applicationName . ' - CSS Object File */' . NEWLINE) === false )
      {
        msg('Unable to write CSS file.');
      }
    }
    if ( $useXSL )
    {
      $template = <<<XSLTEMPLATE
<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output encoding="ISO-8859-15" omit-xml-declaration="yes" method="xml" version="1.0" indent="yes" />

  <xsl:template match="$applicationName">
  </xsl:template>

</xsl:stylesheet>
XSLTEMPLATE;
      if ( file_put_contents('Applications/XSL/' . $applicationName . '.xsl', $template) === false )
      {
        msg('Unable to write XSL stylesheet.');
      }
    }
  }

  msg('Successfully created a new Application called "' . $applicationName . '".', true);

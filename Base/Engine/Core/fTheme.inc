<?php // $Id: fTheme.inc 945 2007-12-20 21:10:24Z madcoder $
############################################################################
#    Copyright (C) 2004-2007 by the Framewerk Development Group            #
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
 * fTheme
 *
 * This contains the fTheme Class
 */
/**
 * Theme Management Object
 *
 * The fTheme Object is responsible for theme management, combining the output data into the theme and returning a final
 * output for a webpage to the main fMain object.
 *
 * @author     Gavin M. Roy <gavinmroy@gmail.com>
 * @link       http://framewerk.org
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2004-2007 the Framewerk Development Group
 * @version    $Rev: 945 $
 * @package    Engine
 * @subpackage Core
 * @uses       fSingletonObject
 * @since      2004
 */

class fTheme extends fSingletonObject
{
  private static $instance = NULL;
  private $body, $htmlHeadContent, $htmlFooterContent, $registry, $themeDir, $onBuildCallback, $xml = NULL;
  private $jsFiles = array();
  public $folderName, $directory, $graphicsDirectory, $noTheme = false, $name, $prefetching = true, $template, $variableTemplate = false;
  public $author, $version, $preview, $url;

  protected function __construct()
  {
    parent::__construct();
    $this->registry = new fRegistry('fTheme');
    $this->loadTheme($this->registry->default);
  }

  private function loadTheme($name)
  {

    // Build the theme path
    $directory = "{$_SERVER['DOCUMENT_ROOT']}{$this->fMain->configuration->site->relativePath}/Themes/{$name}/";

    // Verify the theme directory exists
    if ( !is_dir($directory ) )
    {
      throw new fHTTPException(501, 'Invalid theme "' . $name . '" specified at: "' . $directory . '"');
    }

    // Verify the theme.xml file is there
    if ( !is_file($directory . '/theme.xml') )
    {
      throw new fHTTPException(501, 'Missing theme.xml file for the theme "' . $name . '"');
    }

    // Verify we have a valid xml file, and load it
    $this->xml = simplexml_load_file($directory . '/theme.xml');
    if ( !is_object($this->xml) )
    {
      throw new fHTTPException(500, 'Invalid theme.xml file for &amp;ldquo;' . $name . '&amp;rdquo;');
    }

    $version = strval($this->xml['version']);
    if ( $version != '1.0' )
    {
      throw new fHTTPException(500, 'Incorrect theme.xml format version for ' . $name . '.  Returned ' . $version . ' and 1.0 is required.');
    }

    $this->folderName        = $name;
    $this->themeDir          = $directory;
    $this->directory         = rtrim($this->fMain->configuration->site->baseURI, '/') . '/Themes/' . $this->folderName . '/';
    $this->graphicsDirectory = rtrim($this->fMain->configuration->site->baseURI, '/') . '/Themes/' . $this->folderName . '/Graphics/';

    $this->name    = strval($this->xml->name);
    $this->version = floatval($this->xml->version);
    $this->author  = strval($this->xml->author);
  }

  /**
   * Checks to see if the object is instanced in the internal static variable $instance
   * and if not, does so
   *
   * @return Object instance
   */
  public static function getInstance()
  {
    if ( self::$instance == NULL )
    {
      self::$instance = new fTheme();
    }
    return self::$instance;
  }

  /**
     * Forcefully unset a singleton instance.
     */
  static function destroy()
  {
    self::$instance = NULL;
  }

  /**
   * Add an object specific CSS file
   *
   * @param String/Object $objectName or Object
   * @param String type of CSS file (screen, print, aural)
   */
  function addObjectCSSFile($objectName, $media = 'screen, print')
  {
    // If we were passed an object, get the object name
    if ( is_object($objectName) )
    {
      $objectName = get_class($objectName);
    }

    // Add a debug message if needed
    $this->fDebug->add('Adding CSS object to XHTML document: ' . $objectName, 4);

    // Build the link file
    $link = '<link rel="stylesheet" href="' . rtrim($this->fMain->configuration->site->baseURI, '/') . '/css/' . $this->folderName . '/CSS/' . $objectName . '.css" type="text/css" media="' . $media . '" />';
    $this->htmlHead($link);
  }

  /**
   * Add an object specific JavaScript file
   *
   * @param String/Object $objectName or Object
   * @param String $objectFile optional js file name
   */
  function addObjectJSFile($objectName, $objectFile = NULL)
  {
    if ( is_object($objectName) )
    {
      $objectName = get_class($objectName);
    }

    if ( $objectFile == NULL )
    {
      $objectFile = $objectName;
    }

    // Check to see if there is a theme override
    $filePath = $this->directory . 'JavaScript/' . $objectFile . '.js';
    // $this->directory is a URL-based path.  Add a . to make it search relative to the doc root for local filesystem
    if ( file_exists('.' . $filePath) )
    {
      // Queue this for later, so we don't include the file twice (e.g., in TFormField objects)
      $this->jsFiles[$objectName] = $filePath;
      return true;
    }

    // Try the default
    $filePath = 'Applications/JavaScript/' . $objectName . '/' . $objectFile . '.js';
    if ( file_exists($filePath) )
    {
      // Queue this for later, so we don't include the file twice (e.g., in fFormField objects)
      $file = '/javascript/' . $objectName . '/' . $objectFile . '.js';
      $this->jsFiles[$objectName] = $file;
      return true;
    }
    return false;
  }

  /**
   * Add a JavaScript file
   *
   * @param String filename
   */
  function addJSFile($fileName)
  {
    // Trim off the .js if they include it
    $fileName = str_replace( '.js', '', $fileName);

    // Look to see if the file exists in our theme dir
    if ( file_exists($this->themeDir . 'JavaScript/' . $fileName . '.js') )
    {
      $this->htmlHead('<script type="text/javascript" src="' . $this->directory . 'JavaScript/' . $fileName . '.js"></script>');
    } else {
      // Look to see if it's in the base JS dir
      if ( file_exists("{$_SERVER['DOCUMENT_ROOT']}{$this->fMain->configuration->site->relativePath}/JavaScript/{$fileName}.js") )
      {
        $this->htmlHead('<script type="text/javascript" src="' . rtrim($this->fMain->configuration->site->baseURI, '/') . '/JavaScript/' . $fileName . '.js"></script>');
      }
    }
  }

  /**
   * Override the currently set theme
   *
   * @param string Theme Name
   */
  function override($theme)
  {
    $this->fDebug->add('Overriding theme to: ' . $theme, 4);
    $this->loadTheme($theme);
  }

  /**
   * Read in our template and merege our content passed in with the theme
   *
   * @param string Content to insert into theme
   */
  function buildPage($content, $mime = NULL)
  {
    fProfile::getInstance()->profile("buildPage");

    // If we don't want to combine the theme with the output just return the output
    if ( $this->noTheme == true )
    return $content;

    // Check for jQuery/addons and include them from the theme.xml file
    if ( is_object($this->xml->jQuery) )
    {
      $jQuery = fjQuery::getInstance();

      // Loop through the plugins and add them
      if ( Count($this->xml->jQuery->plugins->plugin) )
      {
        foreach( $this->xml->jQuery->plugins->plugin as $plugin )
        {
          $jQuery->addPlugin(strval($plugin));
        }
      }

      // Loop through the init code and add it
      if ( Count($this->xml->jQuery->onInit->code) )
      {
        foreach( $this->xml->jQuery->onInit->code as $code )
        {
          $jQuery->addInit(strval($code));
        }
      }
    }

    // If we have debug, warning, or profile output on, add the css
    $fDebug = new fRegistry('fDebug');
    $fProfile = new fRegistry('fProfile');
    $fWarn = new fRegistry('fWarn');

    if ( $this->fDebug->hasMessages() || $this->fProfile->hasMessages() || $this->fWarn->hasMessages() )
    {
      $this->addObjectCSSFile('fEngineInformation');
      $this->addObjectJSFile('fEngineInformation');
      fjQuery::getInstance()->addInit('fEngineInformationInit();');
    }

    // Check for interface .js files and include them
    if ( count($this->jsFiles) > 0 )
    {
      foreach($this->jsFiles as $path)
      {
        $this->htmlHead('<script type="text/javascript" src="' . $path . '"></script>');
      }
    }

    // Run through our callbacks
    foreach ( $this->onBuildCallback AS $callback )
    {
      switch ( $callback['type'] )
      {
        case 'Function':
          $function = $callback['function'];
          $this->fDebug->add('Calling onBuild Callback: ' . $function . '()', 4);
          $function();
          break;

        case 'Singleton':
          $this->fDebug->add('Calling onBuild Callback: ' . $callback['class'] . '->' . $callback['function'] . '()', 4);
          $instance = call_user_func(array($callback['class'], 'getInstance'));
          $function = $callback['function'];
          $instance->$function();
          break;

        default:
          throw new Exception('Invalid onBuild callback type: ' . $callback['type']);
      }
    }

    // If it's XML see if a stylesheet exists
    if ( $mime == "text/xml" )
    {
      // Get our XSL filename if it exists
      $xslfile = $this->xslFile(fMain::getInstance()->lastInterface);

      // Transform if a transformation file exists
      if ( $xslfile )
      {
        // Transform the content using the XSL file
        $content = $this->transformXML($xslfile, $content);

        /** Set the outbound mimetype
         *  @todo should we get this information somehow else, like a comment in the xsl?  It could be xsl-fo, etc outputting
         *  pdf, etc
         */
        fOutput::getInstance()->mimeType("text/html");
      }
    }

    // Check our theme.xml for javascript includes and add them if needed
    foreach ( $this->xml->include->javascript AS $javascript )
    {
      $this->fDebug->add('Adding JavaScript file: ' . strval($javascript), 5);
      $this->addJSFile($javascript);
    }

    // If we have a variable template use it, otherwise use a template file
    if ( $this->variableTemplate )
    {
      // If we're overriding with a variable template
      if ( strlen($this->template) > 0 )
      {
        $page = str_replace('{content}', $content, $this->template);
        fProfile::getInstance()->profile("buildPage", "Page Built");
        return $page;
      } else
      throw new Exception("Variable override template not set.");
    } else {

      // Set the default template
      $template = 'template.html';

      // Check to see if there is an override file and process it if there is
      $override = "{$_SERVER['DOCUMENT_ROOT']}{$this->fMain->configuration->site->relativePath}/css/{$this->folderName}/theme.xml";
      if ( is_file($override) )
      {
        $overrideXML = simplexml_load_file($override);
        foreach ( $overrideXML->override->template AS $templateNode )
        {
          $attributes = $templateNode->attributes();
          foreach ( $attributes AS $key=>$value )
          {
            switch ($key)
            {
              case 'pageCategory':
                if ( $value == fOutput::getInstance()->pageCategory )
                $template = strval($templateNode);
                break;
              case 'pageName':
                if ( $value == fOutput::getInstance()->pageName )
                $template = strval($templateNode);
                break;
              case 'Interface':
                if ( $value == $this->fMain->lastInterface )
                $template = strval($templateNode);
                break;
            }
          }
        }
      }

      // Change our our {pageHead} variable
      $content = str_replace('{pageHead}',  $this->getHTMLHead(), $content);

      // Change our our {pageFooter} variable
      $content = str_replace('{pageFooter}',  $this->getHTMLFoot(), $content);

      // Open the template file and combine the output
      $templateFile = "{$_SERVER['DOCUMENT_ROOT']}{$this->fMain->configuration->site->relativePath}/Themes/{$this->folderName}/{$template}";
      if ( is_file($templateFile) )
      {
        $page = file_get_contents($templateFile);
        $page = str_replace("{content}", $content, $page);

        fProfile::getInstance()->profile("buildPage", "Page Built");
        return $page;
      } else {
        throw new Exception("Could not open theme template file:" . $templateFile);
      }
    }
  }

  /**
   * Set our current theme template instead of using a theme template
   *
   * @param string Template content
   */
  function setTemplate($templateData)
  {
    $this->template = $templateData;
    $this->variableTemplate = true;
  }

  /**
   * Return an array of the themes available on the site
   *
   * @return array Theme list
   */
  function gefThemeList($advanced = true)
  {
    $themes = array();

    $dir = "{$_SERVER['DOCUMENT_ROOT']}{$this->fMain->configuration->site->relativePath}/Themes/";
    if ( is_dir($dir) )
    {
      $d = opendir($dir);
      while (($file = readdir($d)) !== false)
      {
        if ( is_dir($themeDir = $dir . $file) )
        {
          if ( file_exists($themeDir . '/theme.xml') )
          {
            $xml = simplexml_load_file($themeDir . '/theme.xml');
            if ( is_object($xml) && intval($xml['hide']) == 0 )
            {
              if ( $advanced )
              {
                $themeData = array();
                $themeData['folder']  = $file;
                $themeData['name']    = strval($xml->name);
                $themeData['author']  = array('name' => strval($xml->author), 'email' => strval($xml->author['email']));
                $themeData['preview'] = strval($xml->preview);
                $themeData['url']     = strval($xml->url);
                $themes[] = $themeData;
              } else {
                $themes[$file] = strval($xml->name);
              }
            }
          }
        }
      }
      closedir($d);
    }
    return $themes;
  }

  /**
   * Toggle the use of the theme or not
   *
   * @param bool $bool
   */
  function useTheme($bool = true)
  {
    $this->noTheme = false;
    if ( $bool == false )
    {
      $this->noTheme = true;
    }
  }

  /**
   * Return the HTML Head
   *
   * @return String
   */
  function getHTMLHead()
  {
    // Trim off space from front and back of our htmlHeadContent variable and return the value
    return trim($this->htmlHeadContent);
  }

  /**
   * Return the HTML Footer
   *
   * @return String
   */
  function getHTMLFoot()
  {
    // Trim off space from front and back of our htmlHeadContent variable and return the value
    return trim($this->htmlFooterContent);
  }

  /**
   * Add data to the HTML <head> area
   *
   * @param String $data
   */
  function htmlHead($data)
  {
    if ( ( strlen($data) ) && ( !strstr($this->htmlHeadContent, $data) ) )
    {
      $this->htmlHeadContent .= $data . "\n";
      $this->fDebug->add("Adding htmlHead: " . HTMLEntities($data), 4);
    }
  }

  /**
   * Add data before the HTML </body> tag
   *
   * @param String $data
   */
  function htmlFoot($data)
  {
    if ( ( strlen($data) ) && ( !strstr($this->htmlFooterContent, $data) ) )
    {
      $this->htmlFooterContent .= $data . "\n";
      $this->fDebug->add("Adding htmlFooter: " . HTMLEntities($data), 4);
    }
  }

  /**
   * Set body variable (eg <body $value>)
   *
   * @param String
   */
  function setBodyAttributes($value)
  {
    $this->body = $value;
  }

  /**
   * Return the body variable
   *
   * @return String
   */
  function getBodyAttributes()
  {
    return $this->body;
  }

  /**
   * Add a callback function to be executed in $this->buildPage();
   *
   * @param string $type
   * @param string $class
   * @param string $function
   * @return unknown
   */
  function onBuild($type = 'Singleton', $class = NULL, $function = NULL)
  {
    $count = Count($this->onBuildCallback);
    switch ($type)
    {
      case 'Function':
        if ( is_callable($function) )
        {
          $this->fDebug->add('Adding fThemeCallback: ' . $function, 4);
          $this->onBuildCallback[$count]['type'] = $type;
          $this->onBuildCallback[$count]['function'] = $function;
        }
        break;
      case 'Singleton':
        if ( is_callable(array($class, "getInstance")) )
        {
          $this->fDebug->add('Adding fThemeCallback: ' . $class . '->' . $function, 4);
          $this->onBuildCallback[$count]['type'] = $type;
          $this->onBuildCallback[$count]['class'] = $class;
          $this->onBuildCallback[$count]['function'] = $function;
          return true;
        } else {
          throw new Exception('Invalid Singleton Object Requested: ' . $class);
        }
        break;
      default:
        throw new Exception('Unsupported Callback Type: ' . $type);
    }
    return false;
  }

  /**
   * Transform XML using XSL and return the output
   *
   * @param String XSL Filename
   * @param String/DOMDocument XML Data
   * @return String transformed output
   */
  function transformXML($xslFile, $xmlData)
  {
    $this->fProfile->profile("transformXSL");
    $this->fDebug->add("Attempting to transform XML data using '$xslFile'", 4);

    // If we dont have any XML data trigger an error
    if ( !isset($xmlData) )
    {
      trigger_error("fTheme->transformXSL() attempted to transform XML data with an empty XML document.", E_USER_WARNING);
    }

    // If our $xslFile isn't a file, trigger an error
    if ( !is_file($xslFile) )
    {
      trigger_error("fTheme->transformXSL() attempted to transform XML data with an non-file XSL document.", E_USER_WARNING);
    }

    // Register the XSL Processor
    $proc = new xsltProcessor();

    // Allow it to use PHP functions
    $proc->registerPHPFunctions();

    // Load our XSL File
    $file = new fFile($xslFile);
    $proc->importStyleSheet(DomDocument::loadXML($file->read()));

    // If our XML data is a DOMDocument, just set $xml to a pointer of $xmlData
    if ( $xmlData instanceOf DOMDocument )
    {
      $xml = $xmlData;
    } else {
      // Load the XML string into a DOM document
      $xml = DomDocument::loadXML($xmlData);
    }

    // Transform the XSL
    $output = $proc->transformToXML($xml);

    // If we dont have any output an error occured
    if ( $output == false )
    {
      trigger_error('fTheme->transformXSL() failed the transformation of ' . basename($xslFile), E_USER_WARNING);
    }

    // End our profiling of this transformation
    $this->fProfile->profile("transformXSL", "XML/XSL Transformation");

    // Return the output
    return $output;
  }

  /**
   * Return a valid XSL file or false
   *
   * @param String base name for XSL file
   * @return mixed Valid XSL file or false
   */
  function xslFile($name)
  {
    // Get the class name if an object was passed in
    if ( is_object($name) )
    {
      $name = get_class($name);
    }

    // Build an array of possible locations for the xsl file, including the current theme directory
    $xslLocations = array(rtrim($this->themeDir, '/'), 'Applications', 'Engine', 'Toolkit');

    foreach ( $xslLocations AS $location )
    {
      $result = $this->fileCheck($location, $name . '.xsl');
      if ( $result !== false )
        return $result;
    }

    // We couldn't find the XSL file so trigger an error
    trigger_error('Could not locate the XSL file for the requested name/object: ' . $name, E_USER_WARNING);
    return false;
  }

  /**
   * Check a directory and all of its subdirectories for a file
   *
   * @param string $directory
   * @param string $filename
   * @return string or false
   */
  private function fileCheck($directory, $filename)
  {
    $dir = opendir($directory);
    while ( $entry = readdir($dir) )
    {
      $path = $directory . '/' . $entry;

      if ( is_link($path) )
      {
        $path = readlink($path);
      }

      if ( is_dir($path) && ( $entry != '.' ) && ( $entry != '..' ) )
      {
        if ( is_file($path . '/' . $filename) )
        {
          return $path . '/' . $filename;
        }

        $result = $this->filecheck($path, $filename);

        if ( $result !== false )
        {
          return $result;
        }
      }
    }
    return false;
  }

}

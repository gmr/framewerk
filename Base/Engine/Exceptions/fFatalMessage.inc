<?php 
/**
 * Fatal Message
 *
 * Fatal Error Page Generator
 *
 * @since      2006-01-22
 * @author     Rick Martinez <rick.martinez@gmail.com>
 * @link       http://framewerk.org
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2006-2009 the Framewerk Development Group
 * @package    Engine
 * @subpackage Exceptions
 * @todo	   This fails if XSL in not installed in PHP.  We cant spit out an error even saying XSL doesnt exist.  Need to find a non-xsl alternative or fallback
 */

class fFatalMessage
{
  protected $content, $registry, $simple, $xslpath;
  public $description, $error, $file, $line, $message;

 /**
  * Constructor
  */
  public function __construct()
  {
    // Load the registry or distribution registry
    if ( is_file( 'Configuration/registry.xml' ) )
    {
      $this->registry = @simplexml_load_file('Configuration/registry.xml');
    } 
    elseif (is_file( 'Configuration/registry.xml.dist' ) ) 
    {
      // The installer failed to read the README
      $this->registry = @simplexml_load_file('Configuration/registry.xml.dist');
    }
  }

  /**
   * Kills execution and shows error page
   */
  public function show()
  {
    die($this->content);
  }

  /**
     * Creates error page
     *
     * @param DOMDocument $dom
     * @return string
     */
  protected function createPage($dom)
  {
    $xsl = new DOMDocument('1.0', 'UTF-8');
    $xsl->load('Engine/Exceptions/XSL/ErrorMessage.xsl');
    $xslproc = new xsltProcessor();
    $xslproc->importStyleSheet($xsl);
    $content = $xslproc->transformToXml($dom);

    // get fMain node for baseURI
    $fMain = array_shift($this->registry->xpath('/registry/node[@name="fMain"]'));
    $fDebug = array_shift($this->registry->xpath('/registry/node[@name="fDebug"]'));
    if ( intval($fDebug->level) < 1 )
    {
      $tpl = file_get_contents('Themes/Error/simple.html');
      $tpl = str_replace("{@pageTitle}", $this->error, $tpl);
      $tpl = str_replace("{@baseURI}", $fMain->site->baseURI, $tpl);
      $this->simple = true;
      $style = 'simple.css';
    } else {
      $tpl = file_get_contents('Themes/Error/template.html');
      $tpl = str_replace("{@pageTitle}", "Server Error in '{$fMain->site->name}'", $tpl);
      $tpl = str_replace("{@baseURI}", $fMain->site->baseURI, $tpl);
      $this->simple = false;
      $style = 'style.css';
    }
    $tpl = str_replace("{@pageTitle}", "Server Error in '{$fMain->site->name}'", $tpl);
    $tpl = str_replace("{themeDir}", rtrim($fMain->site->baseURI, '/') . '/Themes/Error/', $tpl);
    $tpl = str_replace("{content}", $content, $tpl);
    $tpl = str_replace("{css}", $style, $tpl);

    $class = new tClassInfo('fMain');
    $tpl = str_replace("{version}", $class->version, $tpl);
    $tpl = str_replace("{date}", strftime('%X %x'), $tpl);
    return $tpl;
  }

  /**
   * Creates DOM Document for use with generating error page
   */
  protected function createDOM()
  {
    echo microtime(true);flush();

    // Create XML for exception
    $dom = new DOMDocument('1.0', 'UTF-8');
    $root = $dom->createElement("exception");
    $root->appendChild( $dom->createElement("error", $this->error) );
    $root->appendChild( $dom->createElement("description", $this->description) );
    $root->appendChild( $dom->createElement("message", $this->message) );
    $root->appendChild( $dom->createElement("file", $this->file) );
    $root->appendChild( $dom->createElement("line", $this->line) );
    $dom->appendChild($dom);
    return $dom;
  }
}
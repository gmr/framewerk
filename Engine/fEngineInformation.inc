<?php // $Id: fEngineInformation.inc 8486 2007-12-20 15:22:22Z jhansche $
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
 * fEngineInformation
 *
 * This file contains the fEngineInformation abstract object
 */
/**
 * Engine Information Abstract Object
 *
 * This abstract object is the base for fDebug, fWarn, and fProfile objects
 *
 * @author     Gavin M. Roy <gavinmroy@gmail.com>
 * @link       http://framewerk.org
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2005-2007 the Framewerk Development Group
 * @version    $Rev: 8486 $
 * @package    Engine
 * @subpackage Messages
 * @abstract
 */

abstract class fEngineInformation
{
  protected $messages = array();
  private $done = false;
  protected $outputMethod;
  
  static protected $stats = array( 'external' => array( 'count' => 0, 'time' => 0.0 ) );

  /**
   * Add a message to the object message stack
   *
   * @param String $message
   */
  function add($message)
  {
    if ( $this->done == true ) return;

    if ( strlen(Trim($message)) )
    {
      $message = array('timestamp'=>time(), 'value'=>$message);
      $this->messages[] = $message;
    }
  }

  /**
   * Outputs formatted content in divs
   *
   * @return String Profiling Output
   */
  abstract function output();

  static function isInstanced()
  {
    return ( self::$instance !== NULL );
  }

  static function debugUserOutput( $return = false, $json = false )
  {
    if ( $return === true )
    {
      $debug = new stdClass();
      $debug->pdo = tPDO::getUsageStats();
      $debug->memcache = fMemcache::getUsageStats();
      $debug->debug = fDebug::getInstance()->outputArray();
      $debug->warn = fWarn::getInstance()->outputArray();
      $debug->profile = fProfile::getInstance()->outputArray();
      $debug->external = self::$stats['external'];
      $debug->system = new stdClass();
      $debug->system->memusage = round( memory_get_peak_usage( true ) / 1048576, 4);
      $debug->system->host = trim(apc_fetch('host'));
      $debug->system->autoload = $GLOBALS['autoload'];
	  $debug->navigationBar = round( NavigationBar::$buildDuration, 3 );
      
      // These are set on each web server via the clearcache script
      $clear_rev = apc_fetch('last_clear_revision');
      $clear_time = apc_fetch('last_clear_time');
      if ( $clear_rev )
      {
        $debug->system->rev = $clear_rev;
      }
      if ( $clear_time )
      {
        $debug->system->time = $clear_time;
      }
      
      if ( $json === false )
      {
        return $debug;
      }
      return json_encode($debug);
    }
    if ( false === fSession::isInstanced() )
    {
      return '';
    }
    $type = fSession::getInstance()->mybData['SESS_USERTYPE'];
    if ( $type != 2 && $type != 3 && substr($_SERVER['REMOTE_ADDR'], 0, 8) !== '192.168.' )
    {
      return '';
    }
    $fTheme = fTheme::getInstance();
    $output = '
  <link rel="stylesheet" href="' . $fTheme->getVariable('CSS_URL') . 'DebugUser.css?' . $fTheme->getVariable('cachebuster') . '" media="screen" />
  <script type="text/javascript" src="' . $fTheme->getVariable('JAVASCRIPT_URL') . 'jQuery-'.$fTheme->getVariable('jQuery_Version').'/Plugins/myYearbook.DebugUser/myYearbook.DebugUser.js?' . $fTheme->getVariable('cachebuster') . '"></script>
  <script type="text/javascript">
    $(function(){jQuery.mybDebug({EngineInfo:json});});
  </script>
';
    return $output;
  }


  /**
   * Return the theme style information from the configuration
   *
   * @param String $type object type
   * @return String style info
   */
  final protected function getThemeStyle($type)
  {
    list($ret) = fTheme::getInstance()->config->XPath("//framewerk/theme/engineinfo/$type");
    if ( !$ret ) return false;
    return $ret;
  }
  
  final public function addStat( $section, $type, $value )
  {
    self::$stats[$section][$type] += round( $value, 4 );
  }
}
?>

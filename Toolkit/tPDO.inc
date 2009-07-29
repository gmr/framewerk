<?php // $Id$
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
 * @author     Joe Hansche <jhansche@myyearbook.com>
 * @link       http://www.framewerk.org/
 * @license    BSD
 * @since      2007-12-10
 * @copyright  Copyright 2007 Framewerk Development Group
 * @version    $Rev$
 * @package    Toolkit
 * @subpackage PDO
 */

class tPDO extends fSingletonObject
{
  protected $registry;
  protected $instances = array();
  protected $notConnected = array();
  protected $defaultInstance;
  protected $currentInstance;

  protected static $instance;
  protected static $stats = array('time' => 0, 'queries' => 0);

  protected $startTimes = array();
  protected static $queries = array();
  protected static $queryTimes = array();

  protected function __construct( )
  {
    parent::__construct();
    $this->registry = new fRegistry($this);
    $this->defaultInstance = $this->registry->default;
    foreach($this->registry->instance as $instance)
    {
      if ( strlen($instance->dsn) == 0 )
      {
        fWarn::getInstance()->add('tPDO instance in registry.xml has no dsn! Ignoring this instance');
        continue;
      }
      // Save this instance by ID
      $this->notConnected[$instance->id] = $instance;
    }
    if ( strlen($this->defaultInstance) == 0 || ( ! isset($this->instances[$this->defaultInstance]) && ! isset($this->notConnected[$this->defaultInstance]) ) )
    {
      $this->fWarn->add('No default tPDO instance, or default instance id is not defined as a tPDO instance in registry.xml!');
    }
  }

  public function connectInstance($instance)
  {
    if ( ($instance instanceOf fRegistryNode) == false )
    {
      if ( isset($this->notConnected[$instance]) )
      {
        $instance = $this->notConnected[$instance];
        $this->instances[$instance->id] = $this->connectInstance($instance);
        return $this->instances[$instance->id];
      } else if ( isset($this->instances[$instance]) )
      {
        return $this->instances[$instance];
      } else {
        return false;
      }
    }

    if ( isset($this->instances[$instance->id]) )
    {
      return $this->instances[$instance->id];
    }

    $this->fProfile->profile('tPDO_connect_' . $instance->id);

    if ( isset($instance->user) )
    {
      if ( isset($instance->password) )
      {
        $tempPDO = new PDO($instance->dsn, $instance->user, $instance->password);
      } else {
        $tempPDO = new PDO($instance->dsn, $instance->user);
      }
    } else {
      $tempPDO = new PDO($instance->dsn);
    }

    // Setup default (non-instance-related) attributes
    foreach($this->registry->attribute as $attribute)
    {
      $tempPDO->setAttribute($attribute->name, $attribute->value);
    }

    // Setup instance-specific attributes
    foreach($instance->attribute as $attribute)
    {
      $tempPDO->setAttribute($attribute->name, $attribute->value);
    }

    // Force PDO to return a tPDOStatement object instead of PDOStatement.
    $tempPDO->setAttribute(PDO::ATTR_STATEMENT_CLASS, array('tPDOStatement' /*XXX: , $ctor_args ? */ ));

    $instanceData = new stdClass();
    $instanceData->object = $tempPDO;
    $instanceData->configuration = $instance;

    $this->fProfile->profile('tPDO_connect_' . $instance->id, 'tPDO: connected instance: ' . $instance->id);
    return $instanceData;
  }

  /**
   * Get or create tPDO instance
   * 
   * @return tPDO
   */
  public static function getInstance( )
  {
    if ( self::$instance === NULL )
    {
      self::$instance = new tPDO();
    }
    return self::$instance;
  }

  public function getUsageStats()
  {
    $stats['queries'] = array_values(self::$queries);
    $stats['totaltime'] = round(array_sum(self::$queryTimes), 3);
    $stats['totalqueries'] = count(self::$queries);
    $stats['avgtpq'] = 0;
    if ( $stats['totalqueries'] > 0 )
    {
      $stats['avgtpq'] = round($stats['totaltime'] / $stats['totalqueries'], 3);
    }
    $stats['maxtime'] = round(max(self::$queryTimes), 3);
    return $stats;
  }

  public function getCurrentInstance()
  {
    if ( $this->currentInstance === NULL )
    {
      $this->currentInstance = $this->defaultInstance;
    }
    return $this->currentInstance;
  }

  public function startQuery($h)
  {
    $this->startTimes[$h] = microtime(true);
    $this->fProfile->profile($h);
  }

  public function endQuery($h, $queryString, $parameters)
  {
    $time = microtime(true) - $this->startTimes[$h];
    unset($this->startTimes[$h]);
    self::$queryTimes[$h] = $time;
    self::$queries[$h] = $queryString; // . '  /* ' . var_export($parameters, true) . ' */';
    $this->fProfile->profile($h, 'SQL Query: ' . $queryString);
  }

  public function setInstance($instance = NULL)
  {
    if ( $instance === NULL )
    {
      $instance = $this->defaultInstance;
      $this->fDebug->add('Setting the tPDO instance back to the default', 1);
    } else {
      $this->fDebug->add('Setting the tPDO instance to "'.$instance.'"', 1);
    }
    if ( ! isset($this->instances[$instance]) )
    {
      if ( isset($this->notConnected[$instance]) )
      {
        if ( ! isset($this->notConnected[$instance]->autoConnect) || $this->notConnected[$instance]->autoConnect == true )
        {
          $this->instances[$instance] = $this->connectInstance($this->notConnected[$instance]);
        } else {
          $this->fWarn->add('Trying to set the current tPDO instance to an instance that has not been connected yet: ' . $instance);
          return false;
        }
      } else
      {
        $this->fWarn->add('Trying to set the current tPDO instance to an undefined instance id: '.$instance.'!');
        return false;
      }
    }
    $this->currentInstance = $instance;
    return true;
  }

  function handleError( $message )
  {
    if ( $this->registry->errorHandling->fWarn === true )
    {
      fWarn::getInstance()->add($message);
    }
    if ( $this->registry->errorHandling->exception === true )
    {
      throw new Exception ( $message );
    }
  }


  public function beginTransaction()
  {
    return $this->connectInstance($this->getCurrentInstance())->object->beginTransaction();
  }

  public function commit()
  {
    return $this->connectInstance($this->getCurrentInstance())->object->commit();
  }

  public function rollback()
  {
    return $this->connectInstance($this->getCurrentInstance())->object->rollback();
  }

  /**
   * Perform a query
   *
   * @param string $query
   * @return tPDOStatement
   */
  public function query($query)
  {
    $this->connectInstance($this->getCurrentInstance());
    try
    {
      $this->startQuery($h = md5($query));
      $statement = $this->instances[$this->currentInstance]->object->query($query);
      $this->endQuery($h, $query);

      if ( ! is_object($statement) )
      {
        return false;
      }

      $statement->setExecuted();
      // Give the tPDOStatement object a reference to this tPDO object.
      $statement->setPDOObject($this);

      if ( isset($this->instances[$this->currentInstance]->configuration->fetchType) )
      {
        $statement->setFetchMode($this->instances[$this->currentInstance]->configuration->fetchType);
      }
    } catch ( PDOException $e )
    {
      $this->endQuery($h, $query);
      $this->handleError( $e->getMessage() );
    }
    return $statement;
  }

  public function errorInfo()
  {
    return $this->connectInstance($this->getCurrentInstance())->object->errorInfo();
  }

  /**
   * Executes a single SQL statement
   *
   * @param String $statement
   * @return int
   */
  public function exec($statement)
  {
    
    try
    {
      $this->startQuery( $h = md5($statement) );
      $result = $this->connectInstance($this->getCurrentInstance())->object->exec($statement);
      $this->endQuery( $h, $statement );
    } catch( PDOException $e )
    {
      $this->endQuery( $h, $statement );
      $this->handleError( $e->getMessage() );
    }
    return $result;
  }

  /**
   * Prepares a statement for execution
   *
   * @param String $statement
   * @param array $options
   * @return tPDOStatement Prepared statement
   */
  public function prepare($statement, $options = array())
  {
    $this->connectInstance($this->getCurrentInstance());
    try
    {
      // Force PDO to return a tPDOStatement object instead of PDOStatement.
      $options[PDO::ATTR_STATEMENT_CLASS] = array('tPDOStatement' /*XXX: , $ctor_args ? */ );

      $statement = $this->instances[$this->currentInstance]->object->prepare($statement, $options);
      // Give the tPDOStatement object a reference to this tPDO object.
      $statement->setPDOObject($this);

      if ( isset($this->instances[$this->currentInstance]->configuration->fetchType) )
      {
        $statement->setFetchMode($this->instances[$this->currentInstance]->configuration->fetchType);
      }
    } catch ( Exception $e )
    {
      throw $e;
    }
    return $statement;
  }
}

class tPDOStatement extends PDOStatement
{
  protected $tPDO;
  protected $executed = false;
  protected $params = '';
  protected $parameters = array();
  static protected $fProfile, $fDebug;

  private function __construct($pdo)
  {
    if ( $this->fProfile === NULL )
    {
      $this->fProfile = fProfile::getInstance();
    }
    if ( $this->fDebug === NULL )
    {
      $this->fDebug = fDebug::getInstance();
    }
  }

  function bindParam($parameter, & $variable, $datatype = NULL, $length = NULL, $options = NULL)
  {
    $this->params .= $variable;
    $this->parameters[$parameter] = $variable;
    if ( $options !== NULL )
    {
      return parent::bindParam($parameter, $variable, $datatype, $length, $options);
    } else if ( $length !== NULL )
    {
      return parent::bindParam($parameter, $variable, $datatype, $length);
    } else if ( $datatype !== NULL )
    {
      return parent::bindParam($parameter, $variable, $datatype);
    }
    return parent::bindParam($parameter, $variable);
  }

  function bindValue($parameter, $value, $datatype = NULL)
  {
    $this->params .= $value;
    $this->parameters[$parameter] = json_encode($value);
    if ( $datatype !== NULL )
    {
      return parent::bindValue($parameter, $value, $datatype);
    }
    return parent::bindValue($parameter, $value);
  }

  function getParameters()
  {
    return $this->parameters;
  }

  function execute($arguments = NULL)
  {
    $return = false;
    try
    {
      if ( $arguments !== NULL )
      {
        $this->params = join('', $arguments);
      }
      $this->tPDO->startQuery($h = md5($this->queryString . $this->params));
      if ( $arguments === NULL )
      {
        $return = parent::execute();
      } else {
        $return = parent::execute($arguments);
      }
      $this->tPDO->endQuery($h, $this->queryString, $this->parameters);

      if ( $return )
      {
        $this->executed = true;
      }
      else
      {
        // This code will only be reached if ERRMODE is not set to throw exceptions for errors
        $e = $this->errorInfo();
        throw new PDOException($e[0] . ': ' . $e[2] . "\n" . 'SQL: ' . $this->queryString);
      }
      return $return;
    } catch ( PDOException $e )
    {
      $this->tPDO->endQuery( $h, $this->queryString, $this->parameters );
      $this->tPDO->handleError( $e->getMessage() );
    }
    return false;
  }

  function rowCount()
  {
    try
    {
      if ( $this->executed === false )
      {
        throw new Exception('rowCount() called on a tPDOStatement that has not yet been executed!');
      }
      return parent::rowCount();
    } catch ( Exception $e )
    {
      $this->tPDO->handleError($e->getMessage());
    }
    return false;
  }

  function fetch($fetch_style = NULL, $cursor_orientation = NULL, $cursor_offset = NULL)
  {
    try
    {
      if ( $this->executed === false )
      {
        throw new Exception('fetch() called on a tPDOStatement that has not yet been executed!');
      }
      if ( $cursor_offset !== NULL )
      {
        return parent::fetch($fetch_style, $cursor_orientation, $cursor_offset);
      } else if ( $cursor_orientation !== NULL )
      {
        return parent::fetch($fetch_style, $cursor_orientation);
      } else if ( $fetch_style !== NULL )
      {
        return parent::fetch($fetch_style);
      } else {
        return parent::fetch();
      }
    } catch ( Exception $e )
    {
      $this->tPDO->handleError($e->getMessage());
    }
  }
  
  function setExecuted( $bool = true )
  {
    $this->executed = true;
  }

  function setPDOObject(tPDO & $tPDO)
  {
    $this->tPDO = & $tPDO;
  }
}

?>

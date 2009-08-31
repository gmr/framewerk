<?php // $Id: tjQueryLiveGrid.inc 914 2007-08-06 17:45:17Z gmr $
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
 * jQuery Live Grid
 *
 * Dynamically builds and updates a jQuery live grid
 *
 * @since      2007-01-30
 * @author     Gavin M. Roy <gavinmroy@gmail.com>
 * @link       http://framewerk.org
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2007 the Framewerk Development Group
 * @package    Toolkit
 * @subpackage jQuery_Widgets
 * @uses       fSingletonObject
 * @version    $Rev: 914 $
 */

class tjQueryLiveGrid extends fObject
{


  private $class, $fields, $providerURL, $rowsPerPage, $sort, $table;

  function __construct($table, $providerURL, $class)
  {
    parent::__construct();

    if ( !isset($table) )
      throw new Exception('tjQueryLiveGrid created without a table.');

    if ( !isset($providerURL) )
      throw new Exception('tjQueryLiveGrid created without a provider URL.');

    $this->class       = $class;
    $this->fields      = array();
    $this->providerURL = $providerURL;
    $this->rowsPerPage = 25;
    $this->table       = $table;
  }

  function addField($field, $label = NULL, $jsOnClickFunction = NULL, $conditionValue = NULL, $conditionType = '=')
  {
    if ( !isset($this->fields[$field]) )
    {
      $this->fields[$field]['label'] = $label;
      $this->fields[$field]['condition']['value'] = $conditionValue;
      $this->fields[$field]['condition']['type'] = $conditionType;
      $this->fields[$field]['onClick'] = $jsOnClickFunction;
    }
  }

  function sortBy($sortString)
  {
    $this->sort = $sortString;
  }

  function buildOutput($mode = 'html', $page = 1)
  {
    // We dont display any data the first time out because the plugin isnt smart enough to replace pre-existing rows
    if ( $mode != 'html' )
    {
      // Build our prepared query
      $sql = 'SELECT ';
      $condition = '';
      $sort = '';
      foreach ( $this->fields AS $field=>$attributes )
      {
        $sql .= $field . ',';
        if ( isset($attributes['condition']['value']) )
        $condition .= $field . $attributes['condition']['type'] . ' :' . $field . ' AND ';
        $sort .= $field . ',';
      }
      $sql = rtrim($sql, ',');
      $condition = rtrim($condition, ' AND ');
      $sql .= ' FROM ' . $this->table;

      // Add our condition string if it exists
      if ( strlen($condition) )
      $sql .= ' WHERE ' . $condition;

      // Add our sort string if it exists, else sort them in the order added
      if ( strlen($this->sort) )
      {
        $sql .= ' ORDER BY ' . $this->sort;
      } else {
        $sql .= ' ORDER BY ' . $sort;
      }

      // Build our offset
      $page--;
      $offset = $page * $this->rowsPerPage;
      $sql .= ' OFFSET ' . $offset . ' LIMIT ' . $this->rowsPerPage;

      // Prepare our query
      $query = $this->PDO->prepare($sql);

      // loop through the fields and bind the parameters
      foreach ( $this->fields AS $field=>$attributes )
      {
        if ( isset($attributes['condition']['value']) )
        {
          $param = ':' . $field;
          $query->bindParam($param, $attributes['condition']['value']);
        }
      }

      // Execute the query
      $query->execute();

      // Make the data into a DOM object
      $data = $query->fetchAll(PDO::FETCH_OBJ);
      $dom = pdo2dom($data, 'liveGridData');

    } else {
      $dom = new DOMDocument('1.0', 'UTF-8');
      $lgd = $dom->createElement('liveGridData');
      $dom->appendChild($lgd);
    }


    // Get our count for our condition
    $sql = "SELECT count(*) FROM " . $this->table;
    if ( strlen($condition) )
      $sql .= ' WHERE ' . $condition;

    // Prepare our query
    $query = $this->PDO->prepare($sql);

    // loop through the fields and bind the parameters
    foreach ( $this->fields AS $field=>$attributes )
    {
      if ( isset($attributes['condition']['value']) )
      {
        $param = ':' . $field;
        $query->bindParam($param, $attributes['condition']['value']);
      }
    }

    // Execute our count query
    $query->execute();
    $data = $query->fetch(PDO::FETCH_OBJ);
    $maxPages = ceil($data->count / $this->rowsPerPage);

    $xpath = new DOMXpath($dom);
    $xdata = $xpath->query('//liveGridData');
    $xdata->item(0)->setAttribute('page', $page);
    $xdata->item(0)->setAttribute('pages', $maxPages);
    $xdata->item(0)->setAttribute('format', $mode);
    if ( strlen($this->class) )
      $xdata->item(0)->setAttribute('class', $this->class);
    $xdata->item(0)->setAttribute('provider', $this->providerURL);
    $xdata->item(0)->setAttribute('table', $this->table);

    // If we're spitting out the main page add header data
    if ( $mode == 'html')
    {
      $columns = $dom->createElement('columns');
      $xdata->item(0)->appendChild($columns);
      $width = '[';
      foreach ( $this->fields AS $field=>$attributes )
      {
        $width .= '150,';
        if ( !isset($attributes['label']) )
          $attributes['label'] = $field;
        $column = $dom->createElement('column');
        $column->appendChild($dom->createTextNode($attributes['label']));
        $columns->appendChild($column);
      }
      $width = rtrim($width, ',') . ']';
    }

    // Add row numbers to rows
    $x = $offset;
    $xdata = $xpath->query('//liveGridData/row');
    for ( $y = 0; $y < $xdata->length; $y++ )
    {
      $x++;
      $xdata->item($y)->setAttribute('rownum', $x);
    }

    // Add the fjQuery Information
    $fjQuery = fjQuery::getInstance();
    $fjQuery->addPlugin('grid');
    $javascript = "$('table#$this->table').grid({height: 350, width: $width, provider: '$this->providerURL'});";
    $fjQuery->addInit($javascript);

    // Get our Theme Object and transform our output
    $TTheme = TTheme::getInstance();
    $xslFile = $TTheme->xslFile($this);
    return html_entity_decode($TTheme->transformXML($xslFile, $dom));
  }

}
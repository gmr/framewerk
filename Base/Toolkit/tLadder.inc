<?php // $Id: tLadder.inc 914 2007-08-06 17:45:17Z gmr $
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
 * tLadder
 *
 * Framewerk Lazy Add Delete and Replace object for constructing administration
 * backends "automagically"
 */
/**
 * tLadder Object
 *
 * Framewerk Lazy Add Delete and Replace object for constructing administration
 * backends "automagically"
 *
 * @author     Gavin M. Roy <gavinmroy@gmail.com>
 * @link       http://framewerk.org
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @since      2005-12-22
 * @copyright  Copyright 2005-2007 the Framewerk Development Group
 * @version    $Rev: 914 $
 * @package    Toolkit
 * @subpackage Admin
 * @todo       Look at how to make it non-pgsql specific and ajaxy
 */

class tLadder extends fObject
{
  public $baseURI, $database, $filestore, $keyword, $meta, $name, $noun,  $orderBy, $pdo, $schema, $table, $uriValues, $xsl;
  public $allowAdd = true, $allowEdit = true, $allowDelete = true;
  public $onModify;
  private $dom, $form;

  /**
   * Constructor
   *
   * @param tPDO Object
   * @param String $schemaName
   * @param String $tableName
   */
  function __construct($database, $schema, $table)
  {
    parent::__construct();

    $this->baseURI   = $this->fMain->configuration->site->baseURI . $this->fMain->keyword;
    $this->database  = $database;
    $this->filestore = $table;
    $this->keyword   = $table;
    $this->meta      = array();
    $this->name      = $table;
    $this->noun      = 'Record';
    $this->orderBy   = false;
    $this->pdo       = tPDO::getInstance();
    $this->schema    = $schema;
    $this->table     = $table;
    $this->uriValues = fSession::getInstance()->uriValues;
    $this->xsl       = 'tLadder';

    $this->pdo->setInstance($this->database);

    // Get the primary key fields
    $sql = "SELECT f.attnum AS number, f.attname AS name, f.attnum, f.attnotnull AS notnull, pg_catalog.format_type(f.atttypid,f.atttypmod) AS type, CASE WHEN p.contype = 'p' THEN 't' ELSE 'f' END AS primarykey, CASE WHEN p.contype = 'u' THEN 't' ELSE 'f' END AS uniquekey, CASE WHEN ind.indisunique IS TRUE THEN 't' ELSE 'f' END AS unique_index, CASE WHEN p.contype = 'f' THEN gs.nspname END AS fk_schema, CASE WHEN p.contype = 'f' THEN g.relname  END AS fk_table, CASE WHEN p.contype = 'f' THEN p.confkey  END AS fk_fieldnum,  CASE WHEN f.atthasdef = 't' THEN d.adsrc END AS default FROM pg_attribute f JOIN pg_class c ON c.oid = f.attrelid JOIN pg_type t ON t.oid = f.atttypid LEFT JOIN pg_attrdef d ON d.adrelid = c.oid AND d.adnum = f.attnum LEFT JOIN pg_namespace n ON n.oid = c.relnamespace LEFT JOIN pg_constraint p ON p.conrelid = c.oid AND f.attnum = ANY ( p.conkey ) LEFT JOIN pg_class AS g ON p.confrelid = g.oid LEFT JOIN pg_namespace gs ON gs.oid = g.relnamespace LEFT JOIN pg_catalog.pg_index ind ON (ind.indrelid = c.oid AND f.attnum = ANY(ind.indkey) AND ind.indisunique AND ind.indisvalid) WHERE c.relkind = 'r'::char AND n.nspname = :schema AND c.relname = :table AND f.attnum > 0 ORDER BY number;";
    $query = $this->pdo->prepare($sql);
    $query->bindParam(':schema', $this->schema);
    $query->bindParam(':table', $this->table);
    $query->execute();
    
    // Throw an exception if we didn't get a result from the query
    if ( $query->rowCount() == 0 )
      Throw new Exception('Attempt to create tLadder object with invalid table name');

    // Loop through the columns and setup our header info
    $columns = $query->fetchAll(pdo::FETCH_OBJ);
    foreach ( $columns AS $column )
    {
      // Quick hack because our query seems to be returning two rows for one column if it's a foreign and primary key
      foreach ( $this->meta AS $row )
      {
        if ( $row['name'] == $column->name )
        {
          $meta = $row;
          break;
        }
      }

      // Split up the columns data type and precision
      $type = explode('(', $column->type);

      // Database Meta Options
      $meta['name']       = strval($column->name);
      $meta['label']      = ucwords($column->name);
      $meta['number']     = intval($column->number);
      $meta['notnull']    = intval($column->notnull);
      $meta['type']       = strval($type[0]);
      $meta['value']      = strval($column->default);
      $meta['precision']  = strlen($type[1]) ? rtrim($type[1], ')') : NULL;
      $meta['primarykey'] = ($column->primarykey == 't') ? true : false;
      $meta['uniquekey']  = ($column->uniquekey == 't') ? true : false;
      $meta['unique_index']  = ($column->unique_index == 't') ? true : false;

      // Get more information on the default value
      if ( strlen($meta['value']) )
      {
        if ( preg_match("/^([a-z]+)\((|.+)\)\$/i", $meta['value'], $params) )
        {
          $meta['default']['original'] = $params[0];
          $meta['default']['expression'] = $params[1];
          $meta['default']['expval'] = $params[2];
          $meta['value'] = '';
        } else if ( preg_match("/^\'(.*)\'::(.+)\$/", $meta['value'], $params) ) {
          $meta['value'] = $params[1];
          $meta['default']['type'] = $params[2];
        }
      }

      // General Meta Options
      $meta['display']     = false;
      $meta['displayOnly'] = false;
      $meta['isFile']      = false;
      $meta['skip']        = false;
      $meta['valid']       = '';

      // Callback Meta Options
      $meta['preSave']   = array();
      $meta['preEdit']   = array();
      $meta['preModify'] = array();

      // Check to see if there is a foreignkey for this field
      if ( !isset($meta['fk']) )
      {
        if ( strlen($column->fk_table) )
        {
          $meta['fk']['schema'] = $column->fk_schema;        
          $meta['fk']['table'] = $column->fk_table;
          $meta['fk']['data'] = array();

          // If it's a compound foreign key make sure we match up our field number to our connection number
          if ( strstr($column->kn_fieldnum, ','))
          {
            $temp = explode(',', str_replace('{', '', str_replace('}', '', $column->fk_fieldnum)));
            $pos = array_search($column->attnum, $conn);
            $meta['fk']['fieldNumber'] = $temp[$pos];
          } else {
            $meta['fk']['fieldNumber'] = str_replace('{', '', str_replace('}', '', $column->fk_fieldnum));
          }
        }
      }

      $this->meta[$column->name] = $meta;
      unset($meta);
    }
  }

  /**
   * buildOutput
   * Main function to build and return the output of tLadder
   *
   * @return string
   */
  function buildOutput()
  {
    // Default form title
    $title = 'Add ';
  
    // Reset our PDO connection to our database  
    $this->pdo->setInstance($this->database);
  
    // Get our actions and values
  
    $uriValues = $this->uriValues;
    if ( count($uriValues) )
    {
      $action = $uriValues[0];
      if ( count($uriValues) > 1 )
      {
        array_shift($uriValues);
        $values = array();
        foreach ( $uriValues AS $value )
          $values[] = RawURLDecode($value);
      }
    }

    // Check if we found any foreign keys and get the data from the foreign key field
    foreach( $this->meta as $key => $meta )
    {
      if ( isset($meta['fk']['table']) )
      {
        $sql = "SELECT f.attname AS name FROM pg_attribute f JOIN pg_class c ON c.oid = f.attrelid JOIN pg_type t ON t.oid = f.atttypid LEFT JOIN pg_namespace n ON n.oid = c.relnamespace  WHERE c.relkind = 'r'::char AND n.nspname = '" . $meta['fk']['schema'] . "' AND c.relname = '" . $meta['fk']['table'] . "' AND f.attnum > 0 AND f.attnum = " . $meta['fk']['fieldNumber'];
        $query = $this->pdo->query($sql);
        $fk = $query->fetch(pdo::FETCH_OBJ);
        $this->meta[$key]['fk']['field'] = $fk->name;
        $name = $fk->name;
        $displayName = $this->meta[$key]['fk']['displayField'];

        if ( strlen($displayName) )
          $sql = 'SELECT ' . $name . ', ' . $displayName . ' FROM ' . $meta['fk']['schema'] . '.' . $meta['fk']['table'] . ' ORDER BY ' . $displayName;
        else
          $sql = 'SELECT ' . $name . ' FROM ' .  $meta['fk']['schema'] . '.' . $meta['fk']['table'] . ' ORDER BY ' . $name;
          
        $query = $this->pdo->query($sql);

        if ( is_object($query) )
        {
          $data = $query->fetchAll(pdo::FETCH_OBJ);
          $keyData = array();
          foreach ( $data AS $row )
          {
            if ( strlen($displayName) )
            $keyData[$row->$name] = $row->$displayName;
            else
            $keyData[$row->$name] = $row->$name;
          }
          $this->meta[$key]['fk']['data'] = $keyData;
        }
      }
    }

    // Instance our tForm Object
    $this->form = new tForm('post', $this->baseURI . '/save/');

    // Build tForm fields
    $this->buildForm();

    // Act on our action
    switch ( $action )
    {
      case 'delete':
        if ( ! $this->allowDelete )
        {
          fMessages::getInstance()->add( new fErrorMessage( 'Deleting records is disabled!') );
          break;
        }
        $x = 0;
        $sql = 'DELETE FROM ' . $this->schema . '.' . $this->table . ' WHERE ';
        foreach ( $this->meta AS $meta )
        {
          if ( $meta['primarykey'] )
          {
            $sql .= $meta['name'] . " = '" . $values[$x] . "' AND ";
            $x++;
          }
        }
        $sql = rtrim($sql, ' AND ');
        $query = $this->pdo->query($sql);
        if ( $query )
          fMessages::getInstance()->add(new fSuccessMessage($this->noun . ' deleted from the table.'));
        else
          fMessages::getInstance()->add(new fSuccessMessage('Error deleting ' . $this->noun . ' from the table.'));
        break;

      case 'edit':
        if ( ! $this->allowEdit )
        {
          fMessages::getInstance()->add( new fErrorMessage( 'Editing records is disabled!') );
          break;
        }
        $title = 'Edit';
        
        $x = 0;
        $sql = 'SELECT * FROM ' . $this->schema . '.' . $this->table . ' WHERE ';
        foreach ( $this->meta AS $meta )
        {
          if ( $meta['primarykey'] )
          {
            $sql .= $meta['name'] . ' = \'' . $values[$x] . '\' AND ';
            $x++;
          }
        }
        $sql = rtrim($sql, ' AND ');
      
        $query = $this->pdo->query($sql);

        if ( !$query )
        {
          $error = $this->pdo->errorInfo();
          throw new Exception('Could not fetch required information: ' . $error[2] . ' with query: ' . $sql);
        }
        
        $result = $query->fetch(pdo::FETCH_OBJ);
        foreach ( $result AS $col => $val )
        {
          if ( $this->meta[$col]['type'] == 'boolean' )
          {
            $this->meta[$col]['value'] = intval($val) ? 'true' : 'false';
          } else {
            $this->meta[$col]['value'] = $val;
          }
        }
        // Reset the form object because we want to recreate the fields with POST values, this hack should be cleaned up at some point
        // Instance our tForm Object
        $this->form->removeAll();
        $this->buildForm();
        break;

      case 'save':
        // Make sure our form has been submitted and validate before we continue
        if ( $this->form->submitted() && $this->form->validate() )
        {

          // Determine if it's an add or update
          $insert = true;
          foreach ( $this->meta AS $column )
          {
            if ( $column['primarykey'] && strlen($_POST['old_' . $column['name']]) )
            $insert = false;

            if ( ( $column['fk']['table'] == 'tfilestorage' ) && ( $column['fk']['field'] == 'id' ) )
            {
              if ( is_uploaded_file($_FILES[$column['name']]['tmp_name']) )
              {
                $file = new fFileStorage($this->filestore);
                $_POST[$column['name']] = $file->save($_FILES[$column['name']]['tmp_name'],$_FILES[$column['name']]['name'], $_FILES[$column['name']]['type'], $this->fMain->fSession->fUser->uid, NULL);
                if ( !$_POST[$column['name']] )
                {
                  trigger_error('Could not save the uploaded file', E_USER_ERROR);
                }
                unset($file);
              } else {
                $_POST[$column['name']] = null;
              }
            }
          }
          
          if ( $insert && ! $this->allowAdd )
          {
            fMessages::getInstance()->add( new fErrorMessage( 'Adding new records is disabled!') );
            break;
          }
          if ( ! $insert && ! $this->allowEdit )
          {
            fMessages::getInstance()->add( new fErrorMessage( 'Editing records is disabled!') );
            break;
          }

          // Build our queries
          if ( $insert )
          {
            $fieldNames  = '';
            $fieldValues = '';
            foreach ( $this->meta AS $column )
            {
              if ( ( !$column['skip'] ) &&
              ( ( strlen($_POST[$column['name']]) ) ||
              ( ( $column['fk']['table'] == 'tfilestorage' ) &&
              ( $column['fk']['field'] == 'id' ) ) ) )
              {
                $fieldNames  .= $column['name'] . ', ';
                $fieldValues .= ':' . $column['name'] . ', ';
              }
            }
            $fieldNames  = rtrim($fieldNames, ', ');
            $fieldValues = rtrim($fieldValues, ', ');
            $sql = 'INSERT INTO ' . $this->schema . '.' . $this->table . ' (' . $fieldNames . ') VALUES (' . $fieldValues . ');';
          } else {
            // Get the row that we're saving, so we can compare posted and existing values, and only update what has changed
            $sql = 'SELECT * FROM ' . $this->schema . '.' . $this->table . ' WHERE ';
            foreach ( $this->meta AS $meta )
            {
              if ( $meta['primarykey'] )
              {
                $sql .= $meta['name'] . ' = \'' . $_POST['old_' . $meta['name']] . '\' AND ';
              }
            }
            $sql = rtrim($sql, ' AND ');
          
            $query = $this->pdo->query($sql);
    
            if ( !$query )
            {
              $error = $this->pdo->errorInfo();
              throw new Exception('Could not fetch required information: ' . $error[2] . ' with query: ' . $sql);
            }
            
            $existing = $query->fetch(pdo::FETCH_ASSOC);
            $touchedColumn = array( );

            // Now to build the UPDATE query, changing only the fields that are different
            $sql = 'UPDATE ' . $this->schema . '.' . $this->table . ' SET ';
            foreach ( $this->meta AS $column )
            {
              if ( ( !$column['skip'] ) &&
                   ( ( strlen($_POST[$column['name']]) && ( ! $column['primarykey'] || $_POST[$column['name']] != $_POST['old_' . $column['name']] ) ) || 
                     ( ( $column['fk']['table'] == 'tfilestorage' ) &&
                       ( $column['fk']['field'] == 'id' ) &&
                       ( $_POST[$column['name']] != null ) ) ) )
              {
                //XXX: Need to make this more elegant...
                if ( $column['type'] == 'boolean' && $existing[$column['name']] !== NULL )
                {
                  $existing[$column['name']] = ($existing[$column['name']] === true ? 'Yes' : 'No');
                }
                if ( $existing[$column['name']] != $_POST[$column['name']] )
                {
                  $touchedColumn[$column['name']] = true;
                  $sql .= $column['name'] . ' = :' . $column['name'] . ', ';
                }
              }
            }
            if ( count( $touchedColumn ) == 0 )
            {
              fMessages::getInstance()->add(new fErrorMessage('Error saving the ' . $this->noun . ' in the table, because nothing changed!'));
              break;
            }
            $sql = rtrim($sql, ', ') . ' WHERE ';
            foreach ( $this->meta AS $column )
            {
              if ( $column['primarykey'] )
              {
                $sql .= $column['name'] . ' = :old_' . $column['name'] . ' AND ';
              }
            }
            $sql = rtrim($sql, 'AND ');
          }

          // Prepare our query
          $query = $this->pdo->prepare($sql);

          // Bind columns
          foreach ( $this->meta AS $column )
          {
            // Bind our where clause columns on update
            if ( !$insert && $column['primarykey'] )
            {
              $query->bindParam((':old_' . $column['name']), $_POST['old_' . $column['name']]);
            }

            // For updates, ignore anything that wasn't changed
            if ( ! $insert && ! isset( $touchedColumn[$column['name']] ) )
            {
              continue;
            }

            // Bind our data columns
            if ( $column['skip'] != true )
            {
              // Make sure we unset our reused variable names
              unset($field, $fieldValue);

              $field  = ':' . $column['name'];

              // Determine if this is a regular post value or a File Upload Field (not including tfilestorage)
              // All callback functions are required to return a string of the value to insert
              $fieldValue = $_POST[$column['name']];
              
              // On any modification call the onModify
              if ( is_callable($column['onModify']) )
              {
                $fieldValue = call_user_func($column['onModify'], $fieldValue);
              }

              // On an Insert call onSave if it's set
              if ( $insert && is_callable($column['onSave']) )
              {
                $fieldValue = call_user_func($column['onSave'], $fieldValue);
              }

              // On an Update call onEdit if it's set
              if ( !$insert && is_callable($column['onEdit']) )
              {
                $fieldValue = call_user_func($column['onEdit'], $fieldValue);
              }

              //translate values for special column types
              switch($column['type'])
              {
                case 'boolean':
                  if ( $fieldValue == 'on' )
                  {
                    $fieldValue = 't';
                  }
                case 'smallint':
                case 'integer':
                case 'numeric':
                case 'date':
                  //set column value to null if appropriate
                  if ( $fieldValue == '' )
                  {
                    $fieldValue = null;
                  }
                  break;
                default:
              }

              // Of our column is a manually set file column then allow us to get the base64_endoded file and set it's value
              if ( ( $column['isFile'] == true ) && ( is_uploaded_file($_FILES[$column['name']]['tmp_name']) ) )
              {
                $fieldValue = base64_encode(file_get_contents($_FILES[$column['name']]['tmp_name']));
              }
              if ( strlen($fieldValue) && ( ! $column['primarykey'] || $_POST[$column['name']] != $_POST['old_' . $column['name']] ) )
              {
                $query->bindParam($field, $fieldValue);
              }
            }
          }

          // Execute our query
          if ( $query->execute() )
          {
            fMessages::getInstance()->add(new fSuccessMessage($this->noun . ' saved to the table.'));

            // Clear our form values so the form resets after post            
            $this->form->clearValues();

            if ( is_callable( $this->onModify ) )
            {
              //TODO: quick hack to just get the data; should be changed to pass row's primary key fields, and any touched columns
              call_user_func( $this->onModify, $_POST );
            }
          } else {
            fDebug::getInstance()->add(print_r($this->pdo->errorInfo(), true), 4);
            fMessages::getInstance()->add(new fSuccessMessage('Error saving the ' . $this->noun . ' in the table.'));
          }
        }
        break;
      default:
        if ( strlen($action) )
        {
          fMessages::getInstance()->add(new fErrorMessage("Invalid action: $action"));
        }
    }

    // Build our output
    $this->dom = new DOMDocument();
    $this->dom->formatOutput = true;
    $node = appendDomNode($this->dom, 'tLadder', array('baseURI' => $this->baseURI, 'title' => $title, 'name' => $this->name, 'keyword' => $this->keyword, 'noun' => $this->noun, 'allowAdd' => $this->allowAdd, 'allowEdit' => $this->allowEdit, 'allowDelete' => $this->allowDelete));
    $node = appendDomNode($this->dom, $node, NULL, appendDomNode($this->dom, 'tForm', NULL, NULL, $this->form->buildOutput()));
    $node = appendDomNode($this->dom, $node, NULL, $this->buildTable());
    $node = $this->dom->appendChild($node);

    $xsl    = fTheme::getInstance()->xslFile($this->xsl);
    $output = html_entity_decode(fTheme::getInstance()->transformXML($xsl, $this->dom->saveXML()));

    return $output;
  }

  /**
   * Internal function to build the add/edit form
   *
   * @return string output
   */
  private function buildForm()
  {
    foreach ( $this->meta AS $name=>$column )
    {
      if ( $column['primarykey'] && strlen($column['value']) )
      $this->form->add(new tHiddenField('old_' . $column['name'], NULL, $column['value']));

      // Create our field if we havn't been told to skip
      if ( !$column['skip'] )
      {
        $attributes = array('maxlength' => $column['precision']);
        if ( ( $column['primarykey']) || ( $column['notnull']) )
          $attributes['required'] = true;
        if ( strlen($column['valid']) )
          $attributes['valid'] = $column['valid'];

        // If we have a foreign key,  create a select field unless its a tfilestorage foreignkey we'll create a fileupload field.
        if ( is_array($column['fk']) )
        {
          if ( ( $column['fk']['table'] == 'tfilestorage' ) && ( $column['fk']['field'] == 'id' ) )
          {
            // Hack until this gets put back into tForm
            $this->form->setEnctype('multipart/form-data');
            $this->form->add(new tFileUploadField($column['name'], $column['label'], $column['value'], array_merge($attributes)));
          } else {
            $attributes['arrayKey'] = true;
            $attributes['selected']  = $column['value'];
            $this->form->add(new tSelectField($column['name'], $column['label'], $column['fk']['data'], $attributes));
          }
        } else {
          // If there was no foreign key create a form field based on the data type.
          switch ( $column['type'] )
          {
            case 'smallint':
            case 'integer':
              $this->form->add(new tTextField($column['name'], $column['label'], $column['value'], array_merge(array('size'=>5), $attributes)));
              break;
            case 'numeric':
            case 'character':
            case 'character varying':
              $this->form->add(new tTextField($column['name'], $column['label'], $column['value'], $attributes));
              break;
            case 'date':
              $this->form->add(new tDateField($column['name'], $column['label'], $column['value'], $attributes));
              break;
            case 'text':
              if ( $column['isFile'] )
              {
                // Shouldn't always require this... need to figure out how to require it if it's needed, but not on an update.
                $attributes['required'] = false;
                $this->form->add(new tFileUploadField($column['name'], $column['label'], '', $attributes));
              } else {
                $this->form->add(new tTextareaField($column['name'], $column['label'], $column['value'], $attributes));
              }
              break;
            case 'boolean':
              switch( $column['displayType'] )
              {
                case 'checkbox':
                  $this->form->add(new tCheckField($column['name'], $column['label'], $column['value']));
                  break;
                default:
                  if ( strval($column['value']) == 'false' )
                  {
                    $attributes['selected'] = 'No';
                  } else if ( strval($column['value']) == 'true' ) {
                    $attributes['selected'] = 'Yes';
                  }
                  $this->form->add(new tSelectField($column['name'], $column['label'], array('Yes', 'No'), $attributes));
              }
              break;
            default:
              $this->form->add(new tTextField($column['name'], $column['label'], $column['value'], $attributes));
          }
        }
      }
    }
    
    if ( $this->uriValues[0] == 'edit' )
    {
      if ( $this->allowEdit )
      {
        $this->form->add(new tSubmitField('Update'));
      }
      else
      {
        $this->form->add( new tDisplayField( '', 'Editing records is disabled' ) );
      }
    }
    else
    {
      if ( $this->allowAdd )
      {
        $this->form->add(new tSubmitField('Add'));
      }
      else
      {
        $this->form->add( new tDisplayField( '', 'Adding new records is disabled' ) );
      }
    }
    
    return true;
  }

  /**
   * Internal function to build the content table
   *
   * @return string output
   */
  private function buildTable()
  {
    $table = $this->dom->createElement('table');

    $clause = '';
    $orderby = '';
    $select = '';
    $fk = false;
    foreach ( $this->meta AS $column )
    {
      $select .= $this->schema . '.' . $this->table . '.' . $column['name'] . ', ';
      if ( isset( $column['fk'] ) ) 
      {
        if ( isset($column['fk']['displayField']) )
          $select .= $column['fk']['schema'] . '.' . $column['fk']['table'] . '.' . $column['fk']['displayField'] . ' AS fk_' . $column['fk']['table'] . '_' . $column['fk']['displayField'] . ', ';
        else
          $select .= $column['fk']['schema'] . '.' . $column['fk']['table'] . '.' . $column['fk']['field'] . ', ';
        
        $clause .= ' JOIN ' . $column['fk']['schema'] . '.' . $column['fk']['table'] . ' ON ' . $this->schema . '.' . $this->table . '.' . $column['name'] . ' = ' . $column['fk']['schema'] . '.' . $column['fk']['table'] . '.' . $column['fk']['field'];
        $orderby .= $column['fk']['schema'] . '.' . $column['fk']['table'] . '.' . $column['fk']['field'] . ', ';
        $fk = true;
      } else {
        $orderby .= $this->schema . '.' . $this->table . '.' . $column['name'] . ', ';
      }
      
    }
    $orderby = rtrim($orderby, ', ');

    if ( $this->orderBy != false )
      $orderby = $this->orderBy;

    if ( $fk == true )
    {
      $select = rtrim($select, ', ');
      $sql = 'SELECT ' . $select . ' FROM ' . $this->schema . '.' . $this->table . $clause . ' ORDER BY ' . $orderby;
    } else {
      $sql = 'SELECT * FROM ' . $this->schema . '.' . $this->table . ' ORDER BY ' . $orderby . ';';
    }

    $query = $this->pdo->query($sql);
    
    if ( !$query )
    {
      $error = $this->pdo->errorInfo();
      throw new Exception($error[2] . ' - Query: ' . $sql);
    }
    
    $rows = $query->fetchAll(pdo::FETCH_OBJ);

    $headers = $this->dom->createElement('headers');
    foreach ( $this->meta AS $column )
    {
      if ( ($column['primarykey'] || $column['display']) && !$column['skip'] )
      {
        $header = appendDomNode($this->dom, $headers, NULL, appendDomNode($this->dom, 'header', NULL, NULL, ucwords($column['label'])));
      }
    }

    $domRows = $this->dom->createElement('rows');
    foreach ( $rows AS $row )
    {
      // Build our key for the delete and edit links
      $key = '';
      foreach ( $this->meta AS $column )
      {
        if ( $column['primarykey'])
        {
          $key .= rawUrlEncode($row->$column['name']) . '/';
        }
      }
      $key = rtrim($key, '/');
      

      // Display our output
      $domRow = appendDomNode($this->dom, 'row', array('key' => $key));
      foreach ( $this->meta AS $column )
      {
        if ( ($column['primarykey'] || $column['display']) && !$column['skip'] )
        {
          if ( !isset($column['fk']) )
            appendDomNode($this->dom, $domRow, NULL, appendDomNode($this->dom, 'column', array('displayOnly' => intval($column['displayOnly'])), NULL, $row->$column['name']));
          elseif ( !isset($column['fk']['displayField']) )
            appendDomNode($this->dom, $domRow, NULL, appendDomNode($this->dom, 'column', array('displayOnly' => intval($column['displayOnly'])), NULL, $row->$column['fk']['field']));
          else
            appendDomNode($this->dom, $domRow, NULL, appendDomNode($this->dom, 'column', array('displayOnly' => intval($column['displayOnly'])), NULL, $row->{'fk_' . $column['fk']['table'] . '_' . $column['fk']['displayField']}));
        }
      }
      $domRows = appendDomNode($this->dom, $domRows, NULL, $domRow);
    }

    $table = appendDomNode($this->dom, $table, NULL, $headers);
    $table = appendDomNode($this->dom, $table, NULL, $domRows);
    return $table;
  }
}
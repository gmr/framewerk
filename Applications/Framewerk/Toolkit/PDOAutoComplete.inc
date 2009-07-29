<?php // $Id: PDOAutoComplete.inc 919 2007-08-06 18:34:41Z dallas $
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
   * TAutoCompleteXML XML Interface
   *
   * Queries a table hashed with a security string and returns data matching the given data
   *
   * @since      2006-10-30
   * @author     Fred Ghosn <fredghosn@gmail.com>
   * @version    $Rev: 919 $
   * @license    http://opensource.org/licenses/bsd-license.php BSD License
   * @copyright  Copyright 2006-2007 the Framewerk Development Group
   * @uses       fApplication
   * @package    Applications
   * @subpackage Helpers
   */

  class TPDOAutoCompleteXML extends fXMLApplication
  {
    public function execute()
    {
      $dom = new DOMDocument();
      $dom->formatOutput = true;
      $node = $dom->createElement('ajaxresponse');

      if ( strlen($this->uriValues[0]) != 32 || strlen($this->uriValues[1]) != 32 || !strlen($_POST['value']) )
        return $this->output($dom->saveXML($node));

      $registry = new fRegistry($this);

      $query = $this->PDO->prepare('SELECT t.tablename as table, f.attname as field FROM pg_tables t JOIN pg_class c ON c.relname = t.tablename JOIN pg_attribute f ON c.oid = f.attrelid WHERE relkind = \'r\' and f.attnum > 0 AND t.schemaname = \'public\' and md5(f.attname || ?) = ? AND md5(t.tablename || ?) = ?;');
      $query->bindParam(1, $registry->pdoObfuscator);
      $query->bindParam(2, $this->uriValues[1]);
      $query->bindParam(3, $registry->pdoObfuscator);
      $query->bindParam(4, $this->uriValues[0]);
      $query->execute();

      if ( !$tableData = $query->fetch(PDO::FETCH_OBJ) )
        return $this->output($dom->saveXML($node));

      $query = $this->PDO->prepare('SELECT ' . $tableData->field . ' FROM ' . $tableData->table . ' WHERE ' . $tableData->field . ' LIKE \'%\' || :search || \'%\'');
      $query->bindParam(':search', $_POST['value']);
      $query->execute();

      foreach( $query->fetchAll(PDO::FETCH_OBJ) as $row )
      {
        $item = $dom->createElement('item');
        $item->appendChild($dom->createElement('text', $row->{$tableData->field}));
        $item->appendChild($dom->createElement('value', $row->{$tableData->field}));
        $node->appendChild($item);
      }

      $this->output($dom->saveXML($node));
      return true;
    }
  }
?>

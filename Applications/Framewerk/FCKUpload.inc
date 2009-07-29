<?php // $Id: FCKUpload.inc 919 2007-08-06 18:34:41Z dallas $
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
   * Sends a file that's stored via fFileStorage
   *
   * Sends out the data stored via fFileStorage
   *
   * @since      2006-02-02
   * @author     Gavin M. Roy <gavinmroy@gmail.com>
   * @link       http://framewerk.org
   * @license    http://opensource.org/licenses/bsd-license.php BSD License
   * @copyright  Copyright 2006-2007 the Framewerk Development Group
   * @package    Applications
   * @subpackage Base
   * @uses       fApplication
   * @version    $Rev: 919 $
   */

  class FCKUpload extends fApplication
  {
    function execute()
    {
      $this->fTheme->useTheme(false);

      if ( is_uploaded_file($_FILES['NewFile']['tmp_name']))
      {
        $storage = new fFileStorage($this);
        $id = $storage->save($_FILES['NewFile']['tmp_name'], $_FILES['NewFile']['type'], $this->fSession->uid, 'All');
        $type = explode('/', $_FILES['NewFile']['type']);
        $type = $type[1];
        $output = <<<OUTPUT
<script type="text/javascript">
  window.parent.OnUploadCompleted(0, '/download/FCKUpload/$id.$type');
</script>
OUTPUT;
      } else {
        $output = <<<OUTPUT
<script type="text/javascript">
  window.parent.OnUploadCompleted(101, 'Invalid uploaded file.');
</script>
OUTPUT;
      }
      $this->output($output);
      return true;
    }
  }
?>

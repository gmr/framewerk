<?php // $Id: LanguageSelector.inc 881 2007-06-14 01:51:06Z gmr $
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
   * Language Selection Tool
   *
   * Sets users language and returns them to the referring page.
   *
   * @author     Fred Ghosn <fredghosn@gmail.com>
   * @link       http://framewerk.org
   * @license    http://opensource.org/licenses/bsd-license.php BSD License
   * @copyright  Copyright 2005-2007 the Framewerk Development Group
   * @package    Applications
   * @subpackage Base
   * @uses       fApplications
   * @version    $Rev 1$
   */

class LanguageSelector extends fApplication
{
  public function execute()
  {
    $this->ignoreURIUnderscore();

    if ( $this->fSession->validLanguage($this->uriValues[0]) )
    {
      $this->fSession->language = $this->uriValues[0];
      $this->fMessages->add(new fSuccessMessage('Language Updated'));
    } else {
      $this->fMessages->add(new fErrorMessage('Language Error'));
    }

    return $this->returnToReferrer();
  }
}
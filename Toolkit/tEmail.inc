<?php // $Id: tEmail.inc 914 2007-08-06 17:45:17Z gmr $
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
 * Email Class Source
 */
/**
 * Class to send email with attachements / html email
 *
 * @author     Gavin M. Roy <gavinmroy@gmail.com>
 * @link       http://framewerk.org
 * @since      2006-03-26
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2006-2007 the Framewerk Development Group
 * @version    $Rev: 914 $
 * @package    Toolkit
 * @subpackage Email
 * @uses       fObject
 */

class tEmail extends fObject
{
  // Recipient types
  const RECIPIENT_TO = 0x01;
  const RECIPIENT_CC = 0x02;
  const RECIPIENT_BCC = 0x04;
  // Content types
  const CONTENT_TEXT = 0x10;
  const CONTENT_HTML = 0x20;
  const CONTENT_XHTML = 0x40;
  // Newline
  const NEWLINE = "\n";

  protected $attachments = array(), $content = array(), $email, $from, $replyto, $sender;
  protected $header, $message, $recipientString = '', $subject, $toString = '';

  function __construct($sender = NULL)
  {
    parent::__construct();

    if ( isset($sender) )
    {
      if (  strpos($sender, '@') === false )
      {
        // If not an email address, use "$sender" <//site/contact>
        $this->sender = sprintf('"%s" <%s>', $sender, $this->fMain->configuration->site->contact);
      } elseif ( strpos($sender, '"') === false ) {
        // If not a full name+email, use "//site/name" <$sender>
        $this->sender = sprintf('"%s" <%s>', $this->fMain->configuration->site->name, $sender);
      } else {
        // Otherwise, use whatever is given
        $this->sender = $sender;
      }
    } else {
      // If not provided, use: "//site/name" <//site/contact>
      $this->sender = sprintf('"%s" <%s>', $this->fMain->configuration->site->name, $this->fMain->configuration->site->contact);
    }

    $this->from = $this->sender;
  }

  function addRecipient($address, $type = self::RECIPIENT_TO, $name = NULL)
  {
    if ( strlen($name) )
    {
      $address = '"' . $name . '" <' . $address . '>';
    }

    // Add them to the recipient strong
    switch ( $type )
    {
      case self::RECIPIENT_TO:
        if ( $this->toString != '' )
        {
          $this->toString .= ', ';
        }
        $this->toString .= $address;
        break;
      case self::RECIPIENT_CC:
        $this->recipientString .= 'Cc: ' . $address . self::NEWLINE;
        break;
      case self::RECIPIENT_BCC:
        $this->recipientString .= 'Bcc: ' . $address . self::NEWLINE;
        break;
    }
  }

  function setSubject($subject)
  {
    $this->subject = $subject;
  }

  function setSender($sender)
  {
    $this->from = $sender;
  }

  function addContent($message, $type = self::CONTENT_TEXT)
  {
    $this->content[$type] = $message;
  }

  function addAttachment($filename, $mimeType = NULL, $content = NULL, $id = NULL, $disposition = 'attachment')
  {
    if ( $content == NULL && file_exists($filename) )
    {
      $content = file_get_contents($filename);
      if ( $mimeType == NULL )
      {
        $data = getImageSize($filename);
        $mimeType = $data['mime'];
      }
    }

    if ( isset($content) )
    {
      $this->attachments[$filename]['mimeType'] = $mimeType;
      $this->attachments[$filename]['content'] = base64_encode($content);
      if ( $id === NULL )
      {
        $this->attachments[$filename]['id'] = "part." . sprintf("%09d", crc32($name)) . strrchr($this->toString, "@");
      }
      else
      {
        $this->attachments[$filename]['id'] = $id;
      }
      $this->attachments[$filename]['disposition'] = $disposition;
    }
  }

  function setFrom($address)
  {
    $this->from = $address;
  }

  function setReplyTo($address)
  {
    $this->replyto = $address;
  }

  function send()
  {
    $this->build();
    $this->fDebug->add('Headers: ' . $this->header, 4);
    $this->fDebug->add('Sending To: ' . $this->toString, 4);
    $this->fDebug->add('Subject: ' . $this->subject, 4);
    $this->fDebug->add('Message: ' . $this->message, 4);
    return mail($this->toString, $this->subject, $this->message, $this->header);
  }

  protected function build()
  {
    $boundary = 'b' . md5(uniqid(rand(), true));
    
    $this->header  = 'From: ' . $this->from . self::NEWLINE;
    if ( $this->sender != $this->from )
    {
      $this->header  .= 'Sender: ' . $this->sender . self::NEWLINE;
    }
    
    if ( strlen($this->replyto) > 0 )
    {
      $this->header  .= 'Reply-To: ' . $this->replyto . self::NEWLINE;
    }


    if ( strlen($this->recipientString) > 0 )
    {
      $this->header .= $this->recipientString;
    }
    $this->header .= 'MIME-Version: 1.0' . self::NEWLINE;
    $this->header .= 'X-Mailer: Framewerk' . self::NEWLINE;
    $this->header .= 'Content-Type: multipart/alternative; boundary="' . $boundary . '";' . self::NEWLINE;
    $this->header .= 'Content-Transfer-Encoding: 7bit' . self::NEWLINE;

    $this->message = "This is a multipart message in mime format.\n\n";

    if ( count($this->attachments) > 0 )
    {
      $this->message .= '--' . $boundary . "\n";
      $related = 'r' . md5(uniqid(rand(), true));
      $this->message .= 'Content-Type: multipart/related; boundary="' . $related . '";' . self::NEWLINE . self::NEWLINE;
    }

    if ( count($this->content) > 1 )
    {

      if ( strstr($this->message, '--' . $boundary) )
      {
        $this->message .= '--' . $related . "\n";
      } else {
        $this->message .= '--' . $boundary . "\n";
      }

      $alternative = 'a' . md5(uniqid(rand(), true));
      $this->message .= 'Content-Type: multipart/alternative; boundary="' . $alternative . '";' . self::NEWLINE . self::NEWLINE;
    }

    foreach ( $this->content AS $type=>$content )
    {
      switch ( $type )
      {
        case self::CONTENT_TEXT:
          $type = 'text/plain';
          $encoding = 'quoted-printable';
          $charset = 'iso-8859-1';
          $content = $this->quoted_printable_encode($content);
          break;
        case self::CONTENT_HTML:
          $type = 'text/html';
          $encoding = 'base64';
          $charset = 'iso-8859-1';
          $content = chunk_split(base64_encode($content), 68, self::NEWLINE);
          break;
        case self::CONTENT_XHTML:
          //XXX: most mail clients won't recognize content type application/xhtml+xml
          $type = 'text/html';
          $encoding = 'base64';
          $charset = 'UTF-8';
          $content = chunk_split(base64_encode($content), 68, self::NEWLINE);
          break;
        default:
          continue;
          break;
      }

      if ( isset($alternative) )
      {
        $this->message .= '--' . $alternative . "\n";
      } else {
        $this->message .= '--' . $boundary . "\n";
      }

      $this->message .= 'Content-Type: ' . $type . '; charset="' . $charset . '"' . self::NEWLINE;
      $this->message .= 'Content-Transfer-Encoding: ' . $encoding . self::NEWLINE . self::NEWLINE;
      $this->message .= $content . "\n\n";
    }

    if ( isset($alternative) )
    {
      $this->message .= '--' . $alternative . '--' . self::NEWLINE;
    }

    foreach ( $this->attachments AS $name=>$attachment )
    {
      $temp = explode('/', $name);
      $sname = $temp[Count($temp ) - 1];
      if ( isset($alternative) )
      {
        $this->message .= '--' . $related . self::NEWLINE;
      } else {
        $this->message .= '--' . $boundary . self::NEWLINE;
      }
      $this->message .= 'Content-Type: ' . $attachment['mimeType'] . '; name="' . $sname . '";' . self::NEWLINE;
      $this->message .= 'Content-Transfer-Encoding: base64' . self::NEWLINE;
      $this->message .= 'Content-Disposition: ' . $attachment['disposition'] . self::NEWLINE;
      $this->message .= 'Content-ID: <' . $attachment['id'] . '>' . self::NEWLINE . self::NEWLINE;
      $this->message .= chunk_split($attachment['content'], 68, "\n");
    }
    if ( isset($related) )
    {
      $this->message .= '--' . $related . '--' . self::NEWLINE;
    }
    $this->message .= '--' . $boundary . '--' . self::NEWLINE;
  }

  /**
   * Function to encode data in quoted_printable format using PHP stream filters
   *
   * @param string $sString
   * @return string
   */
  function quoted_printable_encode( $sString )
  {
    return imap_8bit( $sString );
  }
}
?>

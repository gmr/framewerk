<?php // $Id: CaptchaImage.inc 919 2007-08-06 18:34:41Z dallas $
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
  * Outputs a Funky custom GD Image with a random string, and stores the string inside the session.
  * If GD is not installed, uses Funky output of numbers.
  *
  * Spiral Function from the Captcha Project by Gavin Roy
  * - http://sourceforge.net/projects/session-captcha/
  *
  * Random Pronouncable String Generator
  * - http://www.zend.com/codex.php?id=215&single=1
  *
  * Text ASCII Display: Joe Hansche / Gavin Roy
  *
  * The following were contributed by: Dallas Gutauckis
  * - Text Placement
  * - Tile Background Type
  * - Center Circles Generation
  * - Bubble Generation
  * - Crazy Triangle Generation
  *
  * @author     Fred Ghosn <fredghosn@gmail.com>
  * @link       http://framewerk.org
  * @license    http://opensource.org/licenses/bsd-license.php BSD License
  * @copyright  Copyright 2006-2007 the Framewerk Development Group
  * @version    $Rev: 919 $
  * @since      2006-01-08
  * @package    Applications
  * @subpackage Helpers
  * @uses       TBinaryInterface
  * @todo       Add ASCII Text output XML settings.
  */
class tCaptchaImage extends fApplication
{
  public  $settingsLocation = NULL;

  private $captchaSettings  = NULL;
  private $textAttributes   = NULL;
  private $textNode         = NULL;
  private $randomString     = '';
  private $randomInteger    = 0;
  private $stringLength     = 5;
  private $imageWidth       = 230;
  private $imageHeight      = 100;
  private $imageBase        = false;
  private $registry;

  /**
   * Constructor
   *
   * Let our execute() function set a mime-type if needed
   */
  function __construct()
  {
    // Call our parent's constructor to get all the goodies we use
    parent::__construct();
    
    // Load Settings
    $this->registry = new fRegistry("tCaptchaImage");

    // We don't want to use a theme
    $this->fTheme->useTheme(false);

    // Seed the random number generator
    srand( (double)microtime(true) * 1000003 );
  }

  /**
   * Main rendering routine
   *
   * @return bool
   */
  function execute()
  {
    self::__construct();
    
    // By default the user isn't banned
    $userBanned = false;

    // By default let the registry tell us if we should use gd or not
    $useGD = $this->useGD();

    // If the registry doesnt have a maxAttempts set, set one
    if ( !$this->registry->maxAttempts )
    $this->registry->maxAttempts = 5;

    // Get our session data for the captcha
    $sessionData = $this->fSession->getPersistVariables($this);
    if ( !isset($sessionData['count']) ) $sessionData['count'] = 0;
    if ( intval($sessionData['count']) >= intval($this->registry->maxAttempts) )
    $userBanned = true;
    else
    $sessionData['count']++;

    if ( $useGD )
    {
      $this->fOutput->setMimeType('image/png');

      // Find our text layer and make it easier to access
      foreach( $this->registry->layers->layer as $layer )
      {
        if ( $layer->type == 'text' )
        {
          $this->textNode = $layer;
        }
      }
      
      if ( $userBanned )
      {
        $captchaOutput = $this->createImage('blocked');
      } else {
        if ( isset($this->textNode->minLength) && isset($this->textNode->maxLength) )
        {
          if ($this->textNode->minLength > $this->textNode->maxLength)
          {
            $minLength = intval($this->textNode->maxLength);
            $maxLength = intval($this->textNode->minLength);
          } else {
            $minLength = intval($this->textNode->minLength);
            $maxLength = intval($this->textNode->maxLength);
          }
          $this->stringLength = rand( $minLength, $maxLength );
        } elseif (isset($this->textNode->length))
        $this->stringLength = intval($this->textNode->length);

        switch( $this->textNode->sequence )
        {
          case 'pronouncable':
            $this->randomString = $this->pronounceableString();
            break;
          case 'dictionary';
          $this->randomString = $this->dictionaryString();
          break;
          default:
            $this->randomString = $this->randomString();
        }
        $captchaOutput = $this->createImage();
        $sessionString = $this->randomString;
      }
    } else {
      $this->fOutput->setMimeType('text/html');
      if ( $userBanned )
      $captchaOutput = '<span id="Captcha_bannedMessage">{!i18n:Captcha|Banned}</span>';
      else {
        $this->randomInteger = rand(1000, 99999);
        $captchaOutput = $this->createText();
        $sessionString = $this->randomInteger;
      }
    }

    $this->fSession->setPersistVariables($this, array('string' => $sessionString, 'count' => $sessionData['count']) );
    $this->output($captchaOutput);
    return true;
  }

  /**
   * Create the image
   *
   * @param string $imageType
   * @return string output (binary/text)
   */
  private function createImage( $imageType = 'string' )
  {
    $bgSet       = false; // Check if background was set correctly during background sequence (failure: revert to sold color)
    $textSet     = false; // Only to be set once, to avoid multiple 'text layers'.

    if ( $this->registry != NULL )
    {
      if ( isset($this->registry->image->width) )
        if ( intval($this->registry->image->width) >= 150 )
          $this->imageWidth = intval($this->registry->image->width);
      if ( isset($this->registry->image->height) )
        if ( intval($this->registry->image->height) >= 80 )
          $this->imageHeight = intval($this->registry->image->height);
    }

    $this->imageBase = ImageCreateTrueColor( $this->imageWidth, $this->imageHeight );
    if ( isset($this->registry->image->background) )
    {
      $background = $this->registry->image->background;
      switch( $background->type )
      {
        case 'color':
          if ( ImageFilledRectangle($this->imageBase, 0, 0, $this->imageWidth, $this->imageHeight, $this->setColor($background->val)) )
          {
            $bgSet = true;
          }
          break;
        case 'image':
          $imageFile = false;
          $cleanBackground = $background->path;
          if ( file_exists($cleanBackground) )
          {
            list( $currentWidth, $currentHeight ) = getImageSize(strval($cleanBackground));
            switch( array_pop(explode('.', $cleanBackground)) )
            {
              case 'jpg':
              case 'jpeg':
                $imageFile = ImageCreateFromJPEG($cleanBackground);
                break;
              case 'gif':
                $imageFile = ImageCreateFromGIF($cleanBackground);
                break;
              case 'png':
                $imageFile = ImageCreateFromPNG($cleanBackground);
                break;
            }
            if ( $imageFile )
              $bgColor = (isset($background->color)) ? $background->color : 'FFFFFF';
            ImageFilledRectangle($this->imageBase, 0, 0, $this->imageWidth, $this->imageHeight, $this->setColor($bgColor));
            switch( $background->fill )
            {
              // Create one instance of the image
              case 'normal':
                ImageCopyResized($this->imageBase, $imageFile, 0, 0, 0, 0, $currentWidth, $currentHeight, $currentWidth, $currentHeight);
                break;
                // Tile the image onto the background
              case 'tile':
                $leftRight = ceil( $this->imageWidth / $currentWidth );
                $topBottom = ceil( $this->imageHeight / $currentHeight );
                for ( $i = 0; $i < $leftRight; $i++ )
                  for ( $x = 0; $x < $topBottom; $x++ )
                    ImageCopy( $this->imageBase, $imageFile, $i * $currentWidth, $x * $currentHeight, 0, 0, $currentWidth, $currentHeight );
                break;
                // default = stretched to fit image size
              default:
                ImageCopyResized($this->imageBase, $imageFile, 0, 0, 0, 0, $this->imageWidth, $this->imageHeight, $currentWidth, $currentHeight);
            }
            $bgSet = true;
          }
          break;
      }
    }

    // We don't have a true background sequence (failed or not specified).  Lets just create a plain background.
    if ( !$bgSet )
    {
      $imgBackColor = $this->setColor( ((isset($this->registry->image->color)) ? $this->registry->image->color : 'FFFFFF') );
      ImageFilledRectangle($this->imageBase, 0, 0, $this->imageWidth, $this->imageHeight, $imgBackColor);
    }

    $imgTextColor    = $this->setColor( ((isset($this->textNode->color)) ? $this->textNode->color                       : 'B98C8C') );
    $imgBorderColor  = $this->setColor( ((isset($this->registry->image->border->color)) ? $this->registry->image->border->color : '000000') );
    $fontPath        = (isset($currentLayer->font)) ? $currentLayer->font : 'Toolkit/tForm/tCaptchaField.ttf';
    
    switch ( $imageType )
    {
      case 'string':

        foreach( $this->registry->layers->layer as $currentLayer )
        {
          switch( $currentLayer->type )
          {
            case 'text':
              if ( !$textSet )
              {
                if ( isset($currentLayer->size) )
                {
                  $textSize = intval($currentLayer->size);
                } else {
                  $minSize = 20;
                  $maxSize = 30;
                  if ( isset($currentLayer->minSize) && isset($currentLayer->maxSize) )
                  {
                    if ($currentLayer->minSize > $currentLayer->maxSize )
                    {
                      $minSize = intval($currentLayer->maxSize);
                      $maxSize = intval($currentLayer->minSize);
                    } else {
                      $minSize = intval($currentLayer->minSize);
                      $maxSize = intval($currentLayer->maxSize);
                    }
                  }
                  $textSize = rand( $minSize, $maxSize );
                }
                if ( isset($currentLayer->angle) ) 
                {
                  $textAngle = intval($currentLayer->angle);
                } else {
                  $minAngle = -25;
                  $maxAngle =  25;
                  if ( isset($currentLayer->minAngle) && isset($currentLayer->maxAngle) )
                  {
                    if ($currentLayer->minAngle > $currentLayer->maxAngle )
                    {
                      $minAngle = intval($currentLayer->maxAngle);
                      $maxAngle = intval($currentLayer->minAngle);
                    } else {
                      $minAngle = intval($currentLayer->minAngle);
                      $maxAngle = intval($currentLayer->maxAngle);
                    }
                  }
                  $textAngle = rand( $minAngle, $maxAngle );
                }
                $fitSize = ImageFTBbox( $textSize, $textAngle, $fontPath, $this->randomString );
                if ($textAngle >= 0)
                {
                  $textX = rand((15+$fitSize[6]),$this->imageWidth-$fitSize[2]-15);
                  $textY = rand(($this->imageHeight-abs($fitSize[1])-15),(15+abs($fitSize[5])));
                }
                if ($textAngle < 0)
                {
                  $textX = rand((15+$fitSize[0]),$this->imageWidth-$fitSize[4]-15);
                  $textY = rand(($this->imageHeight-abs($fitSize[3])-15),(15+abs($fitSize[6])));
                }
                if ( isset($currentLayer->dropShadow) )
                {
                  $shadowDistance = (isset($currentLayer->dropShadow->distance)) ? intval($currentLayer->dropShadow->distance) : 2;
                  $shadowColor    = $this->setColor( (($currentLayer->dropShadow->color) ? $currentLayer->dropShadow->color : '000000') );
                  ImageTTFText( $this->imageBase, $textSize, $textAngle, ($textX + $shadowDistance), ($textY + $shadowDistance), $shadowColor, $fontPath, $this->randomString );
                }

                ImageTTFText( $this->imageBase, $textSize, $textAngle, $textX, $textY, $imgTextColor, $fontPath, $this->randomString );
                $textSet = true;
              }
              break;
            case 'spiral':
              $this->createSpiral( rand(0, $this->imageWidth), rand(0, $this->imageHeight), ((isset($currentLayer->color)) ? $currentLayer->color : '000000') );
              break;
            case 'crazytriangles':
              $this->crazyTriangles();
              break;
            case 'centercircles':
              $circleWidth = 20;
              if ( isset($currentLayer->width) )
              $circleWidth = intval($currentLayer->width);
              $this->createCenterCircles( $circleWidth );
              break;
            case 'bubbles':
              $minSize    = 5;
              $maxSize    = 60;
              $minCircles = 5;
              $maxCircles = 80;
              if ( isset($currentLayer->minSize) && isset($currentLayer->maxSize) )
              {
                if ($currentLayer->minSize > $currentLayer->maxSize )
                {
                  $minSize = intval($currentLayer->maxSize);
                  $maxSize = intval($currentLayer->minSize);
                } else {
                  $minSize = intval($currentLayer->minSize);
                  $maxSize = intval($currentLayer->maxSize);
                }
              }
              if ( isset($currentLayer->minCircles) && isset($currentLayer->maxCircles) )
              {
                if ($currentLayer->minCircles > $currentLayer->maxCircles )
                {
                  $minCircles = intval($currentLayer->maxCircles);
                  $maxCircles = intval($currentLayer->minCircles);
                } else {
                  $minCircles = intval($currentLayer->minCircles);
                  $maxCircles = intval($currentLayer->maxCircles);
                }
              }
              $this->createBubbles( $minSize, $maxSize, $minCircles, $maxCircles );
          }
        }
        break;
      case 'blocked':
        ImageTTFText( $this->imageBase, 12, 0, 5, 20, $imgTextColor, $fontPath, fParser::getInstance()->parse('{!i18n:Captcha|Banned}') );
        break;
    }

    // Capture png data and return
    ob_start();
    ImagePNG( $this->imageBase );
    $imageData = ob_get_contents();
    ob_end_clean();

    // Return the output
    return $imageData;
  }

  /**
   * Select a random line from a specified or default dictionary file.
   * Falls back on randomString() if dictionary file does not exist.
   *
   * @return  (string) Random String
   */
  private function dictionaryString()
  {
    $dictionaryFile = NULL;

    // Check for custom dictionary file
    if ( isset($this->textNode->path) )
    if ( file_exists($this->textNode->path) )
    $dictionaryFile = strval($this->textNode->path);

    // If we don't have a custom dictionary, lets check if our default still exists.
    if ( $dictionaryFile == NULL && file_exists('Toolkit/fForm/CaptchaImage.dict') )
    $dictionaryFile = 'Toolkit/fForm/CaptchaImage.dict';

    // No dictionary == fall back to randomString
    if ( $dictionaryFile == NULL )
    return $this->randomString();

    // Return a random word from the list
    $dictionaryWords = file($dictionaryFile);
    return $dictionaryWords[rand(0, sizeof($dictionaryWords) - 1)];
  }

  /**
   * Create a 'pronouncable' string based on vowels/constants
   *
   * @return  (string) Pronouncable String
   */
  private function pronounceableString()
  {
    $vowels = array( "a", "e", "i", "o", "u" );
    $cons   = array( "b", "c", "d", "g", "h", "j", "k", "l", "m", "n", "p", "r", "s", "t", "u", "v", "w", "tr", "cr", "br", "fr", "th", "dr", "ch", "ph", "wr", "st", "sp", "sw", "pr", "sl", "cl" );
    $num_vowels = count( $vowels );
    $num_cons   = count( $cons );
    $finalString = '';
    for( $i = 0; $i < $this->stringLength; $i++ )
    $finalString .= $cons[rand(0, $num_cons - 1)] . $vowels[rand(0, $num_vowels - 1)];
    return substr( $finalString, 0, $this->stringLength );
  }

  /**
   * Create a random string based on a given or default charset.
   *
   * @return  (string) Random String
   */
  function randomString()
  {
    srand((double)microtime()*1000000);
    $i = 0;
    $finalString = '' ;
    if ( isset($this->registry->layers->text->charset) )
    $charset = $this->registry->layers->text->charset;
    else
    $charset = 'abcdefghijkmnopqrstuvwxyz023456789';
    while ( $i < $this->stringLength )
    {
      $num = rand() % 33;
      $tmp = substr($charset, $num, 1);
      $finalString .= $tmp;
      $i++;
    }
    return $finalString;
  }

  /**
   * Return an array of RGB, based on a hex code passed.
   *
   * @return  (array) r,g,b
   */
  private function hex2rgb( $hex = '' )
  {
    $rgb = array( 'r' => '0', 'g' => '0', 'b' => '0' );

    if ( strlen($hex) >= 2 )
    $rgb['r'] = hexdec( substr($hex, 0, 2) );
    if ( strlen($hex) >= 4 )
    $rgb['g'] = hexdec( substr($hex, 2, 2) );
    if ( strlen($hex) >= 6 )
    $rgb['b'] = hexdec( substr($hex, 4, 2) );

    return $rgb;
  }

  /**
   * Allocate a color into imageBase based on a hex code.
   *
   * @return  color identifier
   */
  function setColor( $hex = '' )
  {
    if ( $this->imageBase == NULL )
    return false;

    $rgb = $this->hex2rgb($hex);
    return ImageColorAllocate( $this->imageBase, $rgb['r'], $rgb['g'], $rgb['b'] );
  }

  /**
   * Create X amount of random shaped/colored triangles.
   *
   * @return  bool
   */
  private function crazyTriangles()
  {
    for( $i=0; $i < 10; $i++ )
    {
      $rcolor = imagecolorallocate( $this->imageBase, rand(0,255), rand(0,255), rand(0,255) );
      $sides  = rand(3,6);
      $y      = 0;
      for( $x=0; $x < $sides; $x++ )
      {
        $xpos = rand(5, $this->imageWidth - 5);
        $ypos = rand(5, $this->imageHeight - 5);
        $points[$y++] = $xpos;
        $points[$y++] = $ypos;
      }
      imagefilledpolygon( $this->imageBase, $points, $sides, $rcolor);
    }
    return true;
  }

  /**
   * Create spiral based on random circle/points and random radius size
   *
   * @return  bool
   */
  private function createSpiral( $origin_x = 100, $origin_y = 100, $hex = '' )
  {
    $rgb = $this->hex2rgb($hex);
    $theta   = 1;
    $thetac  = 6;
    $radius  = rand( 10, 14 );
    $circles = rand( 10, 14 );
    $points  = rand( 33, 36 );
    $lcolor  = ImageColorAllocate( $this->imageBase, $rgb['r'], $rgb['g'], $rgb['b'] );
    for( $i = 0; $i < ( $circles * $points ) - 1; $i++ )
    {
      $theta = $theta + $thetac;
      $rad = $radius * ( $i / $points );
      $x = ( $rad * cos( $theta ) ) + $origin_x;
      $y = ( $rad * sin( $theta ) ) + $origin_y;
      $theta = $theta + $thetac;
      $rad1 = $radius * ( ( $i + 1 ) / $points );
      $x1 = ( $rad1 * cos( $theta ) ) + $origin_x;
      $y1 = ( $rad1 * sin( $theta ) ) + $origin_y;
      ImageLine( $this->imageBase, $x, $y, $x1, $y1, $lcolor );
      $theta = $theta - $thetac;
    }
    return true;
  }

  private function createCenterCircles( $circleWidth = 20 )
  {
    $neededWidth = ceil(($this->imageWidth + $this->imageHeight)^(1/2) / 2);
    $circles     = $neededWidth / $circleWidth;
    $startX      = $this->imageWidth / 2;
    $startY      = $this->imageHeight / 2;
    for ( $i = $circles; $i >= 1;$i-- )
    {
      $randColor = imageColorAllocate($this->imageBase, rand(0,255), rand(0,255), rand(0,255));
      imageFilledEllipse($this->imageBase, $startX, $startY, $i * $circleWidth, $i * $circleWidth, $randColor);
    }
    return true;
  }

  private function createBubbles( $minSize = 5, $maxSize = 60, $minCircles = 5, $maxCircles = 80 )
  {
    for ( $i=0; $i < rand($minCircles, $maxCircles); $i++ )
    {
      $randCircleSize  = rand($minSize,$maxSize);
      $randCircleColor = imageColorAllocate($this->imageBase,rand(0,255),rand(0,255),rand(0,255));
      imagefilledellipse($this->imageBase, rand(0, $this->imageWidth),rand(0, $this->imageHeight),$randCircleSize,$randCircleSize,$randCircleColor);
    }
    return true;
  }

  /**
   * Make our text font for the text version
   *
   * @return string font data
   */
  private function createText()
  {
    $numberArray = array();

    $numberArray[0][0] = '  ###### ';
    $numberArray[0][1] = ' ##   ## ';
    $numberArray[0][2] = '##   # ##';
    $numberArray[0][3] = '##  #  ##';
    $numberArray[0][4] = '## #   ##';
    $numberArray[0][5] = ' ##   ## ';
    $numberArray[0][6] = ' ######  ';

    $numberArray[1][0] = ' ## ';
    $numberArray[1][1] = '### ';
    $numberArray[1][2] = ' ## ';
    $numberArray[1][3] = ' ## ';
    $numberArray[1][4] = ' ## ';
    $numberArray[1][5] = ' ## ';
    $numberArray[1][6] = '####';

    $numberArray[2][0] = ' #######';
    $numberArray[2][1] = '##    ##';
    $numberArray[2][2] = '      ##';
    $numberArray[2][3] = ' #######';
    $numberArray[2][4] = '##      ';
    $numberArray[2][5] = '##      ';
    $numberArray[2][6] = '########';

    $numberArray[3][0] = ' ####### ';
    $numberArray[3][1] = '##     ##';
    $numberArray[3][2] = '       ##';
    $numberArray[3][3] = '  ###### ';
    $numberArray[3][4] = '       ##';
    $numberArray[3][5] = '##     ##';
    $numberArray[3][6] = ' ####### ';

    $numberArray[4][0] = '   #### ';
    $numberArray[4][1] = '  ## ## ';
    $numberArray[4][2] = ' ##  ## ';
    $numberArray[4][3] = '##   ## ';
    $numberArray[4][4] = '########';
    $numberArray[4][5] = '     ## ';
    $numberArray[4][6] = '     ## ';

    $numberArray[5][0] = '########';
    $numberArray[5][1] = '##      ';
    $numberArray[5][2] = '##      ';
    $numberArray[5][3] = '####### ';
    $numberArray[5][4] = '      ##';
    $numberArray[5][5] = '      ##';
    $numberArray[5][6] = '####### ';

    $numberArray[6][0] = ' ###### ';
    $numberArray[6][1] = '##    ##';
    $numberArray[6][2] = '##      ';
    $numberArray[6][3] = '####### ';
    $numberArray[6][4] = '##    ##';
    $numberArray[6][5] = '##    ##';
    $numberArray[6][6] = ' ###### ';

    $numberArray[7][0] = '########';
    $numberArray[7][1] = '      ##';
    $numberArray[7][2] = '     ## ';
    $numberArray[7][3] = '    ##  ';
    $numberArray[7][4] = '   ##   ';
    $numberArray[7][5] = '  ##    ';
    $numberArray[7][6] = ' ##     ';

    $numberArray[8][0] = ' ###### ';
    $numberArray[8][1] = '##    ##';
    $numberArray[8][2] = '##    ##';
    $numberArray[8][3] = ' ###### ';
    $numberArray[8][4] = '##    ##';
    $numberArray[8][5] = '##    ##';
    $numberArray[8][6] = ' ###### ';

    $numberArray[9][0] = ' ###### ';
    $numberArray[9][1] = '##    ##';
    $numberArray[9][2] = '##    ##';
    $numberArray[9][3] = ' #######';
    $numberArray[9][4] = '      ##';
    $numberArray[9][5] = '      ##';
    $numberArray[9][6] = '      ##';

    $generateString = strval($this->randomInteger);
    $captchaOutput  = '<div id="captchaContainer">';
    for ( $currentLine = 0; $currentLine < sizeof($numberArray[0]); $currentLine++ )
    {
      $captchaOutput .= "\n" . '<div class="line">';
      for( $i=0; $i < strlen($generateString); $i++ )
      {
        $currentChar = $generateString{$i};
        $numberLine  = $numberArray[intval($currentChar)][$currentLine];
        for( $y=0; $y < strlen($numberLine); $y++ )
        {
          // Depending on whats in the current position, choose either a light or dark color.
          $color = ( $numberLine{$y} === ' ' ) ? ($color = $this->genColor(0)) : ($color = $this->genColor(1));
          // And output the span with whatever color was generated
          $captchaOutput .= '<span style="background:'.$color.';"></span>';;
        }
        // Create an empty space at end of the line
        $color = $this->genColor(0);
        $captchaOutput .= '<span style="background:'.$color.';"></span>';
      }
      $captchaOutput .= "</div>";
    }
    $captchaOutput .= "\n</div>";
    return $captchaOutput;
  }

  /**
   * Return a light(0) or dark(1) color witin set min/max range hex values
   *
   * @return  string
   */
  private function genColor( $px = 0 )
  {
    $color = '#';
    // [0] == "light" color
    // [1] == "dark"
    // this is an array of (min, max) values for the hex color
    $onoff[0] = array(0x99, 0xff);
    $onoff[1] = array(0x00, 0x59);

    $array = $onoff[$px];

    // Generate 3 random values within the light/dark ranges
    // And add them to the color.  pad any values < 10 (ie, 0 -> f) with a 0 on the left side
    for( $i = 0; $i < 3; $i++ )
    $color .= str_pad( dechex( rand($array[0], $array[1]) ), 2, '0', STR_PAD_LEFT );

    return $color;
  }

  /**
   * Function for the parser to return the markup for captcha (img/actual value)
   *
   * @return string
   */
  function returnMarkup()
  {
    if ( $this->useGD() )
    {
      $output = '<img src="/captcha" alt="Session Captcha" class="captcha" />';
      return $output;
    } else {
      if ( $userBanned )
      {
        $captchaOutput = '<span id="Captcha_bannedMessage">{!i18n:Captcha|Banned}</span>';
      } else {
        $sessionData = $this->fSession->getPersistVariables($this);
        $this->randomInteger = rand(1000, 99999);
        $captchaOutput = $this->createText();
        $sessionString = $this->randomInteger;
        $this->fSession->setPersistVariables($this, array('string' => $sessionString, 'count' => $sessionData['count']) );
      }
      return $captchaOutput;
    }
  }

  /**
   * Should we use GD?
   *
   * @return bool
   */
  private function useGD()
  {
      
    if ( ( strtolower($this->registry->image->type) == 'gd' ) ||
    ( function_exists('imagecreate') ) ||
    ( function_exists('ImageFTBbox') ) )
    {
      return true;
    }

    return false;
  }

}
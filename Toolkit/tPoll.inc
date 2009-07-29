<?php // $Id: tPoll.inc 914 2007-08-06 17:45:17Z gmr $
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
 * Poll handling object
 *
 * @author     Joe Hansche <madcoder@gamesurge.net>
 * @link       http://framewerk.org
 * @license    http://opensource.org/licenses/bsd-license.php BSD License
 * @copyright  Copyright 2005-2007 the Framewerk Development Group
 * @version    $Rev: 914 $
 * @package    Toolkit
 * @subpackage Widgets
 * @uses       fObject
 */

class tPoll extends fObject
{
  protected $fRegistry;

  protected $pollid;
  protected $votes = array();
  protected $counts = array();

  protected $form;
  protected $fMessages;
  protected $PDO;

  function __construct($title = '', $question = '', $choices = array())
  {
    parent::__construct();
    $this->fMain = fMain::getInstance();

    $this->fMessages = $this->fMain->fMessages;
    $this->PDO = $this->fMain->PDO;

    $this->validEntity('title');
    $this->validEntity('question');
    $this->validEntity('choices');

    $this->validEntity('votePermission');

    $this->fRegistry = new fRegistry("tPoll");

    fTheme::getInstance()->addObjectCSSFile("tPoll");

    $this->choices = $choices;
    $this->question = $question;
    $this->title = $title;
    $this->votePermission = 'All';
  }

  function title($title = NULL)
  {
    if ( $title !== NULL && strlen(trim($title)) > 0 )
      $this->title = $title;
    return $this->title;
  }

  function question($question = NULL)
  {
    if ( $question !== NULL && strlen(trim($question)) > 0 )
      $this->question = $question;
    return $this->question;
  }

  function choices($choices = NULL)
  {
    if ( $choices !== NULL && is_array($choices) && count($choices) > 0 )
      $this->choices = $choices;

    return $this->choices;
  }

  function addChoice($choice)
  {
    $temp = $this->choices;
    if ( count($temp) >= $this->fRegistry->maxChoices )
    {
      $this->fMessages->add(new fErrorMessage("Trying to add too many choices to the poll.  The maximum allowed is " . intval($this->fRegistry->maxChoices)));
      return false;
    }
    if ( !is_string($choice) || strlen(trim($choice)) <= $this->fRegistry->minChoiceLength )
    {
      $this->fMessages->add(new fErrorMessage("Choice length is too short.  The choice string must be at least " . intval($this->fRegistry->minChoiceLength) . " characters long."));
      return false;
    }
    $temp[] = $choice;
    $this->choices = $temp;
    return count($temp);
  }

  function delChoice($index)
  {
    $temp = $this->choices;
    if ( count($temp) == 0 )
    {
      $this->fMessages->add(new fErrorMessage('Trying to delete a choice from the poll, but the poll choices are empty!'));
      return false;
    }
    if ( !isset($temp[$index]) )
    {
      $this->fMessages->add(new fErrorMessage("Trying to delete a choice that doesn't exist."));
      return false;
    }
    if ( count($this->votes) > 0 )
    {
      $this->fMessages->add(new fNoticeMessage('Deleting a choice from the poll, but there have already been votes submitted.  Any votes that have been cast for this choice will be deleted!'));
    }
    unset($temp[$index]);
    $this->choices = $temp;
    return count($temp);
  }

  function hasVoted()
  {
    if ( $this->pollid === NULL )
    {
      trigger_error("Checking hasVoted(), but no poll data has been loaded.", E_USER_WARNING);
      return false;
    }
    if ( $this->fMain->fSession->fUser === NULL )
    {
      fDebug::getInstance()->add("Checking hasVoted(), but user is not logged in.", 4);
      return false;
    }
    if ( isset($this->votes[$this->fMain->fSession->fUser->memberid]) )
    return true;
    return false;
  }

  function castVote($choice)
  {
    if ( $this->pollid === NULL )
    {
      trigger_error("Casting a vote, but no poll data has been loaded.", E_USER_WARNING);
      return false;
    }
    if ( $this->fMain->fSession->fUser === NULL )
    {
      $this->fMessages->add(new fErrorMessage("You must be logged in to cast your vote."));
      return false;
    }
    if ( isset($this->votes[$this->fMain->fSession->fUser->memberid]) )
    {
      $this->fMessages->add(new fNoticeMessage("You have already voted on this poll."));
      return false;
    }
    if ( strlen($choice) !== 1 )
    {
      throw new Exception('Unexpected poll choice!');
    }
    $query = $this->PDO->prepare("INSERT INTO poll_votes (id, memberid, choice) VALUES (?, ?, ?)");
    $this->counts[$choice]++;
    $this->counts['total']++;
    foreach($this->counts as $k => $v)
    {
      if ( $k === 'total' || $k === 'max' ) continue;
      if ( $v > $this->counts['max'] )
      {
        $this->counts['max'] = $v;
      }
    }
    $this->votes[$this->fMain->fSession->fUser->memberid] = array('choice'=>$choice, 'when'=>time());
    return $query->execute(array($this->pollid, $this->fMain->fSession->fUser->memberid, $choice));
  }

  function setForm( &$form )
  {
    if ( !($form instanceOf TForm) )
    {
      trigger_error("Trying to set form to a non-TForm object.", E_USER_WARNING);
      return false;
    }
    $this->form = &$form;
  }

  function & generateForm()
  {
    if ( !($this->form instanceOf fForm) )
    {
      $this->form = new fForm('post');
    }
    fTheme::getInstance()->addObjectCSSFile('tPoll');
    $this->form->class = 'tPoll';
    if ( strlen($this->pollid) !== 10 )
    {
      $this->fMessages->add(new fErrorMessage("Trying to generate a form that hasn't been saved!"));
      return false;
    }
    $this->form->add(new fHeadlineField($this->title));
    $this->form->add(new fInstructionalField($this->question));
    $this->form->add(new fHiddenField('pollid', '', $this->pollid));
    foreach($this->choices as $k => $choice)
    {
      $this->form->add(new fRadioField('choice', $choice, $k, array('rlabel'=>true, 'required'=>true)));
    }
    $this->form->add(new fSubmitField('Vote','action'));
    return $this->form;
  }

  function xmlData()
  {
    $dom = new DOMDocument('1.0', 'UTF-8');
    $dom->formatOutput = true;
    $root = $dom->createElement('tPoll');
    $root->setAttribute('id', $this->pollid);
    $root->setAttribute('permission', $this->votePermission);
    $root->setAttribute('barWidth', $this->fRegistry->barWidth);
    $root->setAttribute('totalVotes', $this->counts['total']);
    $root->appendChild($dom->createElement('title', $this->title));
    $root->appendChild($dom->createElement('question', $this->question));
    $choices = $root->appendChild($dom->createElement('choices'));
    foreach($this->choices as $k => $choice)
    {
      $node = $choices->appendChild($dom->createElement('choice'));
      if ( isset($this->votes[$this->fMain->fSession->fUser->memberid]) &&
        $this->votes[$this->fMain->fSession->fUser->memberid]['choice'] == $k )
      $node->setAttribute('myvote', $this->votes[$this->fMain->fSession->fUser->memberid]['when']);
      $node->setAttribute('value', $k);
      $node->setAttribute('votes', $this->counts[$k]);
      $node->setAttribute('percent', ($this->counts['total'] > 0 ? $this->counts[$k] / $this->counts['total'] * 100 : 0));
      switch($this->fRegistry->barStyle)
      {
        case 'percent':
          $node->setAttribute('width', intval($this->fRegistry->barWidth * floatVal($node->getAttribute('percent') / 100)));
          break;
        case 'max':
          $node->setAttribute('width', intval($this->fRegistry->barWidth * ($this->counts['max'] > 0 ? $this->counts[$k] / $this->counts['max'] : 0)));
          break;
      }
      $node->appendChild($dom->createCDataSection($choice));
    }
    return $dom->saveXML($root);
  }

  function getPoll($id)
  {
    $this->counts = array();
    $this->votes = array();

    // Get Poll data
    $query = $this->PDO->prepare('SELECT * FROM poll_data WHERE id = ?;');
    $query->execute(array($id));
    if ( $query->rowCount() == 0 )
    {
      $this->fMessages->add(new fErrorMessage("Trying to load data for a poll that doesn't exist!"));
      return false;
    }
    $data = $query->fetch(PDO::FETCH_OBJ);
    $this->pollid = $data->id;
    $this->title = $data->title;
    $this->votePermission = $data->votepermission;
    $this->question = $data->question;

    if ( $this->votePermission !== 'All' &&
    ($this->fMain->fSession->fUser === NULL || !$this->fMain->fSession->fUser->checkPermission($this->votePermission)) )
    {
      $this->fMessages->add(new fErrorMessage("You do not have the necessary permission to view this poll!"));
      return false;
    }

    // Get choices
    $temp = array();
    $query = $this->PDO->prepare('SELECT * FROM poll_choices WHERE id = ? ORDER BY choice;');
    $query->execute(array($id));
    for($i=0; $i<$query->rowCount(); $i++)
    {
      $data = $query->fetch(PDO::FETCH_OBJ);
      $temp[$data->choice] = $data->text;
      $this->counts[$data->choice] = 0;
    }
    $this->choices = $temp;

    // Get votes
    switch($this->fMain->PDODriver)
    {
      case 'pgsql':
        $query = $this->PDO->prepare('SELECT *,extract(epoch from "when") as "when" FROM poll_votes WHERE id = ?;'); break;
      default: throw new Exception('Invalid PDO Driver');
    }
    $query->execute(array($id));
    $this->counts['total'] = 0;
    $this->counts['max'] = 0;
    for($i=0; $i<$query->rowCount(); $i++)
    {
      $data = $query->fetch(PDO::FETCH_OBJ);
      if ( !isset($this->counts[$data->choice]) )
      $this->counts[$data->choice] = 1;
      else
      $this->counts[$data->choice]++;

      if ( $this->counts[$data->choice] > $this->counts['max'] )
      $this->counts['max'] = $this->counts[$data->choice];
      $this->counts['total']++;
      $this->votes[$data->memberid] = array('choice'=>$data->choice, 'when'=>$data->when);
    }

    return true;
  }
  /**
   * Need a description
   *
   * @todo       Make this use the switch branching in PDO queries to allow for easier expansion if someone wants to write MySQL support or something else
   * @return int
   */
  function savePoll()
  {
    if ( strlen($this->question) * strlen($this->title) * count($this->choices) == 0 )
    {
      $this->fMessages->add(new fErrorMessage('Trying to save the poll, but not all fields are present.'));
      return false;
    }
    if ( count($this->choices) < intval($this->fRegistry->minChoices) )
    {
      $this->fMessages->add(new fErrorMessage('Trying to save the poll, but not enough choices.  There must be at least ' . intval($this->fRegistry->minChoices) . ' choice(s).'));
      return false;
    }
    $this->pollid = generateID('PL');

    try
    {
      $this->PDO->beginTransaction();
      $query = $this->PDO->prepare("INSERT INTO poll_data (id, title, question, votepermission) VALUES (?, ?, ?, ?);");
      if ( !$query->execute(array($this->pollid, $this->title, $this->question, $this->votePermission)) )
      throw new Exception('Error while inserting poll data');
      unset($query);

      foreach($this->choices as $k => $choice)
      {
        if ( strlen($choice) >= intval($this->fRegistry->minChoiceLength) )
        {
          $query = $this->PDO->prepare("INSERT INTO poll_choices (id, choice, text) VALUES (?, ?, ?);");
          if ( !$query->execute(array($this->pollid, $k, $choice)) )
          throw new Exception('Error while inserting choice ' . $k);
          unset($query);
        }
      }
      $this->PDO->commit();
    } catch ( Exception $e )
    {
      $this->PDO->rollback();
      $this->fMessages->add(new fDBErrorMessage($e->getMessage()));
      return false;
    }
    return $this->pollid;
  }
}
?>
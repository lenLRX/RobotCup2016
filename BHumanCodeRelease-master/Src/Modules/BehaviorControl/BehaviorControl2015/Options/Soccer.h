/** The root option that controls the behavior before the robot actually starts to play */
option(Soccer)
{
  common_transition
  {
    theArmMotionRequest.armMotion[Arms::left] = ArmMotionRequest::none;
    theArmMotionRequest.armMotion[Arms::right] = ArmMotionRequest::none;
    theHeadControlMode = HeadControl::none;
  }

  /** Initially, all robot joints are off until the chest button is pressed. */
  initial_state(playDead)
  {
    transition
    {
		//pengwanli 
		//SystemCall::getMode()返回值始终为physicalRobot
      if(SystemCall::getMode() == SystemCall::simulatedRobot)
        goto standUp; // Don't wait for the button in SimRobot

      if(action_done) // chest button pressed and released
        goto standUp;

      // Skip playDead state at a restart after a crash
      else if(Global::getSettings().recover)
        goto standUp;
    }
    action
    {
      SpecialAction(SpecialActionRequest::playDead);
      ButtonPressedAndReleased(KeyStates::chest, 1000, 0);
    }
  }

  /** The robot stands up and starts to play when stand was executed. */
  state(standUp)
  {
    transition
    {
      if(action_done)
        goto playSoccer;
    }
    action
    {
      LookForward();
      Stand();
    }
  }

  /**
   * The main state that triggers the actual soccer behavior.
   * It also checks whether the chest button was pressed.
   */
  state(playSoccer)
  {
    transition
    {
      if(action_done) // chest button pressed and released once
        goto waitForSecondButtonPress;
    }
    action
    {
      HandlePenaltyState();
      HeadControl();
      ButtonPressedAndReleased(KeyStates::chest, 1000, 200);
    }
  }

  state(chestButtonPressedOnce)
  {
    transition
    {
      if(state_time > 0)
        goto playSoccer;
    }
    action
    {
      HandlePenaltyState();
      HeadControl();
    }
  }

  /** The following two states check whether the chest button is quickly pressed another two times. */
  state(waitForSecondButtonPress)
  {
    transition
    {
      if(action_done) // chest button pressed and released for the second time
        goto waitForThirdButtonPress;
      else if(action_aborted) // too slow -> abort
        goto chestButtonPressedOnce;
    }
    action
    {
      HandlePenaltyState();
      HeadControl();
      ButtonPressedAndReleased(KeyStates::chest, 1000, 200);
    }
  }

  state(waitForThirdButtonPress)
  {
    transition
    {
      if(action_done) // chest button pressed and released for the third time
        goto sitDown;
      else if(action_aborted) // too slow -> abort
        goto playSoccer;
    }
    action
    {
      HandlePenaltyState();
      HeadControl();
      ButtonPressedAndReleased(KeyStates::chest, 1000, 200);
    }
  }

  /** The robot sits down and turns off all joints afterwards. */
  state(sitDown)
  {
    transition
    {
      if(action_done)
        goto playDeadDoNotRecover;
    }
    action
    {
      LookForward();
      SpecialAction(SpecialActionRequest::sitDown);
    }
  }

  /** After pressing the chest button thrice we don't want the robot to recover */
  state(playDeadDoNotRecover)
  {
    transition
    {
      if(action_done) // chest button pressed and released
        goto standUp;
    }
    action
    {
      SpecialAction(SpecialActionRequest::playDead);
      ButtonPressedAndReleased(KeyStates::chest, 1000, 0);
    }
  }
}

/*
 * Copyright (c) 2014, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 */
/*
@test
@bug 6236247
@summary Test that setting of always-on-top flags before showing window works
@author dom@sparc.spb.su: area=awt.toplevel
@run main TestAlwaysOnTopBeforeShow
*/

/**
 * TestAlwaysOnTopBeforeShow.java
 *
 * summary:  Test that always-on-top works in the following situations:
 * - when set on a window before showing
 * - when set on a child dialog
 * - that it doesn't generate focus event when set on an invisible window
 */

import java.awt.*;
import java.awt.event.*;
import java.util.concurrent.atomic.AtomicBoolean;


//*** global search and replace TestAlwaysOnTopBeforeShow with name of the test ***

public class TestAlwaysOnTopBeforeShow
{

    //*** test-writer defined static variables go here ***

    private static AtomicBoolean focused = new AtomicBoolean();
    private static AtomicBoolean pressed = new AtomicBoolean();
    private static volatile Object pressedTarget;
    private static Robot robot = null;
    private static void init()
    {
        //*** Create instructions for the user here ***

        Toolkit.getDefaultToolkit().addAWTEventListener(new AWTEventListener() {
                public void eventDispatched(AWTEvent e) {
                    if (e.getID() == MouseEvent.MOUSE_PRESSED) {
                        synchronized(pressed) {
                            pressed.set(true);
                            pressedTarget = e.getSource();
                            pressed.notifyAll();
                        }
                    }
                }
            }, AWTEvent.MOUSE_EVENT_MASK);

        Frame f = new Frame("always-on-top");
        f.setBounds(0, 0, 200, 200);
        f.addFocusListener(new FocusAdapter() {
                public void focusGained(FocusEvent e) {
                    synchronized(focused) {
                        focused.set(true);
                        focused.notifyAll();
                    }
                }
            });

        f.setAlwaysOnTop(true);

        waitForIdle(1000);
        if (focused.get()) {
            throw new RuntimeException("Always-on-top generated focus event");
        }

        f.setVisible(true);

        waitFocused(f, focused);
        focused.set(false);

        Frame f2 = new Frame("auxilary");
        f2.setBounds(100, 0, 200, 100);
        f2.setVisible(true);
        f2.toFront();
        waitForIdle(1000);

        Point location = f.getLocationOnScreen();
        Dimension size = f.getSize();
        checkOnTop(f, f2, location.x + size.width / 2, location.y + size.height / 2);

        Dialog d = new Dialog(f, "Always-on-top");
        d.pack();
        d.setBounds(0, 0, 100, 100);

        waitForIdle(1000);
        checkOnTop(f, f2, location.x + size.width / 2, location.y + size.height / 2);
        waitForIdle(1000);

        focused.set(false);
        f.setVisible(false);
        f.setAlwaysOnTop(false);
        waitForIdle(1000);
        if (focused.get()) {
            throw new RuntimeException("Always-on-top generated focus event");
        }

        TestAlwaysOnTopBeforeShow.pass();

    }//End  init()

    private static void waitForIdle(int mls) {
        try {
            if(robot == null) {
                robot = new Robot();
            }
            robot.waitForIdle();
            Thread.sleep(mls);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    static void waitFocused(Window w, AtomicBoolean b) {
        try {
            synchronized(b) {
                if (w.isFocusOwner()) {
                    return;
                }
                b.wait(3000);
            }
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
        if (!w.isFocusOwner()) {
            throw new RuntimeException("Can't make " + w + " focus owner");
        }
    }

    static void checkOnTop(Window ontop, Window under, int x, int y) {
        under.toFront();
        try {
            Robot robot = new Robot();
            robot.mouseMove(x, y);
            robot.mousePress(InputEvent.BUTTON1_MASK);
            robot.mouseRelease(InputEvent.BUTTON1_MASK);
            synchronized(pressed) {
                if (pressed.get()) {
                    if (pressedTarget != ontop) {
                        throw new RuntimeException("Pressed at wrong location: " + pressedTarget);
                    }
                } else {
                    pressed.wait(5000);
                }
            }
            if (!pressed.get() || pressedTarget != ontop) {
                throw new RuntimeException("Pressed at wrong location: " + pressedTarget);
            }
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

    /*****************************************************
     * Standard Test Machinery Section
     * DO NOT modify anything in this section -- it's a
     * standard chunk of code which has all of the
     * synchronisation necessary for the test harness.
     * By keeping it the same in all tests, it is easier
     * to read and understand someone else's test, as
     * well as insuring that all tests behave correctly
     * with the test harness.
     * There is a section following this for test-
     * classes
     ******************************************************/
    private static boolean theTestPassed = false;
    private static boolean testGeneratedInterrupt = false;
    private static String failureMessage = "";

    private static Thread mainThread = null;

    private static int sleepTime = 300000;

    // Not sure about what happens if multiple of this test are
    //  instantiated in the same VM.  Being static (and using
    //  static vars), it aint gonna work.  Not worrying about
    //  it for now.
    public static void main( String args[] ) throws InterruptedException
    {
        mainThread = Thread.currentThread();
        try
        {
            init();
        }
        catch( TestPassedException e )
        {
            //The test passed, so just return from main and harness will
            // interepret this return as a pass
            return;
        }
        //At this point, neither test pass nor test fail has been
        // called -- either would have thrown an exception and ended the
        // test, so we know we have multiple threads.

        //Test involves other threads, so sleep and wait for them to
        // called pass() or fail()
        try
        {
            Thread.sleep( sleepTime );
            //Timed out, so fail the test
            throw new RuntimeException( "Timed out after " + sleepTime/1000 + " seconds" );
        }
        catch (InterruptedException e)
        {
            //The test harness may have interrupted the test.  If so, rethrow the exception
            // so that the harness gets it and deals with it.
            if( ! testGeneratedInterrupt ) throw e;

            //reset flag in case hit this code more than once for some reason (just safety)
            testGeneratedInterrupt = false;

            if ( theTestPassed == false )
            {
                throw new RuntimeException( failureMessage );
            }
        }

    }//main

    public static synchronized void setTimeoutTo( int seconds )
    {
        sleepTime = seconds * 1000;
    }

    public static synchronized void pass()
    {
        Sysout.println( "The test passed." );
        Sysout.println( "The test is over, hit  Ctl-C to stop Java VM" );
        //first check if this is executing in main thread
        if ( mainThread == Thread.currentThread() )
        {
            //Still in the main thread, so set the flag just for kicks,
            // and throw a test passed exception which will be caught
            // and end the test.
            theTestPassed = true;
            throw new TestPassedException();
        }
        theTestPassed = true;
        testGeneratedInterrupt = true;
        mainThread.interrupt();
    }//pass()

    public static synchronized void fail()
    {
        //test writer didn't specify why test failed, so give generic
        fail( "it just plain failed! :-)" );
    }

    public static synchronized void fail( String whyFailed )
    {
        Sysout.println( "The test failed: " + whyFailed );
        Sysout.println( "The test is over, hit  Ctl-C to stop Java VM" );
        //check if this called from main thread
        if ( mainThread == Thread.currentThread() )
        {
            //If main thread, fail now 'cause not sleeping
            throw new RuntimeException( whyFailed );
        }
        theTestPassed = false;
        testGeneratedInterrupt = true;
        failureMessage = whyFailed;
        mainThread.interrupt();
    }//fail()

}// class TestAlwaysOnTopBeforeShow

//This exception is used to exit from any level of call nesting
// when it's determined that the test has passed, and immediately
// end the test.
class TestPassedException extends RuntimeException
{
}

//*********** End Standard Test Machinery Section **********


//************ Begin classes defined for the test ****************

// if want to make listeners, here is the recommended place for them, then instantiate
//  them in init()

/* Example of a class which may be written as part of a test
class NewClass implements anInterface
 {
   static int newVar = 0;

   public void eventDispatched(AWTEvent e)
    {
      //Counting events to see if we get enough
      eventCount++;

      if( eventCount == 20 )
       {
         //got enough events, so pass

         TestAlwaysOnTopBeforeShow.pass();
       }
      else if( tries == 20 )
       {
         //tried too many times without getting enough events so fail

         TestAlwaysOnTopBeforeShow.fail();
       }

    }// eventDispatched()

 }// NewClass class

*/


//************** End classes defined for the test *******************




/****************************************************
 Standard Test Machinery
 DO NOT modify anything below -- it's a standard
  chunk of code whose purpose is to make user
  interaction uniform, and thereby make it simpler
  to read and understand someone else's test.
 ****************************************************/

/**
 This is part of the standard test machinery.
 It creates a dialog (with the instructions), and is the interface
  for sending text messages to the user.
 To print the instructions, send an array of strings to Sysout.createDialog
  WithInstructions method.  Put one line of instructions per array entry.
 To display a message for the tester to see, simply call Sysout.println
  with the string to be displayed.
 This mimics System.out.println but works within the test harness as well
  as standalone.
 */

class Sysout
{
    private static TestDialog dialog;

    public static void createDialogWithInstructions( String[] instructions )
    {
        dialog = new TestDialog( new Frame(), "Instructions" );
        dialog.printInstructions( instructions );
        dialog.setVisible(true);
        println( "Any messages for the tester will display here." );
    }

    public static void createDialog( )
    {
        dialog = new TestDialog( new Frame(), "Instructions" );
        String[] defInstr = { "Instructions will appear here. ", "" } ;
        dialog.printInstructions( defInstr );
        dialog.setVisible(true);
        println( "Any messages for the tester will display here." );
    }


    public static void printInstructions( String[] instructions )
    {
        dialog.printInstructions( instructions );
    }


    public static void println( String messageIn )
    {
        System.out.println(messageIn);
    }

}// Sysout  class

/**
  This is part of the standard test machinery.  It provides a place for the
   test instructions to be displayed, and a place for interactive messages
   to the user to be displayed.
  To have the test instructions displayed, see Sysout.
  To have a message to the user be displayed, see Sysout.
  Do not call anything in this dialog directly.
  */
class TestDialog extends Dialog
{

    TextArea instructionsText;
    TextArea messageText;
    int maxStringLength = 80;

    //DO NOT call this directly, go through Sysout
    public TestDialog( Frame frame, String name )
    {
        super( frame, name );
        int scrollBoth = TextArea.SCROLLBARS_BOTH;
        instructionsText = new TextArea( "", 15, maxStringLength, scrollBoth );
        add( "North", instructionsText );

        messageText = new TextArea( "", 5, maxStringLength, scrollBoth );
        add("Center", messageText);

        pack();

        setVisible(true);
    }// TestDialog()

    //DO NOT call this directly, go through Sysout
    public void printInstructions( String[] instructions )
    {
        //Clear out any current instructions
        instructionsText.setText( "" );

        //Go down array of instruction strings

        String printStr, remainingStr;
        for( int i=0; i < instructions.length; i++ )
        {
            //chop up each into pieces maxSringLength long
            remainingStr = instructions[ i ];
            while( remainingStr.length() > 0 )
            {
                //if longer than max then chop off first max chars to print
                if( remainingStr.length() >= maxStringLength )
                {
                    //Try to chop on a word boundary
                    int posOfSpace = remainingStr.
                        lastIndexOf( ' ', maxStringLength - 1 );

                    if( posOfSpace <= 0 ) posOfSpace = maxStringLength - 1;

                    printStr = remainingStr.substring( 0, posOfSpace + 1 );
                    remainingStr = remainingStr.substring( posOfSpace + 1 );
                }
                //else just print
                else
                {
                    printStr = remainingStr;
                    remainingStr = "";
                }

                instructionsText.append( printStr + "\n" );

            }// while

        }// for

    }//printInstructions()

    //DO NOT call this directly, go through Sysout
    public void displayMessage( String messageIn )
    {
        messageText.append( messageIn + "\n" );
        System.out.println(messageIn);
    }

}// TestDialog  class

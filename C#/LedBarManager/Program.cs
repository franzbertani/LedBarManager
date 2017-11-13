/// Just an example...
/// If you're curious you can open a serial monitor to take a look 
/// at the communication with arduino.

using System;

namespace LedBarManager
{
    public class Program
    {

        static void Main()
        {
            /// Create LedBarManager object.
            /// Port name is set based on my Mac, probabily you should change it.
            LedBarManager lb = new LedBarManager("/dev/cu.usbmodem1411");

            /// Set the color for the teams
            lb.SetTeamColor(1, "FF00FF");
            lb.SetTeamColor(2, "453400");

            /// Ready, set, go. 
            /// 10 is the total number of points to win the game.
            lb.StartProgressBar(10);
            System.Threading.Thread.Sleep(2000);
            /// Add some points here and there.
            /// Not possible to overflow: if 10 is the maximum number of points
            /// each team can go up to 10.
            /// lb.AddPoint(teamNumber) will add 1 point by default.
            /// ATTENTION: points is different from the led number which is 
            /// determined according to the percentage...
            lb.AddPoint(1, 4);
            lb.AddPoint(2, 3);

            /// Just to let things go a little bit slower...
            System.Threading.Thread.Sleep(1000);
            lb.AddPoint(2, 1);

            System.Threading.Thread.Sleep(1000);
            lb.AddPoint(1, 10);

            /// two effects at a time?
            /// not a problem bro!
            System.Threading.Thread.Sleep(1000);
            lb.ShowEffect(2, 9, 10000);
            System.Threading.Thread.Sleep(1000);
            //lb.ShowEffect(0, 0, 5000);


            System.Threading.Thread.Sleep(10000);


            /// This work as a reset.
            lb.StartProgressBar(10);
            System.Threading.Thread.Sleep(1000);
            lb.AddPoint(2, 3);
            lb.AddPoint(1, 1);


            /// Instantly switches off everything.
            System.Threading.Thread.Sleep(1000);
            lb.SwitchOff();

            /// Let start again...
            System.Threading.Thread.Sleep(1000);
            lb.StartProgressBar(10);
            System.Threading.Thread.Sleep(1000);
            lb.AddPoint(2, 5);
            lb.AddPoint(1, 5);

            while(true){};

        }
    }
}

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

            /// Add some points here and there.
            /// Not possible to overflow: if 10 is the maximum number of points
            /// each team can go up to 10.
            /// lb.AddPoint(teamNumber) will add 1 point by default.
            lb.AddPoint(1, 4);
            lb.AddPoint(2, 3);
            lb.AddPoint(1, 1);

            /// Just to let things go a little bit slower...
            System.Threading.Thread.Sleep(1000);
            lb.AddPoint(1);
            lb.AddPoint(2, 5);

            System.Threading.Thread.Sleep(1000);
            lb.AddPoint(1, 10);

            /// Not really working now... 
            /// Need to fix on Arduino side...
            System.Threading.Thread.Sleep(1000);
            lb.ShowEffect(0, 3, 2000);

            /// This work as a reset.
            lb.StartProgressBar(10);


        }
    }
}

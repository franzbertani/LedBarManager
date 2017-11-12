using System;

namespace LedBarManager
{
    public class Program
    {
        static void Main()
        {

            LedBarManager gp = new LedBarManager("/dev/cu.usbmodem1411");
            gp.SetTeamColor(1, "FF00FF");
            gp.SetTeamColor(2, "453400");
            gp.StartProgressBar(10);

            gp.AddPoint(1, 4);
            gp.AddPoint(2, 3);
            gp.AddPoint(1, 1);

            System.Threading.Thread.Sleep(1000);
            gp.AddPoint(1);
            gp.AddPoint(2, 5);

            System.Threading.Thread.Sleep(1000);
            gp.AddPoint(1, 10);

            System.Threading.Thread.Sleep(1000);
            gp.ShowEffect(0, 3, 2000);




        }
    }
}

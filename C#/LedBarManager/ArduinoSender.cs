using System;
using CommandMessenger;
using CommandMessenger.Transport.Serial;

namespace LedBarManager
{

    enum Command
    {
        Acknowledge,
        Error,
        LedNumberRequest,
        LedNumberResponse,
        LightUpLed,
        SetPlayerColor,
        StartProgressBar,
        ShowEffect,
    };


    public class ArduinoSender
    {

        private SerialTransport _serialTransport;
        private CmdMessenger _cmdMessenger;
        private bool _arduinoReady = false;

        /// <summary>
        /// Setup the serial connection.
        /// </summary>
        /// <param name="portName">Name of the serial port.</param>
        /// <param name="baudRate">Baud rate.</param>
        public void Setup(string portName, int baudRate)
        {
            _serialTransport = new SerialTransport
            {
                CurrentSerialSettings = { PortName = portName, BaudRate = baudRate, DtrEnable = false } // object initializer
            };
            _cmdMessenger = new CmdMessenger(_serialTransport, BoardType.Bit16);
            AttachCommandCallBacks();

            // Attach to received and sent lines a newline
            _cmdMessenger.NewLineReceived += NewLineReceived;
            _cmdMessenger.NewLineSent += NewLineSent;

            _cmdMessenger.Connect();
        }



        // ---------------------------------
        // ---       Command Sender     ----
        // ---------------------------------



        /// <summary>
        /// Gets the number of leds available to effectively store the points.
        /// </summary>
        /// <returns>The led number.</returns>
        public int GetLedNumber()
        {
            var command = new SendCommand((int)Command.LedNumberRequest, (int)Command.LedNumberResponse, 1000);
            var response = _cmdMessenger.SendCommand(command);
            return response.ReadInt32Arg();
        }

        /// <summary>
        /// Request to light a given number of leds for a given player.
        /// </summary>
        /// <param name="ledNumber">Led number.</param>
        /// <param name="playerNumber">Player number (1 or 2)</param>
        public void LightUpLed(int ledNumber, int playerNumber)
        {
            if (ledNumber < 0)
                throw new ArgumentOutOfRangeException(nameof(ledNumber), "negative led count");
            if (playerNumber != 1 && playerNumber != 2)
                throw new ArgumentOutOfRangeException(nameof(playerNumber), "player does not exists");

            string[] arguments = new string[2];
            arguments[0] = ledNumber.ToString();
            arguments[1] = playerNumber.ToString();
            var command = new SendCommand((int)Command.LightUpLed, arguments, (int)Command.Acknowledge, 1000);

            _cmdMessenger.SendCommand(command);
        }

        /// <summary>
        /// Sets the color of the player.
        /// </summary>
        /// <param name="playerNumber">Player number.</param>
        /// <param name="color">HEX string of the color.</param>
        public void SetPlayerColor(int playerNumber, string color)
        {
            if (playerNumber != 1 && playerNumber != 2)
                throw new ArgumentOutOfRangeException(nameof(playerNumber), "player does not exists");
            if (color.Length != 6)
                throw new ArgumentException(nameof(color), "not a valid hex color");

            string[] arguments = new string[2];
            arguments[0] = playerNumber.ToString();
            arguments[1] = color;
            var command = new SendCommand((int)Command.SetPlayerColor, arguments, (int)Command.Acknowledge, 1000);

            _cmdMessenger.SendCommand(command);

        }

        /// <summary>
        /// Starts the progress bar to display teams score.
        /// Uses default color if SetPlayerColor hasn't been called.
        /// </summary>
        public void StartProgressBar()
        {
            var command = new SendCommand((int)Command.StartProgressBar, (int)Command.Acknowledge, 1000);
            _cmdMessenger.SendCommand(command);
        }

        /// <summary>
        /// Shows the effect.
        /// </summary>
        /// <param name="barPortion">Bar portion (0- first player, 1-second player, 3-entire bar)</param>
        /// <param name="effect">Effect number.</param>
        /// <param name="duration">Duration.</param>
        public void ShowEffect(int barPortion, int effect, int duration)
        {
            string[] arguments = new string[3];
            arguments[0] = barPortion.ToString();
            arguments[1] = effect.ToString();
            arguments[2] = duration.ToString();
            var command = new SendCommand((int)Command.ShowEffect, arguments, (int)Command.Acknowledge, 1000);
            _cmdMessenger.SendCommand(command);
        }

        /// <summary>
        /// Close the communication stream.
        /// </summary>
        public void Exit()
        {
            _cmdMessenger.Disconnect();
            _cmdMessenger.Dispose();
            _serialTransport.Dispose();
        }

        /// <summary>
        /// Checks if Arduino is ready
        /// </summary>
        public bool isReady()
        {
            return _arduinoReady;
        }

        /// <summary>
        /// Attachs the command call backs.
        /// </summary>
        private void AttachCommandCallBacks()
        {
            _cmdMessenger.Attach(OnUnknownCommand);
            _cmdMessenger.Attach((int)Command.Acknowledge, OnAcknowledge);
            _cmdMessenger.Attach((int)Command.Error, OnError);

        }



        // ------------------  C A L L B A C K S ---------------------

        // Called when a received command has no attached function.
        void OnUnknownCommand(ReceivedCommand arguments)
        {
            Console.WriteLine("Command without attached callback received");
        }

        // Callback function that prints that the Arduino has acknowledged
        void OnAcknowledge(ReceivedCommand arguments)
        {
            Console.WriteLine("ACK: " + arguments.ReadStringArg());
            _arduinoReady = true;
        }

        // Callback function that prints that the Arduino has experienced an error
        void OnError(ReceivedCommand arguments)
        {
            Console.WriteLine("ERR: " + arguments.ReadStringArg());
        }

        // Log received line to console
        private void NewLineReceived(object sender, CommandEventArgs e)
        {
            Console.WriteLine(@"Received > " + e.Command.CommandString());
        }

        // Log sent line to console
        private void NewLineSent(object sender, CommandEventArgs e)
        {
            Console.WriteLine(@"Sent > " + e.Command.CommandString());
        }




    }
}

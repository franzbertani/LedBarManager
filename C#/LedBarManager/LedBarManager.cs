using System;
namespace LedBarManager
{
    public class LedBarManager
    {
        private int _pointsInGame;
        private int _ledsPerTeam;
        private ArduinoSender _sender;
        private float _percentagePerPoint;

        private float _team1Percentage, _team2Percentage;
        private int _team1Points, _team2Points;


        /// <summary>
        /// Initializes a new instance of the <see cref="T:LedBarManager.LedBarManager"/> class.
        /// Default Baud rate 9600.
        /// </summary>
        /// <param name="portName">Port name.</param>
        public LedBarManager(string portName)
        {
            _sender = new ArduinoSender();
            _sender.Setup(portName, 9600);
            while (!_sender.isReady()) { };
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="T:LedBar.LedBar"/> class.
        /// </summary>
        /// <param name="portName">Port name.</param>
        /// <param name="baudRate">Baud rate. (default 9600)</param>
        public LedBarManager(string portName, int baudRate)
        {
            _sender = new ArduinoSender();
            _sender.Setup(portName, baudRate);
            while (!_sender.isReady()) { };
        }

        /// <summary>
        /// Starts the progress bar to display score.
        /// Default colors are used if not previously set.
        /// </summary>
        /// <param name="pointsInGame">Total number of points in the game.</param>
        public void StartProgressBar(int pointsInGame)
        {
            _pointsInGame = pointsInGame;
            _ledsPerTeam = (int)((_sender.GetLedNumber() / 2.0) + 0.5);
            _percentagePerPoint = (float)_ledsPerTeam / _pointsInGame;
            _team1Percentage = 0;
            _team2Percentage = 0;
            _sender.StartProgressBar();

        }

        /// <summary>
        /// Adds the point.
        /// </summary>
        /// <param name="team">Team (1 or 2)</param>
        /// <param name="number">Number of points to add (default 1)</param>
        public void AddPoint(int team, int number = 1)
        {

            float toAdd = _percentagePerPoint * number;
            int ledNumber;
            int newTeamPoints;
            switch (team)
            {
                case 1:
                    _team1Percentage += toAdd;
                    newTeamPoints = _team1Points + (int)_team1Percentage;
                    newTeamPoints = newTeamPoints < _pointsInGame ? newTeamPoints : _pointsInGame;
                    ledNumber = newTeamPoints - _team1Points;
                    _team1Percentage = _team1Percentage - ledNumber;
                    _sender.LightUpLed(ledNumber, 1);
                    _team1Points = newTeamPoints;
                    break;
                case 2:
                    _team2Percentage += toAdd;
                    newTeamPoints = _team2Points + (int)_team2Percentage;
                    newTeamPoints = newTeamPoints < _pointsInGame ? newTeamPoints : _pointsInGame;
                    ledNumber = newTeamPoints - _team2Points;
                    _team2Percentage = _team2Percentage - ledNumber;
                    _sender.LightUpLed(ledNumber, 2);
                    _team2Points = newTeamPoints;
                    break;
                default:
                    throw new ArgumentOutOfRangeException(nameof(team), "player does not exist.");

            }
        }


        /// <summary>
        /// Sets the color of the team.
        /// </summary>
        /// <param name="team">Team.</param>
        /// <param name="hexColor">HEX color string.</param>
        public void SetTeamColor(int team, string hexColor)
        {
            if (team != 1 && team != 2)
            {
                throw new ArgumentOutOfRangeException(nameof(team), "player does not exist.");
            }

            _sender.SetPlayerColor(team, hexColor);
        }


        /// <summary>
        /// Resets the progress bar.
        /// If game is on it brings back score to 0.
        /// </summary>
        public void ResetProgressBar()
        {
            _sender.StartProgressBar();
        }

        /// <summary>
        /// Shows the effect.
        /// </summary>
        /// <param name="barPortion">Bar portion (0- first player, 1-second player, 3-entire bar).</param>
        /// <param name="effectNumber">Effect number.</param>
        /// <param name="duration">Duration in milliseconds. (default 5000ms)</param>
        public void ShowEffect(int barPortion, int effectNumber, int duration = 5000)
        {
            _sender.ShowEffect(barPortion, effectNumber, duration);
        }
    }
}

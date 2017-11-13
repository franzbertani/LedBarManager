#include <CmdMessenger.h>
#include <FastLED.h>

// ------------------  F A S T L E D ------------------

#define LED_PIN     5
#define COLOR_ORDER GRB
#define CHIPSET     WS2812
#define NUM_LEDS    30
#define BAR_DISTANCE 0
#define MAX_INT_VALUE 65536
#define BRIGHTNESS  200
#define FRAMES_PER_SECOND 60

bool gReverseDirection = false;
CRGB leds[NUM_LEDS];


// ------------  P R O G R E S S - B A R ---------------

unsigned long int team1Color = 0xff0000;  // default colors
unsigned long int team2Color = 0x0000ff;
int team1Leds, team2Leds;
CRGB *team1Front, *team1Tail, *team2Front, *team2Tail; //handy pointers
bool showingProgressBar; // to know if currently in game



// -------------  L E D - E F F E C T S ----------------
CRGB *barPortion[3];
int effectLedsCount[3] = {(NUM_LEDS - BAR_DISTANCE)/2, (NUM_LEDS - BAR_DISTANCE)/2, NUM_LEDS};
uint16_t frame[3] = {0, 0, 0};
uint16_t animateSpeed = 100;
uint8_t  animation[3];
uint8_t brightness = 50;
bool effectRunning[3] = {false, false, false};
long int effectTimestamp[3] = {0, 0, 0};
int effectDuration[3] = {5000, 5000, 5000}; // default value (milliseconds)

enum barPortion
{
  // bar portions
  kFirstTeamBar,
  kSecondTeamBar,
  kEntireBar,
};
enum ledEffects
{
  kRingPair,
  kDoubleChaser,
  kTripleBounce,
  kWaveInt,
  kWave,
  kBlueSparkSlow,
  kBlueSparkFast,
  kWhiteSparkSlow,
  kWhiteSparkFast,
  kRainbow,
};


// ----------  S E R I A L - C O M M A N D S ------------

CmdMessenger cmdMessenger = CmdMessenger(Serial);

enum commands
{
  // Commands
  kAcknowledge,
  kError,
  kLedNumberRequest,
  kLedNumberResponse,
  kLightUpLed,
  kSetPlayerColor,
  kStartProgressBar,
  kShowEffect,
  kGetStatus,
  kSwitchOff,

};

void attachCommandCallbacks()
{
  // Attach callback methods
  cmdMessenger.attach(OnUnknownCommand);
  cmdMessenger.attach(kLightUpLed, OnkLightUpLed);
  cmdMessenger.attach(kSetPlayerColor, OnkSetPlayerColor);
  cmdMessenger.attach(kLedNumberRequest, OnkLedNumberRequest);
  cmdMessenger.attach(kStartProgressBar, OnStartup);
  cmdMessenger.attach(kShowEffect, OnkShowEffect);
  cmdMessenger.attach(kGetStatus, OnGetStatus);
  cmdMessenger.attach(kSwitchOff, OnSwitchOff);
}



// ---------------  C A L L B A C K S -----------------

/*
* Called when receiving a command with no binding
*/
void OnUnknownCommand()
{
  errorEffect();
  cmdMessenger.sendCmd(kError,"Command without attached callback");
}

/*
* Called on arduino startup
*/
void OnArduinoReady()
{
  cmdMessenger.sendCmd(kAcknowledge,"Arduino ready");
}

/*
* Switches on leds for a team
* ARG_1 number of leds
* ARG_2 team number
*/
void OnkLightUpLed()
{
  // Retreive parameter
  int16_t ledNum = cmdMessenger.readInt16Arg();
  int16_t playerNumber = cmdMessenger.readInt16Arg();
  int counter;
  switch (playerNumber){
    case 1:
      for(counter=0; counter<ledNum; counter++){
        *(team1Front + team1Leds + counter) = team1Color;
        *(team1Tail - team1Leds - counter) = team1Color;
      }
      team1Leds += counter;
      break;
    case 2:
      for(counter=0; counter<ledNum; counter++){
        *(team2Front + team2Leds + counter) = team2Color;
        *(team2Tail - team2Leds - counter) = team2Color;
      }
      team2Leds += counter;
      break;
    default:
      cmdMessenger.sendCmd(kError, "wrong player number");
      return;
  }
  FastLED.show();
  cmdMessenger.sendCmd(kAcknowledge, "leds are on");
}

/*
* Set team color (default team1Color = 0xff0000, team2Color = 0x0000ff)
* ARG_1 team number
* ARG_2 color (hex string)
*/
void OnkSetPlayerColor()
{
  int16_t playerNumber = cmdMessenger.readInt16Arg();
  unsigned long int color = strtoul(cmdMessenger.readStringArg(), NULL, 16);

  switch (playerNumber){
    case 1:
      team1Color = color;
      break;
    case 2:
      team2Color = color;
      break;
    default:
      cmdMessenger.sendCmd(kError, "wrong player number");
      errorEffect();
      return;
  }
  cmdMessenger.sendCmd(kAcknowledge, "player color ok");

}

/*
* Sends back the number of leds available to effectively store the score
*/
void OnkLedNumberRequest()
{
  cmdMessenger.sendCmd(kLedNumberResponse, ((NUM_LEDS-BAR_DISTANCE)/2)-2);

}

/*
* Initializes the progress bar; a call during a game resets the bar
*/
void OnStartup()
{
  FastLED.clear();
  *team1Front = team1Color;
  *team1Tail = team1Color;
  *team2Front = team2Color;
  *team2Tail = team2Color;
  team1Leds = 1;
  team2Leds = 1;
  showingProgressBar = true;
  FastLED.show();
  cmdMessenger.sendCmd(kAcknowledge, "setup completed");
}

/*
* Start a led effect
* ARG_1 portion of bar where to show the effect (1- player 1 bar, 2- player 2 bar, 3- whole bar)
* ARG_2 effect number
* ARG_3 duration (milliseconds)
*/
void OnkShowEffect()
{
  int portion = cmdMessenger.readInt16Arg();
  if(portion < 0 || portion > 2)
  {
    cmdMessenger.sendCmd(kError, "wrong portion");
    return;
  }
  animation[portion] = cmdMessenger.readInt16Arg();
  effectRunning[portion] = true;
  effectTimestamp[portion] = millis();
  effectDuration[portion] = cmdMessenger.readInt32Arg();
  cmdMessenger.sendCmd(kAcknowledge, "effect on");
}

void OnGetStatus()
{
  String response = "Arduino status:\n";
  response += "uptime: " + String(millis()) + "\n";
  response += "progress bar: " + String(showingProgressBar) + "\n";
  response += "team1Leds: " + String(team1Leds) + "\n";
  response += "team2Leds: " + String(team2Leds) + "\n";
  cmdMessenger.sendCmd(kAcknowledge, response);
}

void OnSwitchOff()
{
  for(int i=0; i<3; i++)
    effectRunning[i] = false;
  showingProgressBar = false;
  FastLED.clear();
  FastLED.show();
  cmdMessenger.sendCmd(kAcknowledge, "leds are off");
}


//##################################################
//##                    LED EFFECTS               ##
//##################################################

void restoreAfterEffect(int i)
{
  effectRunning[i] = false;
  effectTimestamp[i] = 0;
  frame[i] = 0;
  clearPortion(barPortion[i], effectLedsCount[i]);
  if(showingProgressBar)
    restoreProgressBar(i);
  FastLED.show();
  cmdMessenger.sendCmd(kAcknowledge, "effect stopped");
}

void restoreProgressBar(int i)
{
  switch (i)
  {
    case 0:
      fill_solid(team1Front, team1Leds, team1Color);
      fill_solid(team2Tail-team2Leds+1, team2Leds, team2Color);
      break;
    case 1:
      fill_solid(team2Front, team2Leds, team2Color);
      fill_solid(team1Tail-team1Leds+1, team1Leds, team1Color);
      break;
    case 2:
      fill_solid(team1Front, team1Leds, team1Color);
      fill_solid(team1Tail-team1Leds+1, team1Leds, team1Color);
      fill_solid(team2Front, team2Leds, team2Color);
      fill_solid(team2Tail-team2Leds+1, team2Leds, team2Color);
      break;
  }
}

void errorEffect()
{
  fill_solid(leds, 10, 0xFF0000);
  FastLED.show();
  delay(500);
  FastLED.clear();
  FastLED.show();
}

/*
* FastLED.clear() clear the whole bar, so this is more flexible
*/
void clearPortion(CRGB *first, int number)
{
  for(int i=0; i<number; i++)
    *(first+i) = CRGB::Black;
}



// ---------------  A G G R E G A T E -- A N I M A T I O N S -----------------

void TripleBounce(CRGB ledsArray[], uint16_t frame, int i)    //3 chaser animations offset by 120 degrees each
{
  clearPortion(barPortion[i], effectLedsCount[i]);    //Clear previous buffer
  Bounce(ledsArray,frame,0, i);
  Bounce(ledsArray,frame+(MAX_INT_VALUE/3),100, i);
  Bounce(ledsArray,frame+(MAX_INT_VALUE/3)*2,150, i);
}

void DoubleChaser(CRGB ledsArray[], uint16_t frame, int i)   //2 chaser animations offset 180 degrees
{
  clearPortion(barPortion[i], effectLedsCount[i]);    //Clear previous buffer
  frame = frame * 2;
  Ring(ledsArray, frame, 0, i);
  Ring(ledsArray, frame + (MAX_INT_VALUE / 2), 150, i);
}

void RingPair(CRGB ledsArray[], uint16_t frame, int i)     //2 rings animations at inverse phases
{
  clearPortion(barPortion[i], effectLedsCount[i]);   //Clear previous buffer
  Ring(ledsArray, frame, 30, i);
  Ring(ledsArray, MAX_INT_VALUE - frame, 150, i);
}


void RainbowSpark(CRGB targetleds[], uint16_t animationFrame,uint8_t fade, int i){    //Color spark where hue is function of frame
  Spark(targetleds,animationFrame,fade,animationFrame/255, i);
  delay(20);
}



// ---------------  P R I M I T I V E -- A N I M A T I O N S -----------------

//*********************     Bounce      ***************************
// Linear "Larson scanner" (or knight rider effect) with anti-aliasing
// Color is determined by "hue"
//*****************************************************************
void Bounce(CRGB targetleds[], uint16_t animationFrame, uint8_t hue, int i)
{
  uint16_t pos16;
  if (animationFrame < (MAX_INT_VALUE / 2))
  {
    pos16 = animationFrame * 2;

  }else{
    pos16 = MAX_INT_VALUE - ((animationFrame - (MAX_INT_VALUE/2))*2);
  }

  int position = map(pos16, 0, MAX_INT_VALUE, 0, ((effectLedsCount[i]) * 16));
  drawFractionalBar(targetleds, position, 3, hue,0, i);
}

//************************          Ring           ******************************
// Anti-aliased cyclical chaser, 3 pixels wide
// Color is determined by "hue"
//*****************************************************
void Ring(CRGB targetleds[], uint16_t animationFrame, uint8_t hue, int i)
{
  uint8_t ledsLength = effectLedsCount[i];
  int pos16 = map(animationFrame, 0, MAX_INT_VALUE, 0, ((ledsLength) * 16));
  drawFractionalBar(targetleds, pos16, 3, hue,1, i);
}

//***************************   Wave [Float Math]  *******************************
// Squeezed sine wave
// Uses slow, Arduino sin() function
// Squeezing achieved by using an exponential (^8) sin value
// Color is determined by "hue"
//***********************************************************************************
void Wave(CRGB targetleds[], uint16_t animationFrame, uint8_t hue, int num){
  clearPortion(barPortion[num], effectLedsCount[num]);    //Clear previous buffer
  float deg;
  float value;
  uint8_t ledsLength = effectLedsCount[num];
  for(uint8_t i=0;i<ledsLength;i++)
  {
    deg=float(animationFrame+((MAX_INT_VALUE/ledsLength)*i))/(float(MAX_INT_VALUE)) * 360.0;
    value = pow(sin(radians(deg)),8);    //Squeeeeeeze

    if(value>=0){   //Chop sine wave (no negative values)
      targetleds[i] += CHSV(hue,255,value*256);
    }
  }
}

//***************************   Wave [Integer Math]  *******************************
// unadulterated sine wave.
// Uses FastLED sin16() and no float math for efficiency.
// Since im stuck with integer values, exponential wave-forming is not possible (unless i'm wrong???)
// Color is determined by "hue"
//***********************************************************************************
void WaveInt(CRGB targetleds[], uint16_t animationFrame, uint8_t hue, int num){
  clearPortion(barPortion[num], effectLedsCount[num]);
  uint8_t ledsLength = effectLedsCount[num];
  uint8_t value;
  for(uint8_t i=0;i<ledsLength;i++)
  {
    value=(sin16(animationFrame+((MAX_INT_VALUE/ledsLength)*i)) + (MAX_INT_VALUE/2))/256;
    if(value>=0){
      targetleds[i] += CHSV(hue,255,value);
    }
  }
}

//********************************   Color Spark  ***********************************
// Color of the sparks is determined by "hue"
// Frequency of sparks is determined by global var "animateSpeed"
// "animateSpeed" var contrained from 1 - 255 (0.4% - 100%)
// "fade" parameter specifies dropoff (next frame brightness = current frame brightness * (x/256)
// fade = 256 means no dropoff, pixels are on or off
// NOTE: this animation doesnt clear the previous buffer because the fade/dropoff is a function of the previous LED state
//***********************************************************************************
void Spark(CRGB targetleds[], uint16_t animationFrame,uint8_t fade, uint8_t hue, int num){

  uint8_t ledsLength = effectLedsCount[num];
  uint16_t rand = random16();

    for(int i=0;i<ledsLength;i++)
    {
      targetleds[i].nscale8(fade);
    }


  if(rand < (MAX_INT_VALUE / (256 - (constrain(animateSpeed,1,256)))))  ;
  {
    targetleds[rand % ledsLength].setHSV(hue,255,255);
  }
}

//******************************       Spark       **********************************
// Same as color spark but no hue value, // in HSV white is any hue with 0 saturation
// Frequency of sparks is a percentage mapped to global var "animateSpeed"
// "animateSpeed" var contrained from 1 - 255 (0.4% - 100%)
// "fade" parameter specifies dropoff (next frame brightness = current frame brightness * (x/256)
// fade = 256 means no dropoff, pixels are on or off
// NOTE: this animation doesnt clear the previous buffer because the fade/dropoff is a function of the previous LED state
//***********************************************************************************
void Spark(CRGB targetleds[], uint16_t animationFrame,uint8_t fade, int num){
  uint8_t ledsLength = effectLedsCount[num];
  uint16_t rand = random16();

  for(int i=0;i<ledsLength;i++)
    {
      targetleds[i].nscale8(fade);
    }


  if(rand < (MAX_INT_VALUE / (256 - (constrain(animateSpeed,1,255)))))
  {
    targetleds[rand % ledsLength].setHSV(0,0,255);
  }

}

//Anti-aliasing code care of Mark Kriegsman Google+: https://plus.google.com/112916219338292742137/posts/2VYNQgD38Pw
void drawFractionalBar(CRGB targetleds[], int pos16, int width, uint8_t hue, bool wrap, int num)
{
  uint8_t ledsLength = effectLedsCount[num];
  uint8_t i = pos16 / 16; // convert from pos to raw pixel number

  uint8_t frac = pos16 & 0x0F; // extract the 'factional' part of the position
  uint8_t firstpixelbrightness = 255 - (frac * 16);

  uint8_t lastpixelbrightness = 255 - firstpixelbrightness;

  uint8_t bright;
  for (int n = 0; n <= width; n++) {
    if (n == 0) {
      // first pixel in the bar
      bright = firstpixelbrightness;
    }
    else if (n == width) {
      // last pixel in the bar
      bright = lastpixelbrightness;
    }
    else {
      // middle pixels
      bright = 255;
    }

    targetleds[i] += CHSV(hue, 255, bright );
    i++;
    if (i == ledsLength)
    {
      if (wrap == 1) {
        i = 0; // wrap around
      }
      else{
        return;
      }
    }
  }
}




//#####################################################
//##                       MAIN                      ##
//#####################################################

// Setup function
void setup()
{
  // Serial setup
  Serial.begin(9600);
  cmdMessenger.printLfCr();  // Adds newline to every command
  attachCommandCallbacks();  // Attach my application's user-defined callback methods

  // FASTLED
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness( BRIGHTNESS );
  FastLED.clear();
  FastLED.show();
  team1Front = leds;
  team1Tail = leds + NUM_LEDS - 1;
  team2Front = leds + ((NUM_LEDS-BAR_DISTANCE)/2)+BAR_DISTANCE;
  team2Tail = leds + ((NUM_LEDS-BAR_DISTANCE)/2) - 1;
  barPortion[0] = team1Front;
  barPortion[1] = team2Front;
  barPortion[2] = leds;
  cmdMessenger.sendCmd(kAcknowledge,"Arduino has started!"); // send ready ack

}

// Loop function
void loop()
{
  cmdMessenger.feedinSerialData();   // Process incoming serial data, and perform callbacks

  // Effect loop management
  for(int i=0; i<3; i++)
  {
    if(effectRunning[i] && abs(millis()) - abs(effectTimestamp[i]) >= abs(effectDuration[i]))
      restoreAfterEffect(i);
  }
  for (int i=0; i<3; i++)
  {
    if(effectRunning[i])
    {
      switch(animation[i])
      {
      case kRingPair:
        RingPair(barPortion[i], frame[i], i);
        FastLED.show();
        frame[i] += animateSpeed;
        break;
      case kDoubleChaser:
        DoubleChaser(barPortion[i],frame[i], i);
        FastLED.show();
        frame[i] += animateSpeed;
        break;
      case kTripleBounce:
        TripleBounce(barPortion[i],frame[i], i);
        FastLED.show();
        frame[i] += animateSpeed;
        break;
      case kWaveInt:
        WaveInt(barPortion[i],frame[i],180, i);
        FastLED.show();
        frame[i] += animateSpeed;
        break;
      case kWave:
        Wave(barPortion[i],frame[i],180, i);
        FastLED.show();
        frame[i] += animateSpeed;
        break;
      case kBlueSparkSlow:  //Blue spark (Slow)
        Spark(barPortion[i],frame[i],255,188, i);   //Overloaded version of "Spark" with Hue value, 255 for fade is the slowest fade possible. 256 is on/off
        delay(2);  //Slow things down a bit more for Slow Spark
        FastLED.show();
        frame[i] += animateSpeed;
        break;
      case kBlueSparkFast:  //Blue spark (fast)
        Spark(barPortion[i],frame[i],246,188, i);  //Overloaded version of "Spark" with Hue value, 246 fade is faster which makes for a sharper dropoff
        FastLED.show();
        frame[i] += animateSpeed;
        break;
      case kWhiteSparkSlow:  //White spark (Slow)
        Spark(barPortion[i],frame[i],255, i);  //"Spark" function without hue make a white spark, 255 for fade is the slowest fade possible.
        delay(2);  //Slow things down a bit more for Slow Spark
        FastLED.show();
        frame[i] += animateSpeed;
        break;
      case kWhiteSparkFast:  //White spark (fast) "Spark" function without hue make a white spark, 246 fade is faster which makes for a sharper dropoff
        Spark(barPortion[i],frame[i],245, i);
        FastLED.show();
        frame[i] += animateSpeed;
        break;
      case kRainbow:
        RainbowSpark(barPortion[i],frame[i],240, i);  //240 for dropoff is a pretty sharp fade, good for this animation
        FastLED.show();
        frame[i] += animateSpeed;
        break;
      default:
        cmdMessenger.sendCmd(kError, "wrong effect number");
        restoreAfterEffect(i);
      }

    }
  }

}

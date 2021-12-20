#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#ifdef __AVR__
  #include <avr/power.h>
#endif

// definition of switches
#define estrobo 7
#define auto 2

#define staticLed 8
#define bpmFade 24
#define bpmLed 26
#define modoColor 20
#define turboColors 22
#define randomColorAuto 12

// defiticion of potenciometers
#define R 3
#define G 4
#define B 5

// definition of leds
#define ledStrip 6

// definition of analizer vars
int maxVolt = 0, preVolt = 0, volt = 0; 

const int sensorPin = A0;
const int sampleWindow = 50;

// ledStrip
const int numPixels = 150;

int pixelFormat = NEO_GRB + NEO_KHZ800;
Adafruit_NeoPixel*pixels;

#define DELAYVAL 25 
#define BRIGHTNESS 255


// definition of variables
int opt, r, g, b, finalR, finalG, finalB;
bool autoMode;

// switch array
int arrayPins[]  = {auto, randomColorAuto, staticLed, bpmFade, bpmLed, modoColor, turboColors};

// color definitions
int red[] = {255, 0, 0};
int green[] = {0, 255, 0};
int blue[] = {0, 0, 255};
int purple[] = {240, 0, 255};
int actualColor[] = {240,0,0};

int colors[][3] = {{0,0,255},{0,255,0},{255,0,0},{255,255,0},{255,0,255},{0,255,255},{255,255,255},{255,204,0},
  {255,0,204},{0,255,204},{0,204,255},{204,255,0},{204,0,255},{255,102,0},{255,0,102},{102,0,255},{102,255,0},
  {0,255,102},{0,102,255},{128,0,128},{153,204,255},{153,255,204},{255,204,153}};


// other definitions
int estroboWindow = 40;
long lastChange = 0;

// metodos globales

void staticMode(int c[]) {
  for (int i = 0; i < numPixels; i++) {
    pixels -> setPixelColor(i, pixels -> Color(c[0], c[1], c[2]));
  }
  pixels -> show();
}

void readRGB() {
  r = map(analogRead(R), 0, 1023, 0, 255);
  g = map(analogRead(G), 0, 1023, 0, 255);
  b = map(analogRead(B), 0, 1023, 0, 255);
}

void setFinalColors() {
  finalR = r;
  finalG = g;
  finalB = b;
}

void autoColors() {
  readRGB();
  setFinalColors();
}

void clearStrip() {
  pixels -> clear();
  pixels -> show();
} 

//setup methods 

void maxBright(int c[]) {
  pixels -> setBrightness(255);
  pixels -> show();
}

void turboLed(int c[]) {
  
  maxBright(c);

  int cont = 0;
  
  while (cont < numPixels) {

    pixels -> clear();

    for (int i = cont; i < cont + 5; i++) {
      pixels -> setPixelColor(i, pixels -> Color(c[0], c[1], c[2]));
    }
    cont += 1;
    pixels -> show();
    delay(0.05);
  }
}

void initialSequence() {
  turboLed(red);
  turboLed(green);
  turboLed(blue);
  clearStrip();
} 

// auto methods
void autoLeds() {
    unsigned long startMillis = millis();
   unsigned int signalMax = 0;
   unsigned int signalMin = 1024;
   unsigned int sample;

   while (millis() - startMillis < sampleWindow)
   {
      sample = analogRead(sensorPin);

      if (sample < 1024)
      {
         if (sample > signalMax)
         {
            signalMax = sample; 
         }
         else if (sample < signalMin)
         {
            signalMin = sample;  
         }
      }
   }
   unsigned int peakToPeak = signalMax - signalMin; 

   preVolt = volt;
   volt = (peakToPeak * 5);  
   
   if (volt > maxVolt) {
    maxVolt = volt; 
   }

   pixels -> setBrightness(map(volt, 0, maxVolt, 0 , 255));
   pixels -> show();

}

// metodos modo manual

void fullManualStaticMode() {
  actualColor[0] = map(analogRead(A2), 0, 1023, 0, 255);
  actualColor[1] = map(analogRead(A3), 0, 1023, 0, 255);
  actualColor[2] = map(analogRead(A4), 0, 1023, 0, 255);

  staticMode(actualColor);
  pixels -> setBrightness(map(analogRead(A1), 0, 1023, 0 , 255));
  
}

void modoEstrobo(int c[]) {
  staticMode(c);
  pixels -> setBrightness(0);
  pixels -> show();
  delay(estroboWindow);
  staticMode(c);
  pixels -> setBrightness(255);
  pixels -> show();
  delay(estroboWindow);
}

void colorCarrousel() {
 
  long decision = random(0, 1000);

  if (((millis() - lastChange) > 2500 && decision < 50) || (millis() - lastChange) > 10000) {

    int auxN = random(0, 22);

    actualColor[0] = colors[auxN][0];
    actualColor[1] = colors[auxN][1];
    actualColor[2] = colors[auxN][2];

    lastChange = millis();

  }
}

void colorCarrouselNormal() {

  int auxNormalCarr = random(0, 22);

  actualColor[0] = colors[auxNormalCarr][0];
  actualColor[1] = colors[auxNormalCarr][1];
  actualColor[2] = colors[auxNormalCarr][2];

  lastChange = millis();

  staticMode(actualColor);

  delay(random(500,2000));
}

void readColors() {
  int r = map(analogRead(A2), 0, 1023, 0, 255);
  int g = map(analogRead(A3), 0, 1023, 0, 255);
  int b = map(analogRead(A4), 0, 1023, 0, 255);

  if (r > 15) {
    actualColor[0] = r; 
  } else {
    actualColor[0] = 0;
  }

  if (g > 15) {
    actualColor[1] = g; 
  } else {
    actualColor[1] = 0;
  }

  if (b > 15) {
    actualColor[2] = b; 
  } else {
    actualColor[2] = 0;
  }
}



// setup
void setup() {

  #if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
    clock_prescale_set(clock_div_1);
  #endif

  pixels = new Adafruit_NeoPixel(numPixels, ledStrip, pixelFormat);
  pixels -> begin();
  pixels -> setBrightness(255);
  pixels -> clear();

  // setup switches
  pinMode(estrobo, INPUT);
  pinMode(auto, INPUT);

  // setup potenciometers
  pinMode(R, INPUT);
  pinMode(G, INPUT);
  pinMode(B, INPUT);

  // setup ledStrip
  pinMode(ledStrip, OUTPUT);

  // initial sequence 
  initialSequence();

  actualColor[0] = 220;
  actualColor[1] = 0;
  actualColor[2] = 255;
  staticMode(actualColor);

  Serial.begin(9600);
}

// loop 
void loop() {

  if (digitalRead(10)) {
    modoEstrobo(actualColor);
  } else if(digitalRead(9)) {
    staticMode(actualColor);
    if (digitalRead(8)) {
      colorCarrousel();
    }
    autoLeds();
  } else {
    //inicio del modo manual
    if (digitalRead(4)) {
      turboLed(actualColor);
    } else if (!digitalRead(3)) {
      colorCarrouselNormal();
    } else if (!digitalRead(2)){
      fullManualStaticMode();
    } else {
      pixels -> setBrightness(0);
    }
  }
}
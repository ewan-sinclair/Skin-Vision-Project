#include "Vision_Project.h"

typedef struct {
  int associatedPin;
  long nextTimeMicros;
  long periodMicros;
  long vibeCount;
  int freqHz; //Also used to determine whether the pixel is active
  boolean flipState;
  long lastOpTimeMicros;
  enum strike_states strikeState;
} Pixel;

int pinAssociations[]={4,5,6,7}; //Mappings from pixel array index to pin
Pixel pixels[NUM_PIXELS]; //This holds all the pixels representing the physical coils

long micros_32Hz = 31250; //Period for 32Hz
int framerateHz = 2;
long nextFlipMicros = -1;
long nextFrameMicros = -1;
long nextVibeCountFrameMicros = -1;

void strike(Pixel *pixel);
boolean isReady(Pixel *pixel);
boolean setPixel(Pixel *pixel, boolean newState);
void processStrike(Pixel *pixel);
void strike(Pixel *pixel);

void setup(){
  
  Serial.begin(9600);
  Pixel *pixel;
  for(int currentPixel=0; currentPixel<NUM_PIXELS; currentPixel++){
    pixel = &pixels[currentPixel];
    pixel->nextTimeMicros = -1;
    pixel->vibeCount = 0;
    pixel->freqHz = 32;
    pixel->associatedPin = pinAssociations[currentPixel];
    pixel->flipState = false;
    pixel->lastOpTimeMicros = -1;
    pinMode(pixel->associatedPin, OUTPUT);
    digitalWrite(pixel->associatedPin, LOW);
  }

}

void loop(){
  //Enable what should be on
  //pScanRow();

  //Vibrate any pixel that should be vibrated
  doToggles();
  //vibeCounts();
  
  pvHorizMovingLine();
  
}

/* Flips pixels which require flipping.
 * Implemented to work with a fixed 32Hz vibration frequency,
 * which does not allow for individual vibration or striking. */
void doToggles(){
  long now = micros();
  long nowMillis = millis();
  if(nextFlipMicros < now-micros_32Hz) nextFlipMicros = now; // In case the counter has drifted off
  if(now >= nextFlipMicros){
    nextFlipMicros += micros_32Hz;
    Pixel *pixel;
    for(int currentPixel=0; currentPixel<NUM_PIXELS; currentPixel++){
      pixel = &pixels[currentPixel];
      if(pixel->freqHz > 0){
        pixel->flipState = !pixel->flipState;
        digitalWrite(pixel->associatedPin, pixel->flipState==true? HIGH : LOW);
        pixel->vibeCount++;
      }
    }
  }
}

//Prints information about how many times each pixel has vibrated
void vibeCounts(){
  long now = micros();
  if(nextVibeCountFrameMicros < now-(1000000/framerateHz)) nextVibeCountFrameMicros = now;
  if( now >= nextVibeCountFrameMicros ) {
    nextVibeCountFrameMicros += (1000000/framerateHz);
    Serial.print("[");
    Pixel *pixel;
    for(int count=0; count < NUM_PIXELS; count++){
      pixel = &pixels[count];
      Serial.print(pixel->vibeCount);
      Serial.print(", ");
    }
    Serial.println("]"); 
  }
}

// Causes a coil to perform a quick strike. This will cancel any existing vibration, and do nothing if a strike is in progress.
// Strike states go FIRE -> NEUTRAL. Once at NEUTRAL, the pixel is set false and stays there.
void strike(Pixel *pixel) {
  pixel->freqHz = 0;
  pixel->strikeState=FIRE;
}

//Resolves any strikes to be performed on the given pixel
void processStrike(Pixel *pixel) {
    if(pixel->strikeState == NEUTRAL && pixel->flipState==true) {
      setPixel(pixel, false);
    } else if (pixel->strikeState == FIRE) {
      if(pixel->flipState == true) setPixel(pixel, false);
      else if (setPixel(pixel, true)) pixel->strikeState = NEUTRAL;
    }
}

//Sets a pixel's state, doing appropriate housekeeping and failing if the operation is happening too soon after another flip.
boolean setPixel(Pixel *pixel, boolean newState) {
  if (isReady(pixel)) return false;
  if (  pixel->flipState == newState) return true;
  pixel->lastOpTimeMicros = micros();
  pixel->flipState = newState;
  digitalWrite(pixel->associatedPin, newState);
  return true;
}

//Returns true if the pixel is ready to change state, false otherwise.
boolean isReady(Pixel *pixel) {
 return  micros() - pixel->lastOpTimeMicros < SAFE_PERIOD_MICROS;
}

/* Causes the pixels in the given indexes to strike at the same time */
void strikePixels(int *input, int num) {
  int i;
  Serial.print("Striking pixels: ");
  for (i=0; i < num; i++){
    Serial.print(*input);
    Serial.print(", ");
    processStrike(&pixels[*input]);
    input++;
  }
  Serial.println("");
}

/* Causes the pixels in the given indexes to vibrate for a given number of milliseconds */
void vibratePixels(int *input, int num, int freqHz, long duration){
  long startTime = millis();
  int i;
  int* index = input;
  Serial.print("Vibrating pixels: ");
  for (i=0; i < num; i++){
    Serial.print(*index);
    Serial.print(", ");
    pixels[*index].freqHz = freqHz;
    index++;
  }
  Serial.println("");
  
  while (millis() - startTime <= duration) {
    doToggles();
  }
  
  index = input;
  for (i=0; i < num; i++){
    pixels[*index].freqHz = 0;
    index++;
  }  
  
}

/* #################### Test Pattern Definitions ##################### */

/* Moves a single pixel in a vertical zigzag over the entire array */
void psVerticalZigZag() {
	int delayMillis = FRAME_DELAY_MILLIS;
	
	int a[] = {0};
	strikePixels(&a[0],1);
	delay(delayMillis);

	int a0[] = {5};
	strikePixels(&a0[0],1);
	delay(delayMillis);
	
	int a1[] = {10};
	strikePixels(&a1[0], 1);
	delay(delayMillis);

	int a2[] = {15};
	strikePixels(&a2[0], 1);
	delay(delayMillis);

	int a3[] = {20};
	strikePixels(&a3[0], 1);
	delay(delayMillis);

	int a4[] = {21};
	strikePixels(&a4[0], 1);
	delay(delayMillis);

	int a5[] = {16};
	strikePixels(&a5[0], 1);
	delay(delayMillis);

	int a6[] = {11};
	strikePixels(&a6[0], 1);
	delay(delayMillis);

	int a7[] = {6};
	strikePixels(&a7[0], 1);
	delay(delayMillis);

	int a8[] = {1};
	strikePixels(&a8[0], 1);
	delay(delayMillis);

	int a9[] = {2};
	strikePixels(&a9[0], 1);
	delay(delayMillis);

	int a10[] = {7};
	strikePixels(&a10[0], 1);
	delay(delayMillis);

	int a11[] = {12};
	strikePixels(&a11[0], 1);
	delay(delayMillis);

	int a12[] = {17};
	strikePixels(&a12[0], 1);
	delay(delayMillis);

	int a13[] = {22};
	strikePixels(&a13[0], 1);
	delay(delayMillis);

	int a14[] = {23};
	strikePixels(&a14[0], 1);
	delay(delayMillis);

	int a15[] = {18};
	strikePixels(&a15[0], 1);
	delay(delayMillis);

	int a16[] = {13};
	strikePixels(&a16[0], 1);
	delay(delayMillis);

	int a17[] = {8};
	strikePixels(&a17[0], 1);
	delay(delayMillis);

	int a18[] = {3};
	strikePixels(&a18[0], 1);
	delay(delayMillis);

	int a19[] = {4};
	strikePixels(&a19[0], 1);
	delay(delayMillis);

	int a20[] = {9};
	strikePixels(&a20[0], 1);
	delay(delayMillis);

	int a21[] = {14};
	strikePixels(&a21[0], 1);
	delay(delayMillis);

	int a22[] = {19};
	strikePixels(&a22[0], 1);
	delay(delayMillis);

	int a23[] = {24};
	strikePixels(&a23[0], 1);
	delay(delayMillis);
}

/* Moves a vertical line of strikes from left to right across the array */
void psHorizMovingLine() {
  int delayMillis = 400;
  int a1[] = {0,5,10,15,20};
  int a2[] = {1,6,11,16,21};
  int a3[] = {2,7,12,17,22};
  int a4[] = {3,8,13,18,23};
  int a5[] = {4,9,14,19,24};
  strikePixels(&a1[0],5);
  delay(delayMillis);
  strikePixels(&a2[0],5);
  delay(delayMillis);
  strikePixels(&a3[0],5);
  delay(delayMillis);
  strikePixels(&a4[0],5);
  delay(delayMillis);
  strikePixels(&a5[0],5);
  delay(delayMillis);
}

/* Moves a vertical line of vibrations from left to right across the array */
void pvHorizMovingLine(){
  int delayMillis = 400;
  int a1[] = {0,5,10,15,20};
  int a2[] = {1,6,11,16,21};
  int a3[] = {2,7,12,17,22};
  int a4[] = {3,8,13,18,23};
  int a5[] = {4,9,14,19,24};
  vibratePixels(&a1[0],5, 16, delayMillis);
  vibratePixels(&a2[0],5, 16, delayMillis);
  vibratePixels(&a3[0],5, 16, delayMillis);
  vibratePixels(&a4[0],5, 16, delayMillis);
  vibratePixels(&a5[0],5, 16, delayMillis);
}

/* Scans vibrations along a single row of pixels*/
void pvScanRow(){
  long now = micros();
  if(nextFrameMicros < now-(1000000/framerateHz)) nextFrameMicros = now;
  if( now >= nextFrameMicros ) {
    int newActivePixel;
    nextFrameMicros += (1000000/framerateHz);
    Pixel * pixel;
    Pixel * nextPixel;
    for(int currentPixel=0; currentPixel<NUM_PIXELS; currentPixel++){ 
      pixel = &pixels[currentPixel];
      if(pixel->freqHz > 0) {
        newActivePixel = (currentPixel+1 < NUM_PIXELS) ? currentPixel+1 : 0;
        nextPixel = &pixels[newActivePixel];
        pixel->freqHz = 0;
        nextPixel->freqHz = 32;
        Serial.print(currentPixel);
        Serial.print(" -> ");
        Serial.println(newActivePixel);
        break;
      }
    }
  }
}


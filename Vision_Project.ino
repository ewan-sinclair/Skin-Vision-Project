#include "Vision_Project.h"

typedef struct {
  int associatedPin;
  long nextTimeMicros;
  long periodMicros;
  long vibeCount;
  int freqHz; //Also used to determine whether the pixel is active
  boolean flipState; //Tracks whether the pixel is down (true) or up (false)
  long lastOpTimeMicros; //Tracks when the pixel last changed state
} Pixel;

int pinAssociations[]={ 3,4,5,6,7, 8,9,10,11,12, 52,50,48,46,44, 40,38,36,34,32, 53,51,49,47,45 }; //Mappings from pixel array index to pin
Pixel pixels[NUM_PIXELS]; //This holds all the pixels representing the physical coils

long micros_32Hz = 31250; //Period for 32Hz
int framerateHz = 2;
long nextFlipMicros = -1;
long nextFrameMicros = -1;
long nextVibeCountFrameMicros = -1;

void strike(Pixel *pixel);
boolean isReady(Pixel *pixel);
boolean setPixel(Pixel *pixel, boolean newState);

void setup(){
  
  Serial.begin(9600);
  resetArray();

}

void loop(){
  //Enable what should be on
  //pScanRow();

  //Vibrate any pixel that should be vibrated
  doToggles();
  //vibeCounts();

  Serial.println("pvVerticalZigzag");
  pvVerticalZigZag(400, 16);
  Serial.println("pvHorizLine");
  pvHorizMovingLine(400, 16);
  Serial.println("psHorizLine");
  psHorizMovingLine(400);
  Serial.println("psZigZag");
  psVerticalZigZag(200);
  Serial.println("psDiagLine 0");
  psDiagonalMovingLine(0, 400);
  Serial.println("psDiagLine 1");
  psDiagonalMovingLine(1, 400);
  Serial.println("pvDiagLine 0");
  pvDiagonalMovingLine(0, 400, 16);
  Serial.println("pvDiagLine 1");
  pvDiagonalMovingLine(1, 400, 16);

  
//  pvAll(1000, 1);
  
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

/* Causes the pixels in the given indexes to strike at the same time
returns the number of milliseconds consumed by the strike call. */
int strikePixels(int *input, int num, long delayMillis) {
  
  long start = millis();
  int i;
  int *index = input;
  boolean moving = false;
  Serial.print("Striking pixels: ");
  for (i=0; i < num; i++){
    moving = moving || setPixel(&pixels[*input], false);
    index++;
  }
  
  if(moving) delay(32); //Time for the pins to reset if they hadn't
  index = input;

  for (i=0; i < num; i++){
    Serial.print(*index);
    Serial.print(", ");
    setPixel(&pixels[*index], true);
    index++;
  }
  
  delay(STRIKE_DOWN_TIME);
  index = input;
  
  for (i=0; i < num; i++){
    setPixel(&pixels[*index], false);
    index++;
  }

  Serial.println("");
  long millisElapsed = millis() - start;
  long remainingMillis = delayMillis - millisElapsed;
  if(remainingMillis > 0) {
    delay(remainingMillis);
    return delayMillis;
  } else {
    return millisElapsed;
  }
}

void resetArray() {
  Pixel *pixel;
  for(int currentPixel=0; currentPixel<NUM_PIXELS; currentPixel++){
    pixel = &pixels[currentPixel];
    pixel->nextTimeMicros = -1;
    pixel->vibeCount = 0;
    pixel->freqHz = 0;
    pixel->associatedPin = pinAssociations[currentPixel];
    pixel->flipState = false;
    pixel->lastOpTimeMicros = -1;
    pinMode(pixel->associatedPin, OUTPUT);
    digitalWrite(pixel->associatedPin, LOW);
  }
}

/* Causes the pixels in the given indexes to vibrate for a given number of milliseconds */
void vibratePixels(int *input, int num, int freqHz, long durationMillis){
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
  
  while (millis() - startTime <= durationMillis) {
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
void psVerticalZigZag(int delayMillis) {
	
	int a[] = {0};
	strikePixels(&a[0],1, delayMillis);
	int a0[] = {5};
	strikePixels(&a0[0],1, delayMillis);	
	int a1[] = {10};
	strikePixels(&a1[0], 1, delayMillis);
	int a2[] = {15};
	strikePixels(&a2[0], 1, delayMillis);
	int a3[] = {20};
	strikePixels(&a3[0], 1, delayMillis);
	int a4[] = {21};
	strikePixels(&a4[0], 1, delayMillis);
	int a5[] = {16};
	strikePixels(&a5[0], 1, delayMillis);
	int a6[] = {11};
	strikePixels(&a6[0], 1, delayMillis);
	int a7[] = {6};
	strikePixels(&a7[0], 1, delayMillis);
	int a8[] = {1};
	strikePixels(&a8[0], 1, delayMillis);
	int a9[] = {2};
	strikePixels(&a9[0], 1, delayMillis);
	int a10[] = {7};
	strikePixels(&a10[0], 1, delayMillis);
	int a11[] = {12};
	strikePixels(&a11[0], 1, delayMillis);
	int a12[] = {17};
	strikePixels(&a12[0], 1, delayMillis);
	int a13[] = {22};
	strikePixels(&a13[0], 1, delayMillis);
	int a14[] = {23};
	strikePixels(&a14[0], 1, delayMillis);
	int a15[] = {18};
	strikePixels(&a15[0], 1, delayMillis);
	int a16[] = {13};
	strikePixels(&a16[0], 1, delayMillis);
	int a17[] = {8};
	strikePixels(&a17[0], 1, delayMillis);
	int a18[] = {3};
	strikePixels(&a18[0], 1, delayMillis);
	int a19[] = {4};
	strikePixels(&a19[0], 1, delayMillis);
	int a20[] = {9};
	strikePixels(&a20[0], 1, delayMillis);
	int a21[] = {14};
	strikePixels(&a21[0], 1, delayMillis);
	int a22[] = {19};
	strikePixels(&a22[0], 1, delayMillis);
	int a23[] = {24};
	strikePixels(&a23[0], 1, delayMillis);
}
/* Moves a single pixel in a vertical zigzag over the entire array */
void pvVerticalZigZag(int frameDelayMillis, int freqHz) {
	
	int a[] = {0};
	vibratePixels(&a[0],1, freqHz, frameDelayMillis);
	int a0[] = {5};
	vibratePixels(&a0[0],1, freqHz, frameDelayMillis);
	int a1[] = {10};
	vibratePixels(&a1[0], 1, freqHz, frameDelayMillis);
	int a2[] = {15};
	vibratePixels(&a2[0], 1, freqHz, frameDelayMillis);
	int a3[] = {20};
	vibratePixels(&a3[0], 1, freqHz, frameDelayMillis);
	int a4[] = {21};
	vibratePixels(&a4[0], 1, freqHz, frameDelayMillis);
	int a5[] = {16};
	vibratePixels(&a5[0], 1, freqHz, frameDelayMillis);
	int a6[] = {11};
	vibratePixels(&a6[0], 1, freqHz, frameDelayMillis);
	int a7[] = {6};
	vibratePixels(&a7[0], 1, freqHz, frameDelayMillis);
	int a8[] = {1};
	vibratePixels(&a8[0], 1, freqHz, frameDelayMillis);
	int a9[] = {2};
	vibratePixels(&a9[0], 1, freqHz, frameDelayMillis);
	int a10[] = {7};
	vibratePixels(&a10[0], 1, freqHz, frameDelayMillis);
	int a11[] = {12};
	vibratePixels(&a11[0], 1, freqHz, frameDelayMillis);
	int a12[] = {17};
	vibratePixels(&a12[0], 1, freqHz, frameDelayMillis);
	int a13[] = {22};
	vibratePixels(&a13[0], 1, freqHz, frameDelayMillis);
	int a14[] = {23};
	vibratePixels(&a14[0], 1, freqHz, frameDelayMillis);
	int a15[] = {18};
	vibratePixels(&a15[0], 1, freqHz, frameDelayMillis);
	int a16[] = {13};
	vibratePixels(&a16[0], 1, freqHz, frameDelayMillis);
	int a17[] = {8};
	vibratePixels(&a17[0], 1, freqHz, frameDelayMillis);
	int a18[] = {3};
	vibratePixels(&a18[0], 1, freqHz, frameDelayMillis);
	int a19[] = {4};
	vibratePixels(&a19[0], 1, freqHz, frameDelayMillis);
	int a20[] = {9};
	vibratePixels(&a20[0], 1, freqHz, frameDelayMillis);
	int a21[] = {14};
	vibratePixels(&a21[0], 1, freqHz, frameDelayMillis);
	int a22[] = {19};
	vibratePixels(&a22[0], 1, freqHz, frameDelayMillis);
	int a23[] = {24};
	vibratePixels(&a23[0], 1, freqHz, frameDelayMillis);
}
/* Moves a vertical line of strikes from left to right across the array */
void psHorizMovingLine(int frameDelayMillis) {
  int a1[] = {0,5,10,15,20};
  int a2[] = {1,6,11,16,21};
  int a3[] = {2,7,12,17,22};
  int a4[] = {3,8,13,18,23};
  int a5[] = {4,9,14,19,24};
  strikePixels(&a1[0],5, frameDelayMillis);
  strikePixels(&a2[0],5, frameDelayMillis);
  strikePixels(&a3[0],5, frameDelayMillis);
  strikePixels(&a4[0],5, frameDelayMillis);
  strikePixels(&a5[0],5, frameDelayMillis);
}

/* Moves a vertical line of vibrations from left to right across the array */
void pvHorizMovingLine(int frameDelay, int freqHz){
  int a1[] = {0,5,10,15,20};
  int a2[] = {1,6,11,16,21};
  int a3[] = {2,7,12,17,22};
  int a4[] = {3,8,13,18,23};
  int a5[] = {4,9,14,19,24};
  vibratePixels(&a1[0],5, freqHz, frameDelay);
  vibratePixels(&a2[0],5, freqHz, frameDelay);
  vibratePixels(&a3[0],5, freqHz, frameDelay);
  vibratePixels(&a4[0],5, freqHz, frameDelay);
  vibratePixels(&a5[0],5, freqHz, frameDelay);
}

void psDiagonalMovingLine(int direction, long delayMillis) {

  resetArray(); 
  
  int rows[][5] = {
    {0},
    {5,1},
    {10,6,2},
    {15,11,7,3},
    {20,16,12,8,4},
    {21,17,13,9},
    {22,18,14},
    {23,19},
    {24}
  };
  
  int sizes[] = {1,2,3,4,5,4,3,2,1};

  int i;  
  if(direction > 0){ 
    for(i=0; i<9; i++){
      strikePixels(&rows[i][0], sizes[i], delayMillis);
    }
  } else {
    for(i=8; i>=0; i--){
      strikePixels(&rows[i][0], sizes[i], delayMillis);
    }
    
  }
  
}

void pvDiagonalMovingLine(int direction, long delayMillis, int freqHz) {
  
  int rows[][5] = {
    {0},
    {5,1},
    {10,6,2},
    {15,11,7,3},
    {20,16,12,8,4},
    {21,17,13,9},
    {22,18,14},
    {23,19},
    {24}
  };
  
  int sizes[] = {1,2,3,4,5,4,3,2,1};

  int i;  
  if(direction > 0){ 
    for(i=0; i<9; i++){
      vibratePixels(&rows[i][0], sizes[i], freqHz, delayMillis);
    }
  } else {
    for(i=8; i>=0; i--){
      vibratePixels(&rows[i][0], sizes[i], freqHz, delayMillis);
    }
    
  }
  
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

/* Vibrates all pixels in the array */
void pvAll(long frameDelay, int freqHz) {
  int a1[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24};
  vibratePixels(&a1[0], 25, freqHz, 1000);
}

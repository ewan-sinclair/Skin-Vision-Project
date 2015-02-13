#define NUM_PIXELS 4
#define PERIOD_32HZ_MICROS 31250

typedef struct {
  int associatedPin;
  long nextTimeMicros;
  long periodMicros;
  long vibeCount;
  int freqHz; //Also used to determine whether the pixel is active
} Pixel;

int pinAssociations[]={4,5,6,7}; //Mappings from pixel array index to pin
Pixel pixels[NUM_PIXELS]; //This holds all the pixels representing the physical coils

long micros_32Hz = 31250; //Period for 32Hz
int framerateHz = 2;
boolean currentFlipState = true;
long nextFlipMicros = -1;
long nextFrameMicros = -1;
long nextVibeCountFrameMicros = -1;

void setup(){
  
  Serial.begin(9600);
  Pixel *pixel;
  for(int currentPixel=0; currentPixel<NUM_PIXELS; currentPixel++){
    pixel = &pixels[currentPixel];
    pixel->nextTimeMicros = -1;
    pixel->vibeCount = 0;
    pixel->freqHz = 32;
    pixel->associatedPin = pinAssociations[currentPixel];
    pinMode(pixel->associatedPin, OUTPUT);
    digitalWrite(pixel->associatedPin, LOW);
  }

}

void loop(){
  //Enable what should be on
  scanAll();

  //Vibrate any pixel that should be vibrated
  doToggles();
  vibeCounts();
}

void doToggles(){
  long now = micros();
  long nowMillis = millis();
  if(nextFlipMicros < now-micros_32Hz) nextFlipMicros = now; // In case the counter has drifted off
  if(now >= nextFlipMicros){
    nextFlipMicros += micros_32Hz;
    currentFlipState = !currentFlipState;
    for(int currentPixel=0; currentPixel<NUM_PIXELS; currentPixel++){
      if(pixels[currentPixel].freqHz > 0){
        digitalWrite(pixels[currentPixel].associatedPin, currentFlipState==true? HIGH : LOW);
        pixels[currentPixel].vibeCount++;
      }
    }
  }
}

void scanAll(){
  long now = micros();
  if(nextFrameMicros < now-(1000000/framerateHz)) nextFrameMicros = now;
  if( now >= nextFrameMicros ) {
    int newActivePixel;
    //Serial.println("");
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

      //      Serial.print("[");
      //      for(int count=0; count < NUM_PIXELS; count++){
      //        Serial.print(activePixels[count]);
      //        Serial.print(", ");
      //      }
      //      Serial.print("]");
    }
    //Serial.println("");
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

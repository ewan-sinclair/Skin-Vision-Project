#define NUM_PIXELS 4
#define PERIOD_32HZ_MICROS 31250
#define SAFE_PERIOD_MICROS 31250

enum strike_states { NEUTRAL, FIRE };

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
  scanAll();

  //Vibrate any pixel that should be vibrated
  doToggles();
  vibeCounts();
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

void scanAll(){
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

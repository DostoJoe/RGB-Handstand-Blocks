/*
 * 
 * RGB controller for Arduino
 * 
 * Program uses 2 buttons to select between SOLID colours and FADE colours.
 * 
 */
 
  // Set RGB LED pins
const int redPin = 13;
const int greenPin = 12;
const int bluePin = 11;

  // Set button pins
const int button1 = 2;
const int button2 = 3;

  // Initialise counters for button presses
int counter1 = 0;
int prevCounter1 = 0;
int counter2 = 0;
int prevCounter2 = 0;

  // interrupt
volatile int b1Val = 0;
volatile int b2Val = 0;

  // Colour Arrays
int black[3]  = { 0, 0, 0 };
int white[3]  = { 100, 100, 100 };
int red[3]    = { 100, 0, 0 };
int green[3]  = { 0, 100, 0 };
int blue[3]   = { 0, 0, 100 };
int yellow[3] = { 40, 95, 0 };
int dimWhite[3] = { 30, 30, 30 };

  // Set initial color
int redVal = black[0];
int greenVal = black[1]; 
int blueVal = black[2];

  // delay variables
int wait = 10; // stop for *ms between change of colour
int hold = 1000; // stop for *ms when colour is reached

  // Initialize color variables
int prevR = redVal;
int prevG = greenVal;
int prevB = blueVal;

int exitLoop = 0;

void setup() {
  // Set RGB pins as outputs
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  // Set button pins as inputs
  pinMode(button1, INPUT);
  pinMode(button2, INPUT);

  attachInterrupt(0, ISR_b1, FALLING);
  attachInterrupt(1, ISR_b2, FALLING);
}

void loop() {

switch(b1Press(counter1)){

case 1:
  crossFade(white);
  crossFade(black);
  break;

case 2:
  crossFade(red);
  crossFade(black);
  break;

case 3:
  crossFade(green);
  crossFade(black);
  break;

case 4:
  crossFade(blue);
  crossFade(black);
  break;
  
default:
  // do nothing
  break;
}

switch2:
switch(b2Press(counter2)){

case 1:
  solidCol(white);
  break;

case 2:
  solidCol(red);
  break;

case 3:
  solidCol(green);
  break;

case 4:
  solidCol(blue);
  break;

default:
  // do nothing
  break;
}
}

int b1Press(int buttonPress){
  int counter;
  int b1Val = digitalRead(button1);
  if(b1Val == HIGH){
  counter++;
  }
  return counter;
}

int b2Press(int buttonPress){
  int counter;
  int b2Val = digitalRead(button2); // read
  if(b2Val == HIGH){
  counter++;
  }
  return counter;
}


int calculateStep(int prevValue, int endValue) {
  int step = endValue - prevValue; // What's the overall gap?
  if (step) {                      // If its non-zero, 
    step = 1020/step;              //   divide by 1020
  } 
  return step;
}

int calculateVal(int step, int val, int i) {

  if ((step) && i % step == 0) { // If step is non-zero and its time to change a value,
    if (step > 0) {              //   increment the value if step is positive...
      val += 1;           
    } 
    else if (step < 0) {         //   ...or decrement it if step is negative
      val -= 1;
    } 
  }
  // Defensive driving: make sure val stays in the range 0-255
  if (val > 255) {
    val = 255;
  } 
  else if (val < 0) {
    val = 0;
  }
  return val;
}

void ISR_b1(){
  prevCounter1 = counter1;
  counter1++;
  if(counter1 > 5){
    int counter1 = 0;
  }
  counter2 = 0;
  int exitLoop = 1;
}

void ISR_b2(){
  
}

void solidCol(int color[3]){
  // Convert to 0-255
  int R = (color[0] * 255) / 100;
  int G = (color[1] * 255) / 100;
  int B = (color[2] * 255) / 100;

  analogWrite(redPin, R);
  analogWrite(greenPin, G);
  analogWrite(bluePin, B);
  delay(10);
}

void crossFade(int color[3]) {
  // Convert to 0-255
  int R = (color[0] * 255) / 100;
  int G = (color[1] * 255) / 100;
  int B = (color[2] * 255) / 100;

  int stepR = calculateStep(prevR, R);
  int stepG = calculateStep(prevG, G); 
  int stepB = calculateStep(prevB, B);

  for (int i = 0; i <= 1020; i++) {
    if(exitLoop == 1){ // if a button is pressed, exit loop at next increment
      break;
    }
    redVal = calculateVal(stepR, redVal, i);
    greenVal = calculateVal(stepG, greenVal, i);
    blueVal = calculateVal(stepB, blueVal, i);

    analogWrite(redPin, redVal);   // Write current values to LED pins
    analogWrite(greenPin, greenVal);      
    analogWrite(bluePin, blueVal);
    
    delay(wait); // Pause for 'wait' milliseconds before resuming the loop
  }
  // Update current values for next loop
  prevR = redVal; 
  prevG = greenVal; 
  prevB = blueVal;
  delay(hold); // Pause for optional 'wait' milliseconds before resuming the loop
}

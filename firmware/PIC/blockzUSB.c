/*
 * PIC12F1572
 *
 * Pins:
 *                     +----+
 *                Vdd -|1  8|- Vss
 *           PWM1 RA5 -|2  7|- RA0/ICSPDAT
 *           PWM2 RA4 -|3  6|- RA1/ICSPLCK
 *           RA3/MCLR -|4  5|- RA2 PWM3
 *                     +----+
 * 
 * ----------------------------------
 *   CALiTEC RGB BlockZ LED Program
 * ----------------------------------
 * 
 *        Written by Joe Hill
 * 
 */

#include <xc.h>

#define _XTAL_FREQ 16000000   // Oscillator frequency.

#pragma config FOSC = INTOSC  // INTOSC oscillator: I/O function on CLKIN pin.
#pragma config WDTE = OFF     // Watchdog Timer bit selectable.
#pragma config PWRTE = OFF    // Power-up Timer disabled.
#pragma config MCLRE = OFF     // MCLR disabled.
#pragma config CP = OFF       // Program memory code protection disabled.
#pragma config BOREN = OFF     // Brown-out Reset disabled.
#pragma config CLKOUTEN = OFF // CLKOUT function is disabled; I/O or oscillator function on the CLKOUT pin.
#pragma config WRT = OFF      // Flash Memory Write protection off.
#pragma config STVREN = ON    // Stack Overflow or Underflow will cause a Reset.
#pragma config LVP = OFF      // High-voltage on MCLR/VPP must be used for programming.

#include <stdio.h>
#include <stdlib.h>

  // Setting Global Variables

  // --  Colour Arrays  --
  // Main colours
int black[3]    = { 0, 0, 0 };
int white[3]    = { 100, 100, 100 };
int red[3]      = { 100, 0, 0 };
int green[3]    = { 0, 0, 100 };
int blue[3]     = { 0, 100, 0 };
  // Secondary colours
int orange[3]   = { 100, 0, 10 };
int purple[3]   = { 80, 100, 0 };
int teal[3]     = { 0, 75, 100 };
int lime[3]     = { 35, 0, 100 };
  // More colours...
int yellow[3]   = { 100, 0, 40 };
int purpleL[3]  = { 80, 100, 20 };
int mint[3]     = { 65, 100, 0};



  // Set initial colour
int redVal = 0;
int greenVal = 0; 
int blueVal = 0;

  // delay variables
int wait = 1; // stop for *ms between change of colour
int hold = 175; // stop for *ms when colour is reached
int flashDelay = 400;  // delay between changing solid colours

  // Initialise colour variables
int prevR = 0;
int prevG = 0;
int prevB = 0;

  // Set switch counter variables
int sw1Count = 1;
int sw2Count = 0;

int swNum = 1;

  // Interrupt variables
char exitLoop = 0;
int dummy;
int colourSet = 0;
int intCount1 = 0;
int intCount2 = 0;
int sw1Set = 0;
int sw2Set = 0;
int skipColour = 1;
int timerCount = 5;

  // Battery control
int adcVal;
int batLow = 0;
int lowPowerReset = 1;

  // Sleep Variables
int inSleep = 0;
int callSleep = 0;
int sleepCount = 0;

void init_pic() {
    WDTCON = 0; // Set Watchdog Timer to off.
    OSCCON = 0b01111010; // 16 Mhz oscillator.
    ANSELA = 0;          // Analog off.
    LATA = 0;            // Zero all port bits.

    // Set outputs
    TRISA5 = 0;
    TRISA2 = 0;
    TRISA1 = 0;
    TRISA0 = 0;
    
    // Set inputs
    TRISA3 = 1;
    TRISA4 = 1;
    
    // Weak Pull-ups
    WPUA3 = 1;
    WPUA4 = 1;

    OPTION_REG = 0b00000101;  // WPU enabled, Prescaler assigned to TMR0 at 1:64
    INTCON = 0b11100000;  // Enable Global, Peripheral and Timer0 interrupts and Timer0 flag
    VREGCON = 1;
    
    FVRCON = 0b11000001;
    ADCON0 = 0b01111101;  // FVR is ADC input
    ADCON1 = 0b01110000;  // Internal reference voltage is selected

    RA5 = 1 ;    

}

void init_pwm() {
            
    PWM1CLKCON = 0x01;
    PWM1PH = 0;
    PWM1DC = 0;
    PWM1PR = 32768;
    PWM1CON = 0b11000000;
    
    PWM2CLKCON = 0x01;
    PWM2PH = 0;
    PWM2DC = 0;
    PWM2PR = 32768;
    PWM2CON = 0b11000000;
    
    PWM3CLKCON = 0x01;
    PWM3PH = 0;
    PWM3DC = 0;
    PWM3PR = 32768;
    PWM3CON = 0b11000000;

    redVal = black[0];
    greenVal = black[1];
    blueVal = black[2];
  
    prevR = redVal;
    prevG = greenVal;
    prevB = blueVal;
    
}

void solidCol(int colour[3]){
    
    if(skipColour == 1){
        return;
    }
    
    // Convert to 0-255
    long R = (colour[0] * 65535) / 100;
    long G = (colour[1] * 65535) / 100;
    long B = (colour[2] * 65535) / 100;
    
    // set duty cycle to corresponding colours
    PWM1DC = R;
    PWM1LD = 1;
  
    PWM2DC = G;
    PWM2LD = 1;
    
    PWM3DC = B;
    PWM3LD = 1;
    
    prevR = redVal; 
    prevG = greenVal; 
    prevB = blueVal;    
}

void Sleep(){
    inSleep = 1;

    solidCol(red);
    __delay_ms(500);
    solidCol(black);
    __delay_ms(250);
    solidCol(red);
    __delay_ms(500);
    solidCol(black);
    __delay_ms(250);
    solidCol(red);
    __delay_ms(500);
    solidCol(black);
    __delay_ms(250);
    
    inSleep = 0;
    callSleep = 0;  // set marker for sleep routine to be called
    TRISA = 0;  // All pins as output
    PORTA = 0;  // All pins set low
    INTCON = 0;
    OPTION_REG = 0;  // WPU disabled, Prescaler assigned to WDT at 1:1
    FVRCON = 0;
    ADCON0 = 0;
    ADCON1 = 0;
    //WDTCON = 0b00010111;  // Set Watchdog Timer to on and Period Select bits to 2s delay.
    SLEEP();  // Enter Sleep mode
}

void delay_ms(int microseconds){  // create delay function able to take variable as input
    while(microseconds > 0){
        if(callSleep == 1 && inSleep == 0){
            Sleep();
        }
            if(skipColour == 1){
        return;
        }
        __delay_ms(1);
        microseconds--;
    }
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

void crossFade(int colour[3]){
    if(skipColour == 1){
        return;
    }
    
  // Convert to 0-255
    int R = (colour[0] * 256) / 100;
    int G = (colour[1] * 256) / 100;
    int B = (colour[2] * 256) / 100;

    if(colourSet == 1){ 
        redVal = black[0];
        greenVal = black[1];
        blueVal = black[2];

        prevR = redVal;
        prevG = greenVal;
        prevB = blueVal;
    
        colourSet = 0;
    }
    
    int stepR = calculateStep(prevR, R);
    int stepG = calculateStep(prevG, G); 
    int stepB = calculateStep(prevB, B);
  
  for (int i = 0; i <= 1020; i++) {
      
    if(exitLoop == 1){
          exitLoop = 0;
          colourSet = 1;
          return; // break out of function
    }
          
    redVal = calculateVal(stepR, redVal, i);
    greenVal = calculateVal(stepG, greenVal, i);
    blueVal = calculateVal(stepB, blueVal, i);

    PWM1DC = (redVal * 256);   // Write current values to LED pins
    PWM1LD = 1;
    
    PWM2DC = (greenVal * 256);
    PWM2LD = 1;
    
    PWM3DC = (blueVal * 256);
    PWM3LD = 1;
    
    delay_ms(wait); // Pause for 'wait' milliseconds before resuming the loop
  }
  // Update current values for next loop
  prevR = redVal; 
  prevG = greenVal; 
  prevB = blueVal;
  if(colour == black){
      delay_ms(hold*3);
  }
  delay_ms(hold); // Pause for optional 'wait' milliseconds before resuming the loop
}

void __interrupt() my_isr(void){
    TMR0IF = 0;  // Clear Timer0 interrupt flag
    sleepCount++;
    if(sleepCount > 500){  // Check battery level periodically due to time component of A/D conversion
        if(callSleep == 1){
            return;  // jump out of ISR
        }
        else{
            sleepCount = 0;
            ADCON0bits.GO = 1;  // Start A/D conversion
            while(ADCON0bits.GO)  // Empty loop till conversion finished
                ;                 // this is the reason for 'sleepCount'

            adcVal = ADRESH;  // Assign ADC value to variable

            if(adcVal >= 75){  // Compare ADC value to fixed voltage cutoff     ( adcVal = (1.024*255)/Vdd )
                               // Cutoff ~3.2V therefore,   (1.024*255)/3.2 = 81.6 -> 82
                batLow = 1;
                callSleep = 1;  // set marker for sleep routine to be called
            }
            else{
                batLow = 0;
            }
        }
    }
    
    if(RA3 == 0){
        intCount1++;  // increment interrupt counter
        if(intCount1 > timerCount){
            if(sw1Set == 0){
                swNum = 1;  // Set switch case to flow 1
                sw1Count++;  // increment button 1 counter
                sw2Count = 0;  // set button 2 counter to reset
                exitLoop = 1;  // tell program to exit loop if in between colours
                skipColour = 1;  // Skip black crossfade after colour change
                sw1Set = 1;  // Only run this code once per button press
                dummy = PORTA;
            }
            else{
                // do nothing and wait for button to release
            }
        }
    }
    else{
    intCount1 = 0;
    sw1Set = 0;
    }

    if(RA4 == 0){
        intCount2++;
        if(intCount2 > timerCount){
            if(sw2Set == 0){
                swNum = 2;
                sw2Count++;  // increment button 1 counter
                sw1Count = 0;
                exitLoop = 1;
                skipColour = 1;
                sw2Set = 1;
                dummy = PORTA;
            }
            else{
                // do nothing and wait for button to release
            }
        }
    }
    else{
    intCount2 = 0;
    sw2Set = 0;
    }
}

void VddCheck(){
    delay_ms(100);  // wait for FVR to be stable
    ADCON0bits.GO = 1;  // Start A/D conversion
    while(ADCON0bits.GO)  // Empty loop till conversion finished
        ;                 // this is the reason for 'sleepCount'

    adcVal = ADRESH;  // Assign ADC value to variable

    if(adcVal >= 73 && lowPowerReset == 1){  // Compare ADC value to fixed voltage cutoff     ( adcVal = (1.024*255)/Vdd )
                       // Voltage drop with LED strip at 100% = 0.7V
                       // Cutoff ~3.9V (Vcutoff + Vdrop) therefore,   (1.024*255)/3.9 = 67
        Sleep();
    }
    else{
        lowPowerReset = 0;
    }
}

int main() {
    // Initialise program settings
    init_pic();
    init_pwm();
    VddCheck();
        
    while(1){
        if(swNum == 1){
        switch(sw1Count){  // Switch case for button selector.
            case 1:
                skipColour = 0;
                crossFade(red);
                crossFade(green);
                crossFade(blue);
                break;
                
            case 2:
                skipColour = 0;
                solidCol(white);
                delay_ms(flashDelay);
                solidCol(red);
                delay_ms(flashDelay);
                solidCol(orange);
                delay_ms(flashDelay);
                solidCol(lime);
                delay_ms(flashDelay);
                solidCol(green);
                delay_ms(flashDelay);
                solidCol(teal);
                delay_ms(flashDelay);
                solidCol(blue);
                delay_ms(flashDelay);
                solidCol(purple);
                delay_ms(flashDelay);
                break;
                
            default:
                break;
        }
        if (sw1Count > 2){
            sw1Count = 1;  // if counter is out of range, set counter to 1
        }
        }
        
        else if(swNum == 2){
        switch(sw2Count){  // Switch case for button selector.
            case 1:
                skipColour = 0;
                solidCol(white);
                break;
                
            case 2:
                skipColour = 0;
                solidCol(red);
                break;
                
            case 3:
                skipColour = 0;
                solidCol(green);
                break;
                
            case 4:
                skipColour = 0;
                solidCol(blue);
                break;
                
            case 5:
                skipColour = 0;
                solidCol(orange);
                break;
                
            case 6:
                skipColour = 0;
                solidCol(purple);
                break;
                
            case 7:
                skipColour = 0;
                solidCol(teal);
                break;
                
            case 8:
                skipColour = 0;
                solidCol(lime);
                break;

            default:
                break;
        }
        if (sw2Count > 8){
            sw2Count = 1;  // if counter is out of range, set counter to 1
        }
        }
        
        else{
            swNum = 1;
        }

    }
    
  return (EXIT_SUCCESS);
}
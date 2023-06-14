/*
 * PIC12F1572 PWM example.
 *
 * Pins:
 *                     +----+
 *                Vdd -|1  8|- Vss
 *           PWM1 RA5 -|2  7|- RA0/ICSPDAT
 *           PWM2 RA4 -|3  6|- RA1/ICSPLCK
 *           RA3/MCLR -|4  5|- RA2 PWM3
 *                     +----+
 */

#include <xc.h>

#define _XTAL_FREQ 16000000   // Oscillator frequency.

#pragma config FOSC = INTOSC  // INTOSC oscillator: I/O function on CLKIN pin.
#pragma config WDTE = OFF     // Watchdog Timer disable.
#pragma config PWRTE = OFF    // Power-up Timer enbable.
#pragma config MCLRE = ON     // MCLR/VPP pin function is MCLR.
#pragma config CP = OFF       // Program memory code protection disabled.
#pragma config BOREN = ON     // Brown-out Reset enabled.
#pragma config CLKOUTEN = OFF // CLKOUT function is disabled; I/O or oscillator function on the CLKOUT pin.
#pragma config WRT = OFF      // Flash Memory Write protection off.
#pragma config STVREN = ON    // Stack Overflow or Underflow will cause a Reset.
#pragma config BORV = LO      // Brown-out Reset Voltage (Vbor), low trip point selected.
#pragma config LVP = OFF      // High-voltage on MCLR/VPP must be used for programming.

#include <stdio.h>
#include <stdlib.h>

int state = 0;
int dummy;

void __interrupt() my_isr(void){
    if(INTCONbits.IOCIF == 1){
        if(RA4 == 0){
            state = 1;
        }
        if(RA5 == 0){
            state = 2;
        }
        INTCONbits.IOCIF = 0;
        dummy = PORTA;
    }
}

int main(void) {
    
    OSCCON = 0b01111010; // 16 Mhz oscillator.
    ANSELA = 0;          // Analog off.
    LATA = 0;            // Zero all port bits.

    INTCONbits.GIE = 1;
    INTCONbits.IOCIE = 1;
    INTCONbits.IOCIF = 0;
    
    TRISA4 = 1;
    TRISA5 = 1;
    
    TRISA2 = 0;
    TRISA1 = 0;
    TRISA0 = 0;

    while(1){
    if(state == 0){
        RA0 = 1;
        RA1 = 1;
        RA2 = 1;
    }
    if(state == 1){
        RA0 = 0;
        RA1 = 1;
        RA2 = 1;
    }
    if(state == 2){
        RA0 = 1;
        RA1 = 0;
        RA2 = 0;
    }
    }
    return (EXIT_SUCCESS);
}


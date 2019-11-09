/*
 * funcdef.c
 *
 *  Created on: 5 Apr 2018
 *      Author: dv333
 */
#include "functions.h"
#include "msp430g2553.h"

void setCLK(void)
{
    DCOCTL = 0;                                         // Select lowest DCOx and MODx settings
    BCSCTL1 = CALBC1_1MHZ;                              // Set DCO
    DCOCTL = CALDCO_1MHZ;
    BCSCTL3 |= LFXT1S_2;                                // Set ACLK to use internal VLO
}

void setTimerA1(void)
{
    TA0CTL = TASSEL_2 + ID_0 + MC_0;
    TA0CCR0 = 100;
    TA1CCTL0 = CCIE;                                    // Enable counter interrupt for TACCTL0
    TA1CTL = TASSEL_2 +ID_3 + MC_1;                     // Set timer to use ACLK in Up mode
    TA1CCR0 = 1250;                                     // Interrupts every 10ms
}

void UARTinitialise(void)
{
      UCA0CTL1 |= UCSSEL_2;                             // SMCLK
      UCA0BR0 = 0x68;                                   // 1MHz 9600
      UCA0BR1 = 0x00;                                   // 1MHz 9600
      UCA0MCTL = UCBRS0;                                // Modulation UCBRSx = 5
      UCA0CTL1 &= ~UCSWRST;                             // Initialise USCI state machine
}

void PINinitialise(void)
{
    P2DIR |= 0xFF;                                      // All P2.x outputs
    P2OUT &= 0x00;                                      // All P2.x reset
    P2REN = 0xFF;

    P1SEL |= BIT1 + BIT2 ;                              // Setup P1.1 and P1.2 for UART
    P1SEL2 |= BIT1 + BIT2 ;
    P1SEL |= BIT5;                                      // P1.5 for ADC input

    P1DIR |= RED_LED + GREEN_LED;                       // P1.0 & P1.6 as output
    P1OUT &= 0x00;                                      // Set all outputs to low
    P1DIR |= BIT7;
    P1OUT |= BIT7;                                      // Set P1.7 high for input to POT


    P1REN |= BIT3;                                      // Enable internal pull-up/down resistors
    P1OUT |= BIT3;                                      //Select pull-up mode for P1.3
    P1IE |= BIT3;                                       // P1.3 interrupt enabled
    P1IES |= BIT3;                                      // P1.3 interrupt at falling edge
    P1IFG &= ~BIT3;                                     // Clear interrupt flag
}

void UART_Tx(char *Txsend, unsigned char length)        // Sends string through UART
{
    while(length!=0)
    {
        while(!(IFG2 & UCA0TXIFG));
        UCA0TXBUF = *Txsend;
        Txsend++;
        length--;
    }
}

char* ASCII(char conv)                                  // Converts 8 bit int to ASCII
{
    char ascii[3];
    ascii[0] = 32;
    ascii[1] = conv/10 +48;
    ascii[2] = conv%10 + 48;

    return ascii;
}

void initialiseADC(void)                                // Initialise ADC
{
    ADC10CTL1 = INCH_5 + ADC10DIV_3;                        // Connect to P1.5 and set to / 4
    ADC10CTL0 = SREF_0 + ADC10SHT_3 + ADC10ON + ADC10IE;    // Set voltage references and enable interrupt
    ADC10AE0 |= BIT5;                                       // P1.5 ADC
    __delay_cycles(1000);                                   // Wait for ADC ref to settle
}

void takeSample(void)
{
    ADC10CTL0 &= ~ENC;                                  // Disable Conversion
    while (ADC10CTL1 & BUSY);                           // Wait if ADC10 busy
    ADC10CTL0 |= ENC + ADC10SC;                         // Enable Conversion and conversion start
}

void updateLED(char *period1, unsigned char *counter1, char thresh1, unsigned char timercont1, unsigned int temp1)          // update blinking frequency
{
    if(temp1 < thresh1 && timercont1 == 1)
    {
        *period1 = 10;
    }
    else if(temp1 > thresh1 && timercont1 == 1)
    {
        *period1 = 50;
    }
    else if(temp1 == thresh1 && timercont1 == 1 )
    {
        *period1 = 100;
    }
}

char convNUM(char ls, char ms)                          // convert from ASCII to 8 bit int
{
    if( ls < 48 || ms > 57 )
        return -1;
    else
        return (ms-48)*10 + (ls-48);
}

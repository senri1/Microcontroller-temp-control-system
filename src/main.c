#include "msp430g2553.h"
#include "functions.h"
const char standby[] = { "\r\nCurrently in stand-by mode. Press button S2 to turn on.\r\n" };
const char active[] = { "\r\nActive mode.\r\n" };
const char controlstr[3][47] =
{
 { "\r\nCurrent temperature level setting: Low   \r\n" },
 { "\r\nCurrent temperature level setting: Medium\r\n" },
 { "\r\nCurrent temperature level setting: High  \r\n" }
};
char thresh[] = {12,20,28};
char period= 10;
unsigned char flag = 0;
unsigned char counter = 0;
unsigned char timercont = 0;
unsigned char control = 0;
unsigned char pressNum = 0;
unsigned char pressed = 0;
unsigned int temp=0;
char buffer[] = {0,0,0,0};
char input = 0;

int main(void)
{
   WDTCTL = WDTPW + WDTHOLD;                                        // Stop WDT
   IE1 |= WDTIE;                                                    // Enable WDT interrupt
   setCLK();                                                        // Set clocks
   PINinitialise();                                                 // Initialise pins
   setTimerA1();                                                    // Setup timerA
   UARTinitialise();                                                // Initialise UART
   initialiseADC();                                                 // Initialise ADC
   __bis_SR_register(LPM0_bits + GIE);                              // Enter LPM0 with interrupt enabled
}

#pragma vector=TIMER1_A0_VECTOR                                     // Timer 0 interrupt service routine
__interrupt void Timer1_A0 (void)
{
    if( timercont == 0)
    {
        flag = 0;
        TA0CTL |= MC_0;                                             // Stop timer
        TA0CCTL1 |= OUTMOD_0;                                       // Same value as OUT
        P1SEL &= ~RED_LED;                                          // Stop green LED form using timer peripheral 
        if(counter == 25)
        {
            P1OUT ^= RED_LED;                                       // Toggle LED
            counter = 0;
        }
        counter++;
    }
    else
    {
        if(counter == period){
            P1OUT ^= GREEN_LED;
            counter = 0;
            updateLED(&period, &counter, thresh[control-1], timercont, temp);
        }
        counter ++;
    }
    takeSample();
}

#pragma vector=PORT1_VECTOR                                         // P1 interrupt service routine
__interrupt void Port_1(void)
{
   P1IFG &= ~BIT3;                                                  // P1.3 IFG cleared
   P1IE &= ~BIT3;                                                   // Disable interrupt

   if(P1IES & BIT3)
   {
       if(flag == 0)
       {
           UART_Tx( &active,16);
           P1OUT &= ~RED_LED;                                       // Turn red led off
           IE2 |= UCA0RXIE;                                         // Enable USCI_A0 RX interrupt
           timercont=1;
           counter = 0;
           TA0CTL |= MC_1;                                          // Enable up mode
           flag = 1;
       }
       pressed = 1;
   }
   else
   {
       if(flag!=0 )
       {
           if(control<3)
           {
               control++;
           }
           else
           {
               control=1;
           }
           UART_Tx( &controlstr[control-1][0],45);                  // Print current status
           UART_Tx( ASCII(temp),3);
       }
       pressed = 0;
       switch(control)
       {
       case 1:
       {
           TA0CCTL1 |= OUTMOD_7;                                    // Output to set/reset
           TA0CCR1 = 5;                                             // Set duty cycle
           P1SEL |= RED_LED;                                        // Select timer A peripheral
       }
       break;
       case 2:
       {
           TA0CCTL1 |= OUTMOD_7;
           TA0CCR1 = 50;
           P1SEL |= RED_LED;
       }
       break;
       case 3:
       {
           TA0CCTL1 |= OUTMOD_7;
           TA0CCR1 = 100;
           P1SEL |= RED_LED;
       }
       break;
       }
   }

   pressNum = 0;
   P1IES ^= BIT3;                                               // Toggle rising/falling edge
   IFG1 &= ~WDTIFG;                                             // Clear WDT interrupt flag
   WDTCTL = WDT_MDLY_32;                                        // Set WDT to interrupt every 32ms
}

#pragma vector = WDT_VECTOR                                     // WDT interrupt service routine
__interrupt void wdt_isr(void)
{
    if (pressed == 1)
    {
        if ( ++pressNum == 125 && flag!=0 )
        {
            if(control == 1)
            {
                control =3;
            }
            else
            {
                control--;
            }
            pressNum = 0;
            pressed = 0;
            timercont =0;
            IE2 &= ~UCA0RXIE;                               // Disable USCI_A0 RX interrupt
            UART_Tx( &standby,59);
            P1OUT &= ~GREEN_LED;
        }
    }
P1IFG &= ~BIT3;                                             // Clear P1.3 / button S2 interrupt flag
P1IE |= BIT3;                                               // Enable interrupt for P1.3
}

#pragma vector=USCIAB0RX_VECTOR                             // UART RXD interrupt service routine
__interrupt void USCI0RX_ISR(void)
{
    buffer[3] = buffer[2];
    buffer[2] = buffer[1];
    buffer[1] = buffer[0];
    buffer[0] = UCA0RXBUF;

    if(buffer[0] == 10)
    {
        switch(buffer[3])
        {
        case 'l':
        {
            input = convNUM(buffer[1],buffer[2]);           // Convert to number from ASCII
            if(input > 4 && input < 36)
            {
                thresh[0] = input;
                UART_Tx( "\r\nLower threshold currently: ",29);
                UART_Tx( ASCII(thresh[0]) , 3 );
                UART_Tx( "\r\n",2);
            }
            else
                UART_Tx( "\r\nInvalid value\r\n",17);
        }
        break;
        case'm':
        {
            input = convNUM(buffer[1],buffer[2]);
            if(input > 4 && input < 36)
            {
                thresh[1] = input;
                UART_Tx( "\r\nMiddle threshold currently: ",30);
                UART_Tx( ASCII(thresh[1]) , 3 );
                UART_Tx( "\r\n",2);
            }
            else
                UART_Tx( "\r\nInvalid value\r\n",17);
        }
        break;
        case'h':
        {
            input = convNUM(buffer[1],buffer[2]);
            if(input > 4 && input < 36)
            {
                thresh[2] = input;
                UART_Tx( "\r\nUpper threshold currently: ",29);
                UART_Tx( ASCII(thresh[2]) , 3 );
                UART_Tx( "\r\n",2);
            }
            else
                UART_Tx( "\r\nInvalid value\r\n",17);
        }
        break;
        default:
        {
            UART_Tx( "\r\nInvalid value\r\n",17);
        }
        break;
        }
    }
}

#pragma vector=ADC10_VECTOR                                         // ADC interrupt service routine                
__interrupt void ADC10_ISR (void)
{
    temp = (ADC10MEM >>5) + 5;                                      // Scale ADC input between 5 and 35
}

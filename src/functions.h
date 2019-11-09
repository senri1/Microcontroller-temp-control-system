void    setCLK(void);                                            // Set clocks
void    PINinitialise(void);                                     // Initialise pins
void    setTimerA1(void);                                        // Set timer to interrupt every 0.25s
void    UARTinitialise(void);                                    // Initialise UART
void    UART_Tx(char *Txsend, unsigned char length);             // Send string
char*   ASCII(char conv);                                        // Convert 8 bit int to ASCII
char    convNUM(char ls, char ms);                               // Convert ASCII to 8 bit int
void    initialiseADC(void);                                     // Initialise ADC
void    takeSample(void);                                        // take sample form P1.5 for ADC
void    updateLED(char *period1, unsigned char *counter1, char thresh1, unsigned char timercont1, unsigned int temp1);  // Update led period

#define GREEN_LED BIT0   // CURRENTLY SWAPPED GREEN LED IS ACTUALLY RED AND RED IS GREEN
#define RED_LED BIT6

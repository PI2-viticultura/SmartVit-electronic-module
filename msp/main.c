// Includes
#include <msp430.h> 

// Constant values along the code
#define sleepTime 460799
#define getSample 30719
#define resetValue 0

// Pins
#define anemometer BIT3
#define pluviometer BIT4

// Error values
#define digitalPortError 401
#define adcPortError 402

// Different state of FSM
typedef enum{
    setup,
    clearOutput,
    getAnemometer,
    getPluviometer,
    getWindVane,
    getTemperature,
    getPh,
    getMoisture1,
    getMoisture2,
    getMoisture3,
    sendData,
    sleepMode
}eSystemState;

// Output variables
typedef struct{
    char wind_dir_id;
    float wind_dir;
    char wind_speed_id;
    float wind_speed;
    char rain_quantity_id;
    float rain_quantity;
    char soil_temperature_id;
    float soil_temperature;
    char soil_moisture1_id;
    float soil_moisture1;
    char soil_moisture2_id;
    float soil_moisture2;
    char soil_moisture3_id;
    float soil_moisture3;
    char soil_ph_id;
    float soil_ph;
} data_t;

// Struct
data_t data;


// Intern variables
volatile int timerCounter = 0;                          // Declare variable to control timer
volatile int pulseCounter = 0;                          // Declare variable to control number of pulses

// Functions calls
void setADC();
void setUART();
void setTimer();
void setOtherPins();
void setIds();
void resetVariables();
float getDigitalData(int bit);
float getADCData(int bit);
float collectPhData();
void sendUART(data_t *data);
void countTime();

/**
 * main.c
 */

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;                           // Stop watchdog timer
    eSystemState eCurrentState;                         // Create state variables to control FSM
    eSystemState eNextState = setup;                    // Set next state as setup

    while(1)
    {

        eCurrentState = eNextState;                     // Declares variable that handles state changes
        //FSM
        switch(eCurrentState)
        {
            case setup:
            {
                setADC();                               // Set ADC basic configuration
                setUART();                              // Set UART basic configuration
                setTimer();                             // Set timer configuration
                setOtherPins();                         // Configure unused pins to reduce consume
                setIds();
                eNextState = clearOutput;               // Change to next state
                break;
            }
            case clearOutput:
            {
                resetVariables();                       // Calll function to reset variables
                eNextState = getAnemometer;             // Change to next state
                break;
            }
            case getAnemometer:
            {
                data.wind_speed = getDigitalData(3);    // Call function to acquire digital data form anemometer
                eNextState = getPluviometer;            // Change to next state
                break;
            }
            case getPluviometer:
            {
                data.rain_quantity = getDigitalData(4); // Call function to acquire digital data from pluviometer
                eNextState = getWindVane;               // Change to next state
                break;
            }
            case getWindVane:
            {
                data.wind_dir = getADCData(0);          // Call function to acquire adc data from wind vane
                eNextState = getTemperature;            // Change to next state
                break;
            }
            case getTemperature:
            {
                data.soil_temperature = getADCData(3);  // Call function to acquire adc data from wind vane
                eNextState = getPh;                     // Change to next state
                break;
            }
            case getPh:
            {
                data.soil_ph = collectPhData();         // Call function to acquire data from ph sensor
                eNextState = getMoisture1;              // Change to next state
                break;
            }
            case getMoisture1:
            {
                data.soil_moisture1 = getADCData(5);    // Call function to acquire adc data from 1° soil moisture sensor
                eNextState = getMoisture2;              // Change to next state
                break;
            }
            case getMoisture2:
            {
                data.soil_moisture2 = getADCData(6);    // Call function to acquire adc data from 3° soil moisture sensor
                eNextState = getMoisture3;              // Change to next state
                break;
            }
            case getMoisture3:
            {
                data.soil_moisture3 = getADCData(7);    // Call function to acquire adc data from 2° soil moisture sensor
                eNextState = sendData;                  // Change to next state
                break;
            }
            case sendData:
            {
                sendUART(&data);                        // Send wind direction data using UART

                eNextState = sleepMode;                 // Change to next state
                break;
            }
            case sleepMode:
            {
                countTime();                            // Start counted sleep mode
                eNextState = clearOutput;               // Change to next state
                break;
            }
            default:
            {
                eNextState = sleepMode;                 // Change to next state
            }
        }
    }
}

//############################################################################################################################
// Functions

// Function setADC -> select clock division and voltage reference
void setADC(){
    ADC10CTL0 = SREF_0 + ADC10SHT_3 + REFON;            // Ref -> Vcc reference, 64 CLK S&H
}

// Function setUART -> select clock frequency, baud rate and pins
void setUART(){
    if (CALBC1_1MHZ==0xFF)                              // Check if calibration constant erased
    {
        while(1);                                       // do not load program
    }
    DCOCTL = 0;                                         // Select lowest DCO settings
    BCSCTL1 = CALBC1_12MHZ;                             // Set DCO to 12 MHz
    DCOCTL = CALDCO_12MHZ;                              // Set DCO to 12 MHz
    UCA0CTL1 |= UCSSEL_2;                               // UART Clock -> SMCLK
    UCA0BR0 = 1250;                                     // Baud Rate Setting for 12MHz 9600
    UCA0BR1 = 0;                                        // Baud Rate Setting for 12MHz 9600
    UCA0MCTL = UCBRS_0;                                 // Modulation Setting for 12MHz 9600
    P1SEL |= BIT1 + BIT2 ;                              // Select UART RX/TX function on P1.1,P1.2
    P1SEL2 |= BIT1 + BIT2;                              // Secondary peripheral module function is selected (P1SEL = P1SEL2 = 1)
    UCA0CTL1 &= ~UCSWRST;                               // Initialize UART Module
}

// Function setTimer -> select clock, mode, divisions and stop clock from ticking
void setTimer(){
    BCSCTL1 |= DIVA_3;                                  // ACLK/8
    BCSCTL1 |= ~XTS;                                    // LF mode
    TACTL = TASSEL_1 + ID_3 + MC_1;                     // Select ACLK, ACLK/8, Up Mode
    TACCR0 = 0;                                         //Initially, Stop the Timer
}

// Function setOtherPins -> grant that unused pins don't waste power
void setOtherPins(){
    P2OUT |= BIT0 + BIT1 + BIT5 + BIT6 + BIT7;          // Set unused pins as output to reduce power consumption
}

void setIds(){
    data.wind_dir_id = 'a';                             //Set id for wind vane sensor
    data.wind_speed_id = 'b';                           //Set id for anemometer sensor
    data.rain_quantity_id = 'c';                        //Set id for pluviometersensor
    data.soil_temperature_id = 'd';                     //Set id for temperature sensor
    data.soil_moisture1_id = 'e';                       //Set id for 1º soil moisture sensor
    data.soil_moisture2_id = 'f';                       //Set id for 2º soil moisture sensor
    data.soil_moisture3_id = 'g';                       //Set id for 3º soil moisture sensor
    data.soil_ph_id = 'h';                              //Set id for ph sensor
}

// Function resetVariables -> reset all sending variables to a predetermined value
void resetVariables(){
    data.wind_dir = resetValue;                         // Reset wind_dir variable
    data.wind_speed = resetValue;                       // Reset wind_speed variable
    data.rain_quantity = resetValue;                    // Reset rain_quantity variable
    data.soil_temperature = resetValue;                 // Reset soil_temperature variable
    data.soil_moisture1 = resetValue;                   // Reset soil_moisture1 variable
    data.soil_moisture2 = resetValue;                   // Reset soil_moisture2 variable
    data.soil_moisture3 = resetValue;                   // Reset soil_moisture3 variable
    data.soil_ph = resetValue;                          // Reset soil_ph variable
}

// Function getDigitalData -> Enables pins and interruptions, gets data, disable interruptions and return it
float getDigitalData(int bit){

    if (bit == 3){
        P2DIR |= anemometer;                            // P2.3 as input
        P2IE |= anemometer;                             // P2.3 enable interruption
        P2IES |= anemometer;                            // P2.3 low-to-high interruption
        TACCTL0 |= CCIE;                                // Enable interrupt for CCR0.
        TACCR0 = getSample;                             // Set timer as getSample
        __bis_SR_register(LPM3_bits + GIE);             // Enable interrupt and set MSP to LPM3
        P2IE &= ~anemometer;                            // P2.3 disable interruption

        return pulseCounter;                            // Return number of pulses
    }
    else if (bit == 4){
        P2DIR |= pluviometer;                           // P2.4 as input
        P2IE |= pluviometer;                            // P2.4 enable interruption
        P2IES |= pluviometer;                           // P2.4 low-to-high interruption
        TACCTL0 |= CCIE;                                // Enable interrupt for CCR0.
        TACCR0 = getSample;                             // Set timer as getSample
        __bis_SR_register(LPM3_bits + GIE);             // Enable interrupt and set MSP to LPM3
        P2IE &= ~pluviometer;                           // P2.3 disable interruption

        return pulseCounter;                            // Return number of pulses
    }
    else{
        return digitalPortError;                        // Return portError if installed at wrong pin
    }
}

// Function getADCData -> Enables pins and interruptions, gets data, disable interruptions and return it
float getADCData(int bit){
    if (bit == 0){
        ADC10CTL1 |= INCH_0 + ADC10SSEL1;               // ADC Channel -> A0 P1.0 - Wind Vane, ACLK
    }
    else if (bit == 3){
        ADC10CTL1 |= INCH_3 + ADC10SSEL1;               // ADC Channel -> A0 P1.3 - Soil temperature, ACLK
    }
    else if (bit == 4){
        ADC10CTL1 |= INCH_4 + ADC10SSEL1;               // ADC Channel -> A0 P1.4 - Soil PH, ACLK
    }
    else if (bit == 5){
        ADC10CTL1 |= INCH_5 + ADC10SSEL1;               // ADC Channel -> A0 P1.5 - 1° Soil Moisture, ACLK
    }
    else if (bit == 6){
        ADC10CTL1 |= INCH_6 + ADC10SSEL1;               // ADC Channel -> A0 P1.6 - 2° Soil Moisture, ACLK
    }
    else if (bit == 7){
        ADC10CTL1 |= INCH_7 + ADC10SSEL1;               // ADC Channel -> A0 P1.7 - 3° Soil Moisture, ACLK
    }
    else{
        return adcPortError;                            // Return portError if installed at wrong pin
    }

    ADC10CTL0 |= ADC10ON + ADC10IE;                     // ADC On, Enable Interruption
    ADC10CTL0 |= ENC + ADC10SC;                         // Sampling and conversion started
    __bis_SR_register(LPM3_bits + GIE);                 // Enable interrupt and set MSP to LPM3
    ADC10CTL0 = ENC + ADC10SC;                          // Sampling and conversion ended
    ADC10CTL0 &= ~ADC10ON + ~ADC10IE;                   // ADC Off, Disable Interruption

    return ADC10MEM;                                    // Return value stored at ADC memory

}

float collectPhData(){
    int buf[10];                                        // Declare variable to store 10 measures
    int temp;                                           // Auxiliary variable
    unsigned int i, j;                                  // Loop control variables
    float value;                                        // Return variable

    for (i = 0; i < 10; i++){
        buf[i] = getADCData(4);                         // Collect ph data ten times
    }
    for (i = 0; i < 9; i++){
        for (j = i+1; j < 10; j++){
            if (buf[j] > buf[i]){                       // Compare neighbor data, see if rightmost is bigger,change places with leftmost
                temp = buf[i];
                buf[i] = buf[j];
                buf[j] = temp;
            }
        }
    }
    for (i = 2; i < 8; i++){
        value += buf[i];                                // Get sum of intermediary values
    }
    return value;
}

// Function countTime -> Enables pins and interruptions, put msp to sleep for predetermined time
void countTime(){
    timerCounter = 0;                                   // Reset variable

    TACCTL0 |= CCIE;                                    // Enable interrupt for CCR0.
    TACCR0 = sleepTime;                                 // Set time as sleepTime
    __bis_SR_register(LPM3_bits + GIE);                 // Enable interrupt and set MSP to LPM3
}


// Function sendUART -> Receive variable and id, segment into bits and send it
void sendUART(data_t *data){
    unsigned char *c;                                   // Create temporary variable to point to float data
    unsigned int i, j;                                  // Auxiliary variables to control loop

    IE2 |= UCA0TXIE;                                    // Enable TX interrupt
    for (i = 0; i < 16; i+=2){
        __bis_SR_register(LPM0_bits + GIE);             // Enable interrupt and set MSP to LPM0
        UCA0TXBUF = (unsigned char *) (data+i);                          // Send id

        c = (unsigned char *) (data+i+1);                                  // Get pointer to first byte
        for (j = 0; j <3; j++){
            __bis_SR_register(LPM0_bits + GIE);         // Enable interrupt and set MSP to LPM0
            UCA0TXBUF = *c++;                           // Send first to third bit
        }
        __bis_SR_register(LPM0_bits + GIE);             // Enable interrupt and set MSP to LPM0
        UCA0TXBUF = *c;                                 // Send last bit

    }
    IE2 &= ~UCA0TXIE;                                   // Disable TX interrupt
}


//############################################################################################################################
//Interrupts

// ADC ISR -> Wait for ADC collect data and exit sleep mode
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR (void)
{
    __bic_SR_register_on_exit(LPM3_bits + GIE);         // Exit LMP3 and disable interruption
}

//Timer ISR -> counts until reaches value, stop timer and exit sleep mode
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A_CCR0_ISR(void)
{
    if (TACCR0 == sleepTime)
    {
        timerCounter++;                                 // Increase counter
        if(timerCounter >= sleepTime)
        {
            TACCR0 = 0;                                 // Stop timer
            __bic_SR_register_on_exit(LPM3_bits + GIE); // Exit LMP3 and disable interruption
        }
    }
    else{
        __bic_SR_register_on_exit(LPM3_bits + GIE);     // Exit LMP3 and disable interruption
    }

}

//Pulse ISR -> Increase counter and enables new interruption to occur
#pragma vector = PORT2_VECTOR
__interrupt void PORT2_ISR(void)
{
    pulseCounter++;                                     // Increase counter
    P2IFG = 0;                                          // Reset interrupt flag
}

// UART ISR -> Waits till transmission is completed
#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCI0TX_ISR(void)
{
    while (!(IFG2&UCA0TXIFG));                          // Check if TX is ongoing
    __bic_SR_register_on_exit(LPM0_bits + GIE);         // Exit LMP3 and disable interruption
}

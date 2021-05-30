//Patrick Reedy; May 28, 2021

/*--------------------------------------------------------------------------------------------------------------------------------------
*  Library Definitions
--------------------------------------------------------------------------------------------------------------------------------------*/
#include "msp.h"
#include "COMP.h"
#include "DCO.h"
#include "UART.h"
#include "ADC14.h"

/*--------------------------------------------------------------------------------------------------------------------------------------
*  Function Definitions
--------------------------------------------------------------------------------------------------------------------------------------*/
void UART_init(void);
void UART_print(uint8_t cha);
void set_DCO(uint32_t freq);
void UART_print_string(char string[]);
void UART_esc_code(char string[]);
uint32_t find_min_array(uint32_t calculation[]);
uint32_t find_max_array(uint32_t calculation[]);
void COMP_init(void);
void TIMERA_COMP_init(void);
void TIMERA_ADC_init(void);
void UART_print_num(uint32_t num);
void ADC14_init(void);
void ref_gen(uint32_t microvolts);

/*--------------------------------------------------------------------------------------------------------------------------------------
*  Global Variables
--------------------------------------------------------------------------------------------------------------------------------------*/
volatile uint32_t overflow = 0, tau = 0, current = 0, previous = 0;
volatile uint16_t i = 0;
volatile uint16_t sample[SAMPLES];
uint32_t calculation[SAMPLES];
uint32_t frequency = 0, minimum, maximum, average, optimization;
uint8_t convend_flag = 0;

/*--------------------------------------------------------------------------------------------------------------------------------------
*  Main
--------------------------------------------------------------------------------------------------------------------------------------*/

void main(void)
{
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;                     //Stop watchdog timer
    typedef enum {                                                  //New variable types for Finite State Machine
        INITIALIZE,                                                 //State to initialize all modules used
        SAMPLING,                                                   //State to sample incoming waveform for reference voltage generation
        REF_GEN,                                                    //State to generate the reference voltage
        FREQUENCY                                                   //State to calculate and display the frequency
    } STATE;

    STATE NS = INITIALIZE;
    while(1) {
        switch(NS) {
            case INITIALIZE: {
                set_DCO(FREQ_24_MHZ);                               //Set SMCLK and MCLK to 24MHz
                UART_init();                                        //Initialize UART, COMP, ADC14, and TIMERA
                COMP_init();
                TIMERA_COMP_init();
                ADC14_init();
                TIMERA_ADC_init();
                P7->SEL0 |= BIT2;
                P7->SEL1 &= ~BIT2;
                P7->DIR |= BIT2;
                NVIC->ISER[0] = (1 << (TA1_N_IRQn & 31));           //NVIC enable for TimerA Interrupts
                NVIC->ISER[0] |= (1 << (ADC14_IRQn & 31));          //NVIC enable for ADC14 Interrupts
                NVIC->ISER[0] |= (1 << (TA0_0_IRQn & 31));          //NVIC enable for TIMER_A0
                __enable_irq();                                     //Enable global interrupts
                NS = SAMPLING;                                      //Switch states to SAMPLING
                break;
            }

            case SAMPLING: {
                TIMER_A0->CCTL[0] |= TIMER_A_CCTLN_CCIE;            //Enable TIMER_A0 Interrupts
                if (convend_flag == 1) {                            //Check if conversion has ended
                    convend_flag = 0;                               //Reset the conversion end flag
                    TIMER_A0->CCTL[0] &= ~TIMER_A_CCTLN_CCIE;       //Disable TIMER_A0 Interrupts
                    NS = REF_GEN;                                   //Switch states to REF_GEN
                    break;
                }
                else {
                    NS = SAMPLING;                                  //Wait in the sampling state until sampling ends
                    break;
                }
            }

            case REF_GEN: {
                for (i=0;i<SAMPLES;i++) {                           //Run for the amount of samples
                    calculation[i] = (sample[i]*2014-32742)/10;     //Voltage optimization in micro volts for each sample taken
                }
                minimum = find_min_array(calculation);              //Find the minimum value within the calculation array
                maximum = find_max_array(calculation);              //Find the maximum value within the calculation array
                average = (maximum + minimum)/2;                    //Get the average from the min and max valuess
                ref_gen(average);                                   /*Generate the reference voltagesss for the comparator
                                                                      based on the average of the incoming wave*/
                NS = FREQUENCY;                                     //Switch states to FREQUENCY to begin calculating
                break;
            }

            case FREQUENCY: {
                frequency = (CPU_FREQ)/tau;                         /*Calculate the frequency of the wave based on time between
                                                                      rising clock edges of comparator*/
                optimization = (frequency*10040+7166)/10000;        //Apply linear optimization for 1Hz to 1kHz
                if (optimization < LOWERTHRESH) {                   //Check if in the lower range of frequencies
                    optimization++;                                 //Apply linear optimization for 1Hz to 70Hz
                }
                UART_esc_code(CLEAR);                               //Clear the terminal screen
                UART_esc_code(RESCUR);                              //Reset the cursor location to top left of screen
                UART_print_string("Frequency: ");                   //Print the preface for the frequency
                UART_print_num(optimization);                       //Print the optimization of the frequency to the terminal
                UART_print_string(" Hz ");                          //Print the units for the frequency
                NS = SAMPLING;                                      //Switch states back to sampling to restart the process
                break;
            }

            default: {
                NS = INITIALIZE;                                    //If errors occur reset the entire process
                break;
            }
        }
    }
}

//Interrupt handler used to sample the incoming waveform
void ADC14_IRQHandler(void) {
    static uint32_t count = 0;
    if (ADC14->IFGR0 & ADC14_IFGR0_IFG0) {                          //Check for MEM interrupt flag for end of conversion
        if (count != SAMPLES) {
            sample[count] = ADC14->MEM[0];                          //Save the value of the current sample/conversion
            count++;                                                //Increment the count to get next value of array
        }
        else {
            ADC14->CLRIFGR0 |= ADC14_IFGR0_IFG0;                    //Set bit high to clear interrupt flag for end of conversion
            count = 0;
            convend_flag = 1;
        }
    }                                                               //Flag reset on reading from MEM register
}

//Interrupt handler used to create uniform timing between samples of the waveform
void TA0_0_IRQHandler(void) {
    if (TIMER_A0->CCTL[0] & TIMER_A_CCTLN_CCIFG){                   //Check for CCR0 value being reached (Delay between samples)
        TIMER_A0->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG;                  //Reset the flag
        ADC14->CTL0 |= (ADC14_CTL0_ENC                              //Enable Conversion
                      | ADC14_CTL0_SC);                             //Start the sample/conversion Process
    }
}

//Interrupt handler used to calculate rollover and time difference between rising edges of the comparator using the TimerA module
void TA1_N_IRQHandler(void) {
    if (TIMER_A1->CTL & TIMER_A_CTL_IFG) {                          //Occures when rollover
        TIMER_A1->CTL &= ~TIMER_A_CTL_IFG;
        overflow++;                                                 //Increment overflow
    }
    if (TIMER_A1->CCTL[3] & TIMER_A_CCTLN_CCIFG) {                  //Occurs when rising edge of comparator output is read
        TIMER_A1->CCTL[3] &= ~TIMER_A_CCTLN_CCIFG;                  //Reset the flag
        previous = current;                                         //Save previous value of current
        current = TIMER_A1->CCR[3];                                 //Save the current value that the CCR has reached
        tau = ((current - previous) + (overflow * MAXTIME));        //Calculate the time difference between capture edges
        overflow = 0;                                               //Reset the rollover amount for new calculation
    }
}

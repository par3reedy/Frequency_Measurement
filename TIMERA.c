//Patrick Reedy; May 28, 2021

/*--------------------------------------------------------------------------------------------------------------------------------------
*  Library Definitions
--------------------------------------------------------------------------------------------------------------------------------------*/
#include "msp.h"
#include "COMP.h"

/*--------------------------------------------------------------------------------------------------------------------------------------
*  Function Definitions
--------------------------------------------------------------------------------------------------------------------------------------*/

//Function to initialize the TIMERA for use with the comparator
void TIMERA_COMP_init(void) {
    TIMER_A1->CTL = (TIMER_A_CTL_IE                                         //Enable interrupts for TimerA
                   | TIMER_A_CTL_MC__CONTINUOUS                             //TimerA is in continuous mode
                   | TIMER_A_CTL_ID__1                                      //Divide input clk by 1
                   | TIMER_A_CTL_SSEL__SMCLK);                              //Set TimerA to SMCLK 24MHz
    TIMER_A1->CCTL[3] = (TIMER_A_CCTLN_OUTMOD_0                             //Output mode output
                       | TIMER_A_CCTLN_CAP                                  //Capture mode enabled
                       | TIMER_A_CCTLN_SCS                                  //Synchronized capture source
                       | TIMER_A_CCTLN_CCIS__CCIB                           //Select CCI0B signal from Comparator
                       | TIMER_A_CCTLN_CM__RISING                           //Capture signal on the rising edge of the clk
                       | TIMER_A_CCTLN_CCIE);                               //Enable TimerA capture interrupts
}

void TIMERA_ADC_init(void) {
    TIMER_A0->CCR[0] = MAXTIME;                                             //Count amount is 200 24MHz Clock Cycles
    TIMER_A0->CTL = (TIMER_A_CTL_TASSEL_2                                   //Run TimerA with SMCLK
                   | TIMER_A_CTL_MC__UP);                                   //Count with TimerA in UP Mode
    TIMER_A0->CCTL[0] = TIMER_A_CCTLN_CCIE;                                 //Enable TIMER_A0
}

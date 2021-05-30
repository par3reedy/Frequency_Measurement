//Patrick Reedy; May 28, 2021

/*--------------------------------------------------------------------------------------------------------------------------------------
*  Library Definitions
--------------------------------------------------------------------------------------------------------------------------------------*/
#include "msp.h"
#include "DCO.h"
#include "ADC14.h"

/*--------------------------------------------------------------------------------------------------------------------------------------
*  Function References
--------------------------------------------------------------------------------------------------------------------------------------*/
void set_DCO(uint32_t freq);

/*--------------------------------------------------------------------------------------------------------------------------------------
*  Function Definitions
--------------------------------------------------------------------------------------------------------------------------------------*/

//Function to initialize the ADC14 for use
void ADC14_init(void) {
    ADC->SEL0 |= AIN;                                   //Setup ADC analog pin for the ADC14 on A1 (P5.4)
    ADC->SEL1 |= AIN;
    ADC14->CTL0 = (ADC14_CTL0_ON                        //Turn the ADC core on and ready for conversion
                 | ADC14_CTL0_SHT0__4                   //Sample hold for 4 clock cycles
                 | ADC14_CTL0_CONSEQ_0                  //Single channel single conversion
                 | ADC14_CTL0_SSEL__SMCLK               //Operate with SMCLK @ 24MHz
                 | ADC14_CTL0_DIV__1                    //ADC clk is SMCLK frequency (24MHz)
                 | ADC14_CTL0_SHP);                     //Sample and Hold Pulse Mode  with ADC sample timer
    ADC14->CTL1 = (ADC14_CTL1_RES__14BIT);              //14-bit conversion
    ADC14->MCTL[0] = (ADC14_MCTLN_INCH_1                //Analog input on A1
                    | ADC14_MCTLN_VRSEL_0);             //Using AVss and AVcc or 0-3.3V
    ADC14->IER0 |= ADC14_IER0_IE0;                      //Enable conversion complete interrupt
}

//Function to find the maximum of a given array
uint32_t find_min_array(uint32_t calculation[]){
    uint32_t minimum = calculation[0], i = 0;           //Initialize variables for function
    for (i=0;i<SAMPLES;i++) {                           //Run for the amount of samples
        if (minimum > calculation[i+1]) {               //Check when the current minimum value is bigger than the next
            minimum = calculation[i+1];                 //Make the next value the minimum
        }
    }
    return minimum;
}

//Function to find the minimum of a given array
uint32_t find_max_array(uint32_t calculation[]){
    uint32_t maximum = calculation[1], i = 0;           //Initialize variables for function
    for (i=0;i<SAMPLES;i++) {                           //Run for the amount of samples
        if ((maximum < calculation[i])                  //Check when the current maximum value is bigger than the next
          & (calculation[i] < MAXVOLTAGE)) {            //Check if calculation is errant (bigger than maximum voltage)
            maximum = calculation[i];                   //Make the next value the maximum
        }
    }
    return maximum;
}

//Patrick Reedy; May 28, 2021

/*--------------------------------------------------------------------------------------------------------------------------------------
*  Library Definitions
--------------------------------------------------------------------------------------------------------------------------------------*/
#include "msp.h"
#include "COMP.h"

/*--------------------------------------------------------------------------------------------------------------------------------------
*  Function Definitions
--------------------------------------------------------------------------------------------------------------------------------------*/

//Function to initialize the comparator for use with internal reference generation
void COMP_init(void) {
    COMP_E1->CTL0 = ( (0 << COMP_E_CTL0_IMEN_OFS)           //Negative terminal for analog input disabled
                    | (1 << COMP_E_CTL0_IPEN_OFS)           //Positive terminal for analog input enabled
                    | COMP_E_CTL0_IPSEL_0);                 //Set positive input terminal of Comparator to C1.0 (P6.7)
    COMP_E1->CTL1 = ( (0 << COMP_E_CTL1_MRVS_OFS)           //Comparitor output determines reference voltage used
                    | (1 << COMP_E_CTL1_ON_OFS)             //Turn the comparitor on
                    | COMP_E_CTL1_PWRMD_0                   //Power mode = High Speed Mode
                    | COMP_E_CTL1_FDLY_3                    //Filter delay of 3600ns
                    | (1 << COMP_E_CTL1_F_OFS)              //Output low pass filter enabled
                    | (0 << COMP_E_CTL1_OUTPOL_OFS));       //Output of comparator is non-inverting
    COMP_E1->CTL2 = ( COMP_E_CTL2_REFL__OFF                 //Refernce amplifier disabled
                    | COMP_E_CTL2_REF1_10                   //Lower reference of 1V (32*1V/3.3V =~ 10)
                    | COMP_E_CTL2_RS_1                      //Vcc is applied to the resistor ladder
                    | (1 << COMP_E_CTL2_RSEL_OFS)           //Vref applied to V- when CEEX = 0
                    | COMP_E_CTL2_REF0_12);                 //Upper reference of 2V (32*2V/3.3V =~ 19)
    COMP_E1->CTL3 = (COMP_E_CTL3_PD0);                      //Disable buffer for analog input of Comparator
    COMPPORT->SEL0 |= COMPAIN;                              //Set up GPIO for Comparator analog input pin
    COMPPORT->SEL1 |= COMPAIN;
}

//Function to generate lower and upper reference voltages for the comparator
void ref_gen(uint32_t microvolts){
    uint32_t factor;                                        //Initialize variables
    uint8_t upper, lower;
    factor = microvolts*VOLTDIVMAX/MAXVOLTAGE;              //Determine voltage divider factor with given offset
    if (factor > (VOLTDIVMAX-1)) {                          //Check if factor is larger than maximum factor
        factor = VOLTDIVMAX-1;                              //Set the factor to the max minus 1 (-1 b/c counting 0)
    }
    upper = factor;                                         //Upper becomes the factor
    lower = upper - 1;                                      //Lower is 1 lower than factor for hystersis correction
    COMP_E1->CTL2 &= ~COMP_E_CTL2_REF0_MASK;                //Clear the current upper reference voltage factor
    COMP_E1->CTL2 |= ((upper << COMP_E_CTL2_REF0_OFS)       //Set the upper reference voltage factor
                     & COMP_E_CTL2_REF0_MASK);
    COMP_E1->CTL2 &= ~COMP_E_CTL2_REF1_MASK;                //Clear the current lower reference voltage factor
    COMP_E1->CTL2 |= ((lower << COMP_E_CTL2_REF1_OFS)       //Set the lower reference voltage factor
                     & COMP_E_CTL2_REF1_MASK);
}

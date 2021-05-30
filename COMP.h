//Patrick Reedy; May 28, 2021

#ifndef COMP_H_
#define COMP_H_

#define COMPPORT                P6              //Port used for the comparator
#define COMPAIN                 BIT7            //Pin used for the comparator
#define VOLTDIVMAX              32              //Maximum voltage divider factor value
#define MAXVOLTAGE              3300000         //Maximum voltage for voltage divider
#define MAXTIME                 65535           //Maximum time with 24MHz used to sample waveforms
#define CPU_FREQ                24000000        //Frequency used to operate all devices
#define LOWERTHRESH             71              //Frequency that marks the lower threshold for optimization

#endif /* COMP_H_ */

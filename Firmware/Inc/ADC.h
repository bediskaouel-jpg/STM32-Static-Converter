/*
 * ADC.h
 *
 *  Created on: 30 oct. 2026
 *      Author: USER
 */

#ifndef ADC_H_
#define ADC_H_

#include <stdint.h>

void ADC_Init(void);
void ADC_Read(uint16_t* Amp , uint16_t* Freq);

#endif /* ADC_H_ */

/*
 * ADC.c
 *
 */

#include "ADC.h"
#include <stm32f4xx.h>
#include "config.h"

void ADC_Init(void)
{
	WriteRegisterField(&RCC->APB2ENR, RCC_APB2ENR_ADC1EN_Pos, RCC_APB2ENR_ADC1EN_Msk, 1U); // ADC1 Clock Enable

	/*ADC Clock Enable*/
	WriteRegisterField(&ADC->CCR, ADC_CCR_ADCPRE_Pos, ADC_CCR_ADCPRE_Msk, 1U); // APB2 Prescaler /4
	WriteRegisterField(&ADC1->CR1, ADC_CR1_RES_Pos, ADC_CR1_RES_Msk, 0U); // 12 Bit Resolution

	WriteRegisterField(&ADC1->SMPR2, ADC_SMPR2_SMP1_Pos, ADC_SMPR2_SMP1_Msk, 4U); // 84 cycles for conversion for ADC1 IN 1
	WriteRegisterField(&ADC1->SMPR2, ADC_SMPR2_SMP2_Pos, ADC_SMPR2_SMP2_Msk, 4U); // 84 cycles for conversion for ADC1 IN 2

	WriteRegisterField(&ADC1->SQR1, ADC_SQR1_L_Pos, ADC_SQR1_L_Msk, 0U); // 1 conversion per Sequence

	WriteRegisterField(&ADC1->CR2, ADC_CR2_CONT_Pos, ADC_CR2_CONT_Msk, 0U); // Single Conversion Mode
	WriteRegisterField(&ADC1->CR2, ADC_CR2_EOCS_Pos, ADC_CR2_EOCS_Msk, 1U); // Notify at the end of every Conversion
	WriteRegisterField(&ADC1->CR2, ADC_CR2_ALIGN_Pos, ADC_CR2_ALIGN_Msk, 0U); // Right Align
	WriteRegisterField(&ADC1->CR2, ADC_CR2_ADON_Pos, ADC_CR2_ADON_Msk, 1U); // AD On
}

void ADC_Read(uint16_t* Amp , uint16_t* Freq)
{
	static uint16_t ADC_Val = 0;
	/*Amplitude Value Read*/
	WriteRegisterField(&ADC1->SQR3, ADC_SQR3_SQ1_Pos, ADC_SQR3_SQ1_Msk, 1U); // First Conversion ADC1 IN1
	WriteRegisterField(&ADC1->CR2, ADC_CR2_SWSTART_Pos, ADC_CR2_SWSTART_Msk, 1U); // Conversion Start
	while((ADC1->SR & ADC_SR_EOC_Msk) != (1U << ADC_SR_EOC_Pos)); // Wait for end of Conversion 1
	ADC_Val = (uint16_t)ADC1->DR; // ADC IN1 Value
	*Amp = (ADC_Val * MAX_DC_VAL) / ADC_RANGE;

	/*Frequency Value Read*/
	WriteRegisterField(&ADC1->SQR3, ADC_SQR3_SQ1_Pos, ADC_SQR3_SQ1_Msk, 2U); // Second Conversion ADC1 IN2
	WriteRegisterField(&ADC1->CR2, ADC_CR2_SWSTART_Pos, ADC_CR2_SWSTART_Msk, 1U); // Conversion Start
	while((ADC1->SR & ADC_SR_EOC_Msk) != (1U << ADC_SR_EOC_Pos)); // Wait for end of Conversion 2
	ADC_Val = (uint16_t)ADC1->DR; // ADC IN2 Value
	*Freq = ((ADC_Val * (MAX_FREQ_VAL - MIN_FREQ_VAL) )/ ADC_RANGE) + MIN_FREQ_VAL;
}

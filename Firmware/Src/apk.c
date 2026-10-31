/*
 * apk.c
 *
 */

#include "apk.h"
#include "config.h"
#include <stdint.h>
#include <stm32f4xx.h>
#include <math.h>
#include <stdlib.h>

uint16_t Amp = 0;
uint16_t Freq = 0;

static volatile uint16_t IRQCounter = 0;


static volatile float m = 0.0f;
static volatile uint8_t SinTableSize = 0;

static int16_t * volatile SinRefTable = NULL;

void DC_AC_Conversion(void)
{
    static uint16_t OldFreq = 0;

    m = (float)Amp / (float)MAX_DC_VAL; // Amplitude variation

    if ((abs((int)OldFreq - (int)Freq) >= 2)) // Avoid ADC Small variations and recalculation for same Frequency
    {
        WriteRegisterField(&TIM1->DIER, TIM_DIER_UIE_Pos, TIM_DIER_UIE_Msk, 0U); // TIM interrupt Disable
        uint8_t OldSize = SinTableSize;
        SinTableSize = (uint8_t)(FREQ_PWM / Freq); // Calculate Number of PWM Periods in desired period
        if (OldSize != 0U)
        {
            IRQCounter = (uint16_t)(((uint32_t)IRQCounter * SinTableSize) / OldSize);   // Same phase for new Frequency
        }
        else
        {
            IRQCounter = 0;
        }

        free(SinRefTable);
        SinRefTable = malloc(SinTableSize*sizeof(int16_t)); // Table of Sin Values for number of PWM cycles in desired Period

        if (SinRefTable == NULL)
        {
            SinTableSize = 0;
            return;
        }

        for(uint8_t i =0; i<SinTableSize; i++)
        {
        	SinRefTable[i] = (int16_t)((float)COUNTER_MAX_VAL*sinf(((float)M_PI*2.0f * i)/SinTableSize)); // Table of Counter Compare values for every PWM cycle
        }

        OldFreq = Freq; // Old Frequency Memorization
    }
    WriteRegisterField(&TIM1->DIER, TIM_DIER_UIE_Pos, TIM_DIER_UIE_Msk, 1U); // TIM INT Enable
}

void DC_DC_Conversion(void)
{
	WriteRegisterField(&TIM1->DIER, TIM_DIER_UIE_Pos, TIM_DIER_UIE_Msk, 0U); // TIM INT Disable
	static uint32_t CCR1_Val = 0;
	m = (float)Amp / (float)MAX_DC_VAL; // Calculate Amplitude Variation
	CCR1_Val = (uint32_t)(m*(float)COUNTER_MAX_VAL);
	TIM1->CCR1 = (CCR1_Val << TIM_CCR1_CCR1_Pos); // Constant PWM for DC-DC Conversion
	TIM1->CCR2 = 0U; // PWM2 Disable
}

void TIM1_UP_TIM10_IRQHandler(void)
{
	static uint32_t CCR1_Val = 0;
	static uint32_t CCR2_Val = 0;
	if ((TIM1->SR & TIM_SR_UIF_Msk) == TIM_SR_UIF)
	{
		WriteRegisterField(&TIM1->SR, TIM_SR_UIF_Pos, TIM_SR_UIF_Msk, 0U); // Interrupt Flag Reset

		CCR1_Val = (uint32_t)((COUNTER_MAX_VAL + m*(float)SinRefTable[IRQCounter]) / 2); // Calculate PWM1 Duty cycle
		WriteRegisterField(&TIM1->CCR1, TIM_CCR1_CCR1_Pos, TIM_CCR1_CCR1_Msk, CCR1_Val); // Setting PWM 1 Duty cycle
		CCR2_Val = (uint32_t)((COUNTER_MAX_VAL - m*(float)SinRefTable[IRQCounter]) / 2); // Calculate PWM2 Duty Cycle
		WriteRegisterField(&TIM1->CCR2, TIM_CCR2_CCR2_Pos, TIM_CCR2_CCR2_Msk, CCR2_Val); // Setting PWM2 Duty cycle

		IRQCounter++; // Number of iterations
		if(IRQCounter >= SinTableSize) // End of Sin reference Period
		{
			IRQCounter = 0; // Iteration counter reset
		}
	}
}


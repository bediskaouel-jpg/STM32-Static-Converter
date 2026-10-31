/*
 * config.h
 *
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include <stdint.h>

#define MAX_DC_VAL 325U
#define MAX_FREQ_VAL 100U
#define MIN_FREQ_VAL 50U
#define ADC_RANGE 4095U
#define FREQ_PWM 10000U
#define COUNTER_MAX_VAL 8400

typedef enum
{
	DC_DC_Converter = 0,
	DC_AC_Inverter
}Convertor_Mode;

/*Clear Register Field Before Setting it*/
static inline void WriteRegisterField(volatile uint32_t *Reg, uint32_t Pos, uint32_t Msk, uint32_t val)
{
	*Reg &= ~(Msk); // Clear Field
	*Reg |= (val << Pos); // Set Register
}

#endif /* CONFIG_H_ */

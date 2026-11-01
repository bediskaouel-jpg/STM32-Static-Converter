/*
 * apk.h
 *
 */

#ifndef APK_H_
#define APK_H_

#include <stdint.h>

void Sin_Init(void);
void DC_AC_Conversion(void);
void DC_DC_Conversion(void);

extern uint16_t Amp;
extern uint16_t Freq;

#endif /* APK_H_ */

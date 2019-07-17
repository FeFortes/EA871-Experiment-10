/*!
 * @file util.h
 *
 * @date Feb 7, 2017
 * @author Fernando Fortes Granado
 */

#include <stdlib.h>

#ifndef UTIL_H_
#define UTIL_H_

#define GPIO_PIN(x)  ((1)<<(x)) ///< obtem o bit do pino x

void ConvUI2String (unsigned int j, char * s);

void ConvF2String (float j, char * s);

void ConvString2UI (char * s, unsigned int *j);

void ConvString2F (char * s, float *j);

void ConvString2HHMMSS (char * s, uint8_t *horas, uint8_t *minutos, float *segundos);

void ConvMMSS2String(uint8_t MM, float SS, char *s);

void ConvHHMMSS2String(uint8_t HH, uint8_t MM, float SS, char *s);

#endif /* UTIL_H_ */

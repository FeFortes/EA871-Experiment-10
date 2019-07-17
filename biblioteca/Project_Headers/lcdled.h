/*!
 * lcdled.h
 *
 *  Created on: Feb 8, 2017
 *      Author: Fernando Fortes Granado
 */

#ifndef LCDLED_H_
#define LCDLED_H_

#include "derivative.h"
#include "util.h"
/*
 * Enumera&ccedil&atilde;o do sinal de Enable
 */
typedef enum _tipo_enable {
	LCD,     /*!< sinal Enable do LCD */
	LEDS     /*!< sinal Enable dos Leds */
} tipo_enable;

/*
 * Estrutura para representar a instrução do LCD e o seu tempo de processamento 
 */
typedef struct _lcd {
	uint8_t cop;
	uint8_t tempo;
} lcd;

#define GPIO_PIN(x)  ((1)<<(x)) ///< obtem o bit do pino x
#define LCD_FUNCTION_SET 0x38
#define LCD_DISPLAY_CONTROL 0x0C
#define LCD_DISPLAY_CLEAR 0x01
#define LCD_ENTRY_MODE_SET 0x06


void initLcdledGPIO(void);
void pulso(tipo_enable p, uint8_t t);
void RS(uint8_t l);
void enviaLCD(char c, uint8_t t);
void initLCD(void);
void mandaString(char * s);
void enviaLed(char c);
void limpaLCD(void);
void criaBitmap(uint8_t end, uint8_t *bitmap);

#endif /* LCDLED_H_ */

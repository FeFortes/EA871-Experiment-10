/*!
 * \file handler.h
 *
 * \date Feb 15, 2017
 * \author Wu Shin-Ting e Fernando Granado
 */

#ifndef HANDLER_H_
#define HANDLER_H_

#include "derivative.h"
#include "util.h"

void PORTA_IRQHandler(void);
void SysTick_Handler(void);
void PIT_IRQHandler(void);
void UART0_IRQHandler(void);
void ADC0_IRQHandler (void);
void LPTimer_IRQHandler (void);

#endif /* HANDLER_H_ */

/*!
 * \file systick.h
 *
 * \date Feb 14, 2017
 * \author Fernando Fortes Granado
 */

#ifndef SYSTICK_H_
#define SYSTICK_H_

#include "derivative.h"

void initSysTick (unsigned int periodo);

void enableSysTickInterrup(void);

void disableSysTickInterrup(void);

#endif /* SYSTICK_H_ */

/*!
 * pit.h
 *
 *  Created on: Feb 15, 2017
 *      Author: Fernando Fortes Granado
 */

#ifndef PIT_H_
#define PIT_H_

#include "derivative.h"
#include "util.h"

void initPIT(uint8_t div4, unsigned int periodo);
void enablePITInterrup(void);
void disablePITInterrup(void);

#endif /* PIT_H_ */

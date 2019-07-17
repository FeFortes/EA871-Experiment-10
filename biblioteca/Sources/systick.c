/*!
 * \file systick.c
 *
 * \date Feb 14, 2017
 * \author Wu Shin-Ting e Fernando Granado
 */

#include "systick.h"

/*!
 * \fn initSysTick (void)
 * \brief Inicializa o temporizador SysTick.
 */
void initSysTick (unsigned int periodo) {
	SYST_RVR = SysTick_RVR_RELOAD(periodo);   
	SYST_CVR = SysTick_CVR_CURRENT(0x0);   ///< zerar o contador             
	SYST_CSR |= (SysTick_CSR_CLKSOURCE_MASK |    ///< clock source = core clock 
			SysTick_CSR_ENABLE_MASK); ///< habilita SysTick           
}

/*!
 * \brief Habilita interrup&ccedil;&atilde;o do SysTick
 */
void enableSysTickInterrup (void){
	SYST_CSR |= SysTick_CSR_TICKINT_MASK;	
}

/*!
 * \brief Desabilita interrup&ccedil;&atilde;o do SysTick
 */
void disableSysTickInterrup (void){
	SYST_CSR &= ~SysTick_CSR_TICKINT_MASK;	
}

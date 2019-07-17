/*!
 * \file pit.c
 *
 * \date Feb 15, 2017
 * \author Wu Shin-Ting e Fernando Granado
 */

#include "pit.h"

/**
 * \fn InitPIT (void)
 * 
 * \brief Configura o timer 0 do PIT para que gere um sinal peri&oacute;dico 2*periodo/20971520.
 * \param[in] div4 divisor de frequ&ecirc;ncia do n&uacute;cleo
 * \param[in] periodo contagem m&aacute;xima
 */
void initPIT(uint8_t div4, unsigned int periodo) {
	SIM_SCGC6 |= SIM_SCGC6_PIT_MASK;        ///< TPM1=SIM_SCGC6[23]=1 (Clock gate de PIT habilitado)
	
	/*
	 * Clock do PIT &eacute; bus clock (Tabela 5-2 de KL-25 Sub-Family Reference Manual)
	 * frequ&ecirc;ncia de bus clock = 
	 * 	            (clock do n&uacute;cleo)/(SIM_CLKDIV1[OUTDIV1]*SIM_CLKDIV1[OUTDIV4])
	 * 	            (Figura 5-1 de KL-25 Sub-Family Reference Manual)
	 * frequ&ecirc;ncia do núcleo = 20.971520 MHz
	 * Periodo_{PIT} =PIT_LDVAL0[TSV]/(frequ&ecirc;ncia de bus clock)          
	 * 	              
	 */
	//OUTDIV1=SIM_CLKDIV1[31:28] &eacute carregado no reset com 0000 (dividido por 1)
	SIM_CLKDIV1 &= ~SIM_CLKDIV1_OUTDIV4(0b111);  ///< OUTDIV1=SIM_CLKDIV1[18:16]=0b000
	SIM_CLKDIV1 |= SIM_CLKDIV1_OUTDIV4(div4);  ///< OUTDIV1=SIM_CLKDIV1[18:16]=0b000 (dividido por 1)

	/*
	 * Reinicializa novo ciclo de contagem
	 */
	PIT_TCTRL0 &= ~(PIT_TCTRL_TEN_MASK | ///< TEN=PIT_CTRL0[31]=0 (desativa timer 0)
			PIT_TCTRL_TIE_MASK );    ///< TIE=PIT_CTRL0[30]=0 (desabilita interrup&ccedil;&atilde; do timer 0)
	PIT_LDVAL0 = PIT_LDVAL_TSV(periodo);       ///< TSV=PIT_LDVAL0[31:0]=x              
	
	PIT_TFLG0 |= PIT_TFLG_TIF_MASK;      ///< TIF=PIT_TFLG0[31]: w1c (limpa pendências)

	PIT_TCTRL0 |= (PIT_TCTRL_TEN_MASK);  ///< TEN=PIT_CTRL0[31]=1 (ativa timer 0)

	PIT_TCTRL0 &= ~PIT_TCTRL_CHN_MASK;   ///< CHN=PIT_CTRL0[29]=0 (n&atilde;o encadeado com o predecessor)
	
	PIT_MCR &= ~(PIT_MCR_FRZ_MASK |      ///< FRZ=PIT_MCR[31]=0 (continua contagem no modo Debug)
		PIT_MCR_MDIS_MASK );         ///< MDIS=PIT_MCR[30]=0 (habilita clock)
}

/*!
 * \brief Habilita a interrup&ccedil;&atilde;o do temporizador PIT
 */
void enablePITInterrup(void) {
	PIT_TCTRL0 |= PIT_TCTRL_TIE_MASK;    ///< TIE=PIT_CTRL0[30]=1 (habilita interrup&ccedil;&atilde;o do timer 0)
}

/*!
 * \brief Desabilita a interrup&ccedil;&atilde;o do temporizador PIT
 */
void disablePITInterrup(void) {
	PIT_TCTRL0 &= ~PIT_TCTRL_TIE_MASK;    ///< TIE=PIT_CTRL0[30]=0 (desabilita interrup&ccedil;&atilde;o do timer 0)
}

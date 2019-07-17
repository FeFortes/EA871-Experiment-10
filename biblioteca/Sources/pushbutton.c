/*!
 * @file pushbutton.c
 *
 * @date Feb 7, 2017
 * @author Wu Shin-Ting e Fernando Granado
 */

#include "pushbutton.h"
#include "atrasos.h"

/**
* @brief Inicializa botoeiras 
*/
void initPushbutton (void) {
    /*!  Habilita os clocks dos módulos */
    SIM_SCGC5 |= (SIM_SCGC5_PORTA_MASK) ;    

    /*! Configura pino A4 como GPIO (MUX = 001) */
    PORTA_PCR4 &= ~PORT_PCR_MUX(111);
    PORTA_PCR4 |= PORT_PCR_MUX(001);

    /*! Configura pino A5 como GPIO (MUX = 001) */
    PORTA_PCR5 |= PORT_PCR_MUX(001);

    /*! Configura pino A12 como GPIO (MUX = 001) */
    PORTA_PCR12 |= PORT_PCR_MUX(001);

    /*! Configura a direcao dos pinos 4, 5 e 12 da PORTA como entrada */
    GPIOA_PDDR &= ~(GPIO_PDDR_PDD(GPIO_PIN(4)) |
    		GPIO_PDDR_PDD(GPIO_PIN(5)) |
    		GPIO_PDDR_PDD(GPIO_PIN(12)));

}

/**
 * @brief Habilita a interrup&ccedil;&atilde;o dos pinos em que pushbutton est&atilde;o ligados
 */
void enablePushbuttonIRQA(void) {
	PORTA_PCR4 |= PORT_PCR_ISF_MASK | 
				  PORT_PCR_IRQC(0b1010);       ///< sens&iacute;vel a borda de descida
	PORTA_PCR5 |= PORT_PCR_ISF_MASK | 
				  PORT_PCR_IRQC(0b1011);       ///< sens&iacute;vel aduas bordas
	PORTA_PCR12 |= PORT_PCR_ISF_MASK | 
		      	  PORT_PCR_IRQC(0b1010);               ///< sens&iacute;vel a borda de descida
}

/**
* @brief L&ecirc; o valor do pino 12 da porta A
* @param [in] pta 0 (PTA5) ou 1 (PTA12)
* @return estado do bot&atilde;o (0 apertado; 1 normal)
*/
short le_pta (enum _tipo_pta pta) {
	volatile unsigned int a;
	short tmp;

	a = GPIOA_PDIR;       /*! Registrador de entrada na PORTA (32 bits) */
	switch (pta) {
	case PTA4: tmp = (short) (a & GPIO_PIN(4));
	break;
	case PTA5: tmp = (short) (a & GPIO_PIN(5));
	break;
	case PTA12: tmp = (short) (a & GPIO_PIN(12));
	}
	return tmp;
}


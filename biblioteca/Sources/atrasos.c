/*!
 * @file atrasos.c
 * @brief Gerencia os atrasos por la&ccedil;o de espera
 * @date Ago 27, 2017
 * @author Fernando Fortes Granado
 */


/*!
 * @brief gera um atraso correspondente a m&uacute;ltiplos de 10us
 * @param t n&uacute;mero de vezes
 */
void delay10us(unsigned int t)
{
	asm("iteracao1:"
		"movs r1, #69\n\t"
		"iteracao2:"
		"sub r1, #1\n\t"
		"bne iteracao2\n\t"
		"sub r0, #1\n\t"
		"bne iteracao1\n\t"
		);
}



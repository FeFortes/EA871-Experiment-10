/*
 * @file util.c
 *
 * @date Feb 8, 2017
 * @author Wu Shin-Ting e Fernando Granado
 */

#include "util.h"

/*!
 * \fn ConvUI2String(unsigned int j, char * s)
 * \brief Converte em ASCII o inteiro sem sinal j.
 * \param[in]  j valor inteiro sem sinal
 * \param[out] s representa&ccedil;&atilde;o em ASCII
 */
void ConvUI2String (unsigned int j, char * s) {
	short k = 0;
	char aux[11];
	/*!
	 * Passo 1: extrai os algarismos decimais
	 */
	if (j == 0) { ///< Caso especial
		aux[k] = 0;
		k++;
	}
	while (j) {
		aux[k] = j % 10;
		j = j / 10;
		k++;
	}
	/*!
	 * Passo 2: representa em ASCII os d&iacute;gitos decimais
	 */
	while (k > 0) {
		*s = (aux[k-1] + 0x30);
		s++;                    ///< incrementa o endere&ccedil;o por unidade do tipo de dados
		k--;
	}
	*s = 0;                     ///< adiciona o terminador '\0'
}

/*!
 * \fn ConvF2String(float j, char * s)
 * \brief Converte em ASCII um valor em ponto flutuante j com uma casa decimal.
 * \param[in]  j valor em ponto flutuante
 * \param[out] s representa&ccedil;&atilde;o em ASCII com uma casa decimal
 */
void ConvF2String (float j, char * s) {
	int decimal[5], i, k;
	if(j<1){ /*! escreve 0 em ASCII no apontador caso o numero seja menor que 1 */
		*s=0x30;
		s++;
	}
	/*! passo 1: extrair os digitos de j */
	
	/*!numero multiplicado por 10 sem casas decimais*/
	k = (int)(j*10); /*numero multiplicado por 10 sem casas decimais*/

	if (k) {
		for (i=0; k!=0; i++) {
			decimal[i] = k % 10; /* captura os digitos de k e armazena no vetor decimal na ordem invertida */
			k=k/10;
		}
		i--; /*! ajuste para nao ler lixo */
		/*! passo 2: converte para ASCII os digitos antes do ponto */
		while (i>0) {
			*s = decimal[i]+0x30; /*! escreve no apontador os digitos antes do ponto na ordem correta */
			s++;
			i--;
		}
		*s = '.'; /*! converte o ponto para ASCII */
		s++;
		*s = decimal[0]+0x30; /*! escreve o digito depois do ponto */ /*! converte uma casa decimal para ASCII */
		s++;
		

	
	}
	else { /*! caso especial em que o numero e 0.0xxxx*/
		*s = '.';
		s++;
		*s= 0x30;
		s++;
	}

	*s = 0;

}

/*!
 * \fn ConvString2UI(char * s, unsigned int *j)
 * \brief Converte uma string de caracteres num valor inteiro sem sinal j.
 * \param[in] s representa&ccedil;&atilde;o em ASCII com uma casa decimal
 * \param[out]  j valor inteiro sem sinal
 */
void ConvString2UI (char * s, unsigned int *j) {
	
	*j = 0;
	/*!
	 * Assume-se que a string s s&oacute; contem d&iacute;gitos v&aacute;lidos
	 */
	/*!
	 * Converte a parte inteira
	 */
	while (*s != '\0') {
		*j = (*j)*10 + (*s - 0x30);
		s++;
	}
}

/*!
 * \fn ConvString2F(char * s, float *j)
 * \brief Converte uma string de caracteres num valor em ponto flutuante j.
 * \param[in] s representa&ccedil;&atilde;o em ASCII com uma casa decimal
 * \param[out]  j valor em ponto flutuante
 */
void ConvString2F (char * s, float *j) {
	int esquerda, direita, decimal;
	esquerda = (int)(*s)-0x30;
	s++;
	direita = (int)(*s)-0x30;
	s = s + 2; /*! pula o ponto*/
	decimal = (int)(*s)-0x30;
	
	*j = (float)(10*esquerda);
	*j = (float)(*j + direita);
	*j = *j + (float)decimal/10.0;
}

/*!
 * \fn ConvString2HHMMSS(char * s, uint8_t *horas, uint8_t *minutos, float *segundos)
 * \brief Converte uma string de caracteres em horas, minutos e segundos
 * \param[in] s string no formato HH:MM:SS.s
 * \param[out]  horas horas
 * \param[out]  minutos minutos
 * \param[out]  segundos segundos
 */
void ConvString2HHMMSS (char * s, uint8_t *horas, uint8_t *minutos, float *segundos) {
	
	///Ting: para economizar memoria voce poderia ter usado tipo uint8_t ...
	int Hesquerda, Hdireita, Mesquerda, Mdireita;
	Hesquerda = (int)*s-0x30;
	s++;
	Hdireita = (int)*s-0x30;
	s = s + 2;
	Mesquerda = (int)*s-0x30;
	s++;
	Mdireita = (int)*s-0x30;
	s = s + 2;
	
	ConvString2F (s, segundos);
	
	*horas = Hdireita + 10*Hesquerda;
	*minutos = Mdireita + 10*Mesquerda;

	
}


/*!
 * \fn ConvMMSS2String(uint8_t MM, float SS, char * s)
 * \brief Converte no formato MM:SS.S em ASCII o valor em minutos e o valor em segundos.
 * \param[in]  MM valor em minutos
 * \param[in]  SS valor em segundos
 * \param[out] s representa&ccedil;&atilde;o em ASCII com uma casa decimal em segundos
 * 
 */
void ConvMMSS2String(uint8_t MM, float SS, char *s) {
	unsigned int minutos;
	minutos = (unsigned int)MM;
	if(minutos<10) { /*! garante 2 digitos na representacao dos minutos*/
		*s=0x30;
		s++;
		ConvUI2String(minutos, s);
		s++;
		*s=':';
		s++;
	}
	else { 
		ConvUI2String(minutos, s);
		s=s+2;
		*s=':';
		s++;
	}
	if(SS<10) { /*! garante 3 digitos na representacao dos segundos*/
		*s=0x30;
		s++;
		ConvF2String (SS, s);
	}
	else {
		ConvF2String (SS, s);
	}
}


/*!
 * \fn ConvHHMMSS2String(uint8_t HH, uint8_t MM, float SS, char * s)
 * \brief Converte no formato MM:SS.S em ASCII o valor em minutos e o valor em segundos.
 * \param[in]  HH valor em horas
 * \param[in]  MM valor em minutos
 * \param[in]  SS valor em segundos
 * \param[out] s representa&ccedil;&atilde;o em ASCII com uma casa decimal em segundos
 * 
 */
void ConvHHMMSS2String(uint8_t HH, uint8_t MM, float SS, char *s) {
	unsigned int H;
	H = (unsigned int) HH;
	if(HH < 10) {
		*s=0x30;
		s++; 
		ConvUI2String(H, s); /*! escreve as horas*/
		s++;
		*s=':';
		s++;
	}
	else { 
		ConvUI2String(H, s); /*! escreve as horas */
		s=s+2;
		*s=':';
		s++;
	}
	ConvMMSS2String(MM, SS, s); /*! escreve os minutos e segundos */
}


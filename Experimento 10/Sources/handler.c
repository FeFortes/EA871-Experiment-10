/*!
 * \file handler.c
 *
 * \date Feb 15, 2017
 * \author Wu Shin-Ting e Fernando Granado
 */

#include "derivative.h"
#include "handler.h"
#include "ledRGB.h"
#include "lcdled.h"
#include "estrutura.h"
#include "pit.h"
#include "atrasos.h"
#include "adc.h"

#include "uart.h"

extern short buffer_ADC[];
extern uint8_t flag_ADC0_temp, flag_ADC0_pot;
/*!
 * buffer circular de recep&ccedil;&atilde;o
 */
extern buffer_circular buffer_receptor;

/*!
 * buffer circular de transmiss&atilde;o
 */
extern buffer_circular buffer_transmissor;

extern uint8_t flag_recepcao; 


/*!
 * vetor de cores do led
 */
extern cor cores_estaticas[];

/*!
 * vetor de intervalos de tempo pr&eacute;-definidos 
 */
extern float intervalos[];

/*!
 * vari&aacute;vel de cor/intervalo de tempo selecionada
 */         
extern uint8_t indice5;            

/*!
 * vari&aacute;vel do modo (temporizador_parado=0, temporizador_configurando=1, 
 * cron&ocirc;metro_correndo=2, cron&ocirc;metro_parado=3, temporizador_correndo=4)
 */         
extern uint8_t modo;            

/*!
 * vari&aacute;vel do contador de tempo 
 */         
extern uint32_t copia_tempo, tempo, tempo_relogio, ultimo_tempo, buzzer, temporizador;

/*!
 * vari&aacute;veis de estado
 */
extern uint8_t flag_muda_cor, flag_muda_tempo, flag_muda_estado, flag_muda_relogio;

extern cor_pf cores[16];
/*!
 * \fn UART0_IRQHandler (void)
 * \brief Rotina de serviço para UART0 (Vetor 28)
 */
void UART0_IRQHandler(void) {
	char d;

	/*!
	 * S&oacute; h&aacute; uma exce&ccedil;&atilde;o associada ao UARTx.
	 * Por&eacute;m cada m&oacute;dulo UARTx disp&oacute;e de registradores de estado 
	 * que indicam diferentes condi&ccedil;&otilde;es de exce&ccedil;&atilde;o.
	 */
	if (UART0_S1 & UART0_S1_RDRF_MASK)          
	{
		/*!
		 * Evento gerado pelo receptor
		 */
		d = UART0_D;                 
		/*!
		 * insere o item no buffer circular
		 */
		while (!bufCircular_insere (&buffer_receptor, d));

		bufCircular_insere(&buffer_transmissor, d);
		enableTEInterrup();  ///< habilita a interrup&ccedil;&atilde;odo transmissor
		
		
		if (d == '\r') {  ///< at&eacute; entrar <CR> para indicar o fim de uma unidade de informa&ccedil;&atilde;o
			bufCircular_insere(&buffer_transmissor, '\n');  ///mudar de linha ...
			flag_recepcao = 1;             ///< levantar a bandeira
		}
	} else if (UART0_S1 & UART0_S1_TDRE_MASK) {
		/*!
		 * Evento gerado pelo transmissor
		 */
		if (bufCircular_vazia(&buffer_transmissor))
			UART0_C2 &= ~UART_C2_TIE_MASK;
		else 
			UART0_D = bufCircular_remove(&buffer_transmissor);
	}	
}



void PORTA_IRQHandler(void) {
	/*!
	 * Caso a botoeira apresente bouncing, insira um delay antes do processamento
	 */
	if (PORTA_PCR4 & PORT_PCR_ISF_MASK) {
		/*!
		 * Temporizador: contagem regressiva
		 */
		if (GPIOA_PDIR & GPIO_PIN(4)) { 
			/*! Soltou */
			enviaLed(0x00);               ///< apaga leds vermelhos indicadores
		} else {
			/*! Apertou */

				if (modo == 3 || modo == 0 || modo == 6) {
					/*!
					 * Dispara o temporizador
					 */
					enviaLed(0xFF);               ///< liga leds vermelhos indicadores
					reconfigModoTPM1Cn(1,0b0101);          ///< modo "toggle output on match" 
					reconfigValorTPM1Cn(1,TPM1_CNT);       ///< setar o valor do contador do modulo 1 no seu canal 1 
					/*na linha de cima deve escrever Conter ovf?*/
					enableTPM1CnInterrup(1);                ///< habilita interrup&ccedil;&atilde;odo canal 1 (controle do buzzer) e contagem de tempos
					modo = 4;
					/*!
					 * Conforme
					 * ftp://ftp.dca.fee.unicamp.br/pub/docs/ea871/ARM/KL25P80M48SF0RM.pdf#page=555
					 * "When switching from one channel mode to a different channel mode, 
					 * the channel must first be disabled and this must be acknowledged in 
					 * the LPTPM counter clock domain."
					 */
					alteraCorLedRGBPWM((uint16_t)(cores[indice5].vermelho*COUNTER_OVF),  /*para que serve esse altera cor? pode tirar?*/
							(uint16_t)(cores[indice5].verde*COUNTER_OVF),
							(uint16_t)(cores[indice5].azul*COUNTER_OVF));
					ligaLedRGBPWM (cores[indice5].vermelho, cores[indice5].verde, 
							cores[indice5].azul);
					copia_tempo = temporizador*3;  ///< o intervalo do temporizador é 3 vezes o intervalo entre as varia&ccedil;&otilde;es de cores
					flag_muda_estado = 1;
					flag_muda_tempo = 1;
			}
		}
		PORTA_PCR4 |= PORT_PCR_ISF_MASK;            ///< abaixa a bandeira de interrup&ccedil;&atilde;o
	}
}


/**
 * \fn PIT_IRQHandler (void)
 * 
 * \brief Rotina de serviço para PIT (Vetor 38, IRQ22, prioridade em NVICIPR5[23:22])
 * 
 */
void PIT_IRQHandler(void) {
	/*!
	 * incremanta tempo
	 */ 	
	tempo_relogio++; /*!atualiza relogio*/
	if(modo == 6) /*! mostrar o valor atual do relogio*/
		flag_muda_tempo = 1;                ///< levanta a bandeira de aviso
	PIT_TFLG0 |= PIT_TFLG_TIF_MASK;     ///< TIF=PIT_TFLG0[31]: w1c (limpa pendências)
}

/**
 * \brief non-maskable interrupt handler
 * \note A fun&ccedil;&atilde;o do PTA4 &eacute; NMI. Se habilitar no NVIC o processamento
 * da interrup&ccedil&atilde;o da porta A, pode-se gerar interrup&ccedil;&otilde;es inesperadas.
 * Uma solu&ccedil;&atilde;o &eacute; criar uma rotina de tratamente vazia 
 * (entra e sai sem fazer nada). 
 */



/*!
 * \fn FTM0_IRQHandler (void)
 * \brief Rotina de serviço para TPM0 (Vetor 33, IRQ17, prioridade em NVICIPR4[15:14])
 * Acender leds vermelhos do shield FEEC quando PTA5 for pressionado.
 * Variar a cor do ledRGB conforme o intervalo de tempo em que PTA5 fica pressionado.
 */
void FTM0_IRQHandler(void) {
	/*!
	 * A ordem de tratamento dos eventos &eacute; importante.
	 * Ordem da mais para menos priorit&aacute;ria.
	 */
	if (TPM0_STATUS & TPM_STATUS_CH0F_MASK) {
		temporizador++;
		if (temporizador%640 == 0) { /*! apos completar 640 ciclos passou-se 0.5s */
			/*!
			 * Intervalo de 0.5s
			 */
			indice5 = (indice5+1) % MAX_COR;
			/*!
			 * Conforme
			 * ftp://ftp.dca.fee.unicamp.br/pub/docs/ea871/ARM/KL25P80M48SF0RM.pdf#page=555
			 * "When switching from one channel mode to a different channel mode, 
			 * the channel must first be disabled and this must be acknowledged in 
			 * the LPTPM counter clock domain."
			 */
			alteraCorLedRGBPWM((uint16_t)(cores[indice5].vermelho*COUNTER_OVF), 
					(uint16_t)(cores[indice5].verde*COUNTER_OVF),
					(uint16_t)(cores[indice5].azul*COUNTER_OVF));
			ligaLedRGBPWM (cores[indice5].vermelho, cores[indice5].verde, 
					cores[indice5].azul);
			if (indice5 == 0) temporizador = 640;           ///< valor do temporizador deve ser computado cicilicamente 
			flag_muda_cor = 1;
		}
		TPM0_C0SC |= TPM_CnSC_CHF_MASK;     		///< CHF=TPM0_C0SC[7]: w1c (limpa a pend&ecirc;ncia do canal)
	} else if (TPM0_STATUS & TPM_STATUS_CH2F_MASK) {
		if (GPIOA_PDIR & GPIO_PIN(5)) { ///< soltou PTA5
			if (modo == 1) {
				

				enviaLed(0x00);               ///< apaga leds vermelhos indicadores
				apagaLedRGBPWM();
				disableTPM0CnInterrup(0);	           ///< desabilita a interrup&ccedil;&atilde;o do canal 0 (controle do led RGB)
				/*!
 				 * Quantidade total de ticks de rel&oacute;gio
 				 */
				temporizador = temporizador - (temporizador % 640); /*! faz com que cada cor tenha um unico tempo associado */

				modo = 6;
				flag_muda_estado = 1;
			}
		} else { ///< apertou PTA5
				if (modo == 3 || modo == 0 || modo == 6) {
				/*!
				 * Inicia a configura&ccedil;&atilde;o do temporizador
				 */
				enviaLed(0xFF);               ///< liga leds vermelhos indicadores
				reconfigModoTPM0Cn(0,0b0101);          ///< modo "toggle output on match" 
				reconfigValorTPM0Cn(0,TPM0_C2V);       ///< setar o valor do contador capturado pelo canal 2 (PTA5) no canal 0			
				enableTPM0CnInterrup(0);                ///< habilita interrup&ccedil;&atilde;odo canal 0 (controle da varia&ccedil;&atilde;o de cored do led RGB)
				temporizador = 640;                     ///< inicializar a contagem dos ticks
				indice5 = 0;                   ///< inicializar a sequ&ecirc;ncia de cores
				/*!
				 * Conforme
				 * ftp://ftp.dca.fee.unicamp.br/pub/docs/ea871/ARM/KL25P80M48SF0RM.pdf#page=555
				 * "When switching from one channel mode to a different channel mode, 
				 * the channel must first be disabled and this must be acknowledged in 
				 * the LPTPM counter clock domain."
				 */
				alteraCorLedRGBPWM((uint16_t)(cores[indice5].vermelho*COUNTER_OVF), 
						(uint16_t)(cores[indice5].verde*COUNTER_OVF),
						(uint16_t)(cores[indice5].azul*COUNTER_OVF));
				ligaLedRGBPWM (cores[indice5].vermelho, cores[indice5].verde, 
						cores[indice5].azul);
				modo = 1;                           ///< altera para estado temporizador_correndo
				flag_muda_estado = 1;
				flag_muda_cor = 1;                  ///< levanta a bandeira indicadora
			}
		} 
		TPM0_C2SC |= TPM_CnSC_CHF_MASK;     		///< CHF=TPM0_C2SC[7]: w1c (limpa a pend&ecirc;ncia do canal)
	} 
}

/*!
 * \fn FTM1_IRQHandler (void)
 * \brief Rotina de serviço para TPM1 (Vetor 34, IRQ18, prioridade em NVICIPR4[23:22])
 * Controlar um cron&ocirc;metro
 */
void FTM1_IRQHandler(void) {
	/*!
	 * Para contornar bouncing da botoeira do meu shield
	 * Ajuste foi emp&iacute;rico
	 */
	delay10us(20);
	/*!
	 * A ordem de tratamento dos eventos &eacute; importante.
	 * Ordem da mais para menos priorit&aacute;ria.
	 */
	if (TPM1_STATUS & TPM_STATUS_CH0F_MASK) {
		if (GPIOA_PDIR & GPIO_PIN(12)) { ///< soltou PTA12
			/*!  Soltou! */
			enviaLed(0x00);               ///< apaga leds vermelhos sinalizadores do estado PTA4 pressionado 			
		} else { ///< apertou PTA12
			/*!
			 * Apertou: iniciar/parar
			 */
			enviaLed(0xFF);               ///< acende leds vermelhos sinalizadores do estado PTA4 pressionado
			
				if ((modo == 3 || modo == 0 || modo == 6) && (buzzer == 0)) { /*! modo menu sem buzzer tocando*/
				/*!
				 * Iniciar o cron&ocirc;metro
				 */
				modo = 2;
				tempo = 0;
				reconfigModoTPM1Cn(1,0b0101);          ///< modo "toggle output on match" 

				reconfigValorTPM1Cn(0,TPM1_C0V);       ///< setar o valor do contador capturado pelo canal 2 (PTA5) no canal 0			

				enableTPM1CnInterrup(1);                ///< habilita interrup&ccedil;&atilde;o peri&oacute;dica do canal 1 (controle do buzzer e contagem do tempo)
				flag_muda_estado = 1;
				flag_muda_tempo = 1;                ///< levanta a bandeira de aviso
			} else if (modo == 2){
				/*!
				 * Parar o cron&ocirc;metro
				 */
				reconfigModoTPM1Cn(1,0b0000);          ///< modo "desliga" o canal 1
				disableTPM1CnInterrup(1);                ///< desabilita interrup&ccedil;&atilde;odo peri&oacute;dica do canal 1 (controle do buzzer e contagem do tempo)
				modo = 3;
				flag_muda_tempo = 1;                ///< atualizar o &uacute;ltimo instante
				ultimo_tempo = tempo;
			}
		}
		TPM1_C0SC |= TPM_CnSC_CHF_MASK;     		///< CHF=TPM1_C0SC[7]: w1c (limpa a pend&ecirc;ncia do canal)
	} else if (TPM1_STATUS & TPM_STATUS_CH1F_MASK) { 
		if (modo == 2) {             ///< incrementa o tempo do cron&ocirc;metro
			tempo += 1;
			if((tempo%128)==0) flag_muda_tempo = 1;                ///< atualiza o cronometro a cada 0.1s
		} else if (modo == 4) {
			copia_tempo -= 1;                   ///< decrementa o tempo do temporizador
			flag_muda_tempo = 1;                ///< levanta a bandeira de aviso
			if (copia_tempo < 1) {
				/*!
				 * Contagens restantes s&atilde;o ignoradas
				 */
				copia_tempo = 0;
				apagaLedRGBPWM();
				disableTPM1CnInterrup(1);                ///< desabilita interrup&ccedil;&atilde;odo canal 1 (controle do buzzer e contagem do tempo)
			}

		} else if (modo == 6) {

			if (buzzer)
				buzzer--;
			else {
				reconfigModoTPM1Cn(1,0b0000);          ///< modo "desliga" o canal 1
				setaMux(&PORT_PCR_REG(PORTE_BASE_PTR,21), 0b000);		///< desabilita porta E
				disableTPM1CnInterrup(1); 
			}
		}
		TPM1_C1SC |= TPM_CnSC_CHF_MASK;     		///< CHF=TPM1_C1SC[7]: w1c (limpa a pend&ecirc;ncia do canal)
	}
}

/*! 
 * \fn ADC0_IRQHandler (void)
 * \brief Rotina de servi&ccedil;o para atender ADC0 
 */
void ADC0_IRQHandler (void) {
	static uint8_t ind=0;
	
	if( ADC0_SC1A & ADC_SC1_COCO_MASK )
	{
		switch (ind) {
		case 0:
			  buffer_ADC[ind] = ADC0_RA;   ///< ler o registrador do conversor (limpa COCO)
			  selecionaCanalADC0 (0x1A);
			  ind++;
			  flag_ADC0_pot = 1;
			  break;
		case 1:
			  buffer_ADC[ind] = ADC0_RA;   ///< ler o registrador do conversor (limpa COCO)
			  disableInterrupADC0 ();
			  ind = 0;
			  flag_ADC0_temp = 1;           ///< levanta a bandeira de ter dados prontos no ADC0
			  break;
		}
	}
}

/*!
 * \fn LPTMR0_IRQHandler (void)
 * \brief Rotina de servi&ccedil;o para atender LPTMR0
 */
void LPTimer_IRQHandler(void) {
	/*!
	 * Fa&ccedil;a a medida e habilite a interrup&ccedil;&atilde;o do ADC0
	 */
	selecionaCanalADC0 (0x09);
	enableInterrupADC0();
	
	LPTMR0_CSR |= LPTMR_CSR_TCF_MASK;   ///< Limpar pend&ecirc;ncias (w1c)
}

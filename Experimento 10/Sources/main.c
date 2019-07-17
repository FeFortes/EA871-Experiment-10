/*!
 * \file main.c
 * \brief Comunica&ccedil;&atilde;o Serial Ass&iacute;ncrono UART.
 * \details Este c&oacute;digo testa a configura&ccedil;&atilde;o no modo de 
 *          interrup&ccedil;&atilde;o do m&oacute;dulo UART0 dispon&iacute;vel no MKZ25Z128. 
 *          &Eacute; demonstrado o envio de uma string de nome de cor e o eco 
 *          dos caracteres digitados.
 * \mainpage Projeto de envio de nome de cores conforme a letra digitada.
 * \author Wu Shin-Ting e Fernando Granado
 * \date 30/07/2017
 * \note Documenta&ccedil;&atilde;o do c&oacute;digo no formato de Doxygen:
 *       http://mcuoneclipse.com/2014/09/01/automatic-documentation-generation-doxygen-with-processor-expert/
 */
//MOSTRA TEMPERATURA NO CANTO SUPERIOR DIREITO NO ESTADO 6 E ATUALIZA DINAMICAMENTE
#include "derivative.h" 
#include "ledRGB.h"
#include "uart.h"
#include "nvic.h"
#include "estrutura.h"
#include "stdlib.h"
#include "string.h"
#include "atrasos.h"
#include "pushbutton.h"
#include "lcdled.h"
#include "pit.h"
#include "port.h"
#include "lcdled.h"
#include "util.h"
#include "adc.h"
#include "lptmr.h"
/*!
 * buffers de dados de interface com UART
 */
buffer_circular buffer_receptor, buffer_transmissor;

#define COUNTER_OVF 1024      ///< valor m&aacute;ximo do TPMx_CNT
#define MAX 0xfff              ///< maximum conversion extent (12 bits)
#define VREFH 3300              ///< tens&atilde;o de refer&ecirc;ncia alta (mV)
#define VREFL 0.0               ///< tens&atilde;o de refer&ecirc;ncia baixa (mV)

uint8_t flag_ADC0_temp, flag_ADC0_pot;                ///< quantidade de elementos no buffer
unsigned short buffer_ADC[2];   ///< buffer para dois canais

/*!
 * Vari&aacute;vel de estado para coordenar os fluxos
 */
uint8_t flag_recepcao;

cor_pf cores[16] = {
    		{1.0, 0.0, 0.0},       ///< vermelho
    		{1.0, 0.42, 0.42},     ///< salmao 
    		{1.0, 1.0, 0.15},      ///< amarelo claro
    	    {0.0, 1.0, 0.0},       ///< verde
    	    {0.05, 0.9, 0.05},     ///< verde floresta 
    	    {0., 0.75, 0.75},      ///< azul berilo    
    	    {0.02, 0.3, 1.0},      ///< azul celeste 
    	    {0.0, 0.0, 1.0},       ///< azul           
    		{0.6, 0.1, 0.6},       ///< violeta        
    	    {0.65, 0.37, 0.27},    ///< marrom         
    	    {0.698, 0.35, 0.40},   ///< tijolo         
    		{1.0, 1.0, 1.0},        ///< branco
    		
    		{0.44, 0.86, 0.58},	    ///< Aquamarine
    		{0.48, 0.40, 0.93},	    ///< SlateBlue2
    		{0., 0., 0.5},			///< navy
    		{0., 0., 0.}			///< preto
};

/*!
 * Arranjos de caracteres de tamanho implicitamente definido pela inicializa&ccedil;&atilde;o .
 */
char vermelho[20] = "VERMELHO            ";
char branco[20] = "BRANCO         ";
char azul[20] = "AZUL              "; 
char preto[20] = "PRETO            ";
char verde[20] = "VERDE             ";
char magenta[20] = "MAGENTA          ";
char amarelo[20] = "AMARELO           ";
char ciano[20] = "CIANO            ";
char salmao[20] = "SALMAO           ";
char amareloclaro[20] ="AMARELO CLARO      ";
char verdefloresta[20] ="VERDE FLORESTA      ";
char azulberilo[20] = "AZUL BERILO       ";
char azulceleste[20]="AZULCELESTE       ";
char violeta[20]="VIOLETA             ";
char marrom[20]="MARROM            ";
char tijolo[20] = "TIJOLO           ";
char Aquamarine[20] = "AQUAMARINE        ";
char SlateBlue2[20] = "SLATEBLUE2        ";
char navy[20] ="NAVY                ";


/*!
 * Vetor de cores sinalizadoras dos intervalos do temporizador
 */
struct _cor cores_estaticas[] = {{ACESO,APAGADO,APAGADO},{APAGADO,ACESO,APAGADO},
    				{APAGADO,APAGADO,ACESO},{ACESO,APAGADO,ACESO},{ACESO,ACESO,APAGADO},{APAGADO,ACESO,ACESO},{ACESO,ACESO,ACESO}};

float intervalos[] = {1.5,3,4.5,6,7.5,9,10.5,12,13.5,15,16.5,18,19.5,21,22.5,24};

uint8_t indice5;  ///< intervalo do temporizador

uint8_t modo;  ///< modo = 0 (temporizador_parado); 1 (temporizador_configurando); 2 (cron&ocirc;metro_correndo); 
			   ///< 3 (cron&ocirc;metro_parado); 4 (temporizador_correndo)

uint32_t copia_tempo, tempo, tempo_relogio, ultimo_tempo, buzzer = 0, temporizador = 640; ///< vari&aacute;vel do contador de tempo 

uint8_t flag_muda_cor, flag_muda_tempo, flag_muda_estado, flag_muda_relogio; ///< vari&aacute;veis de estado



/*
 * \brief Mostra a cor dos leds no LCD
 * \param[in] ind &indice 
 */
void mostra_cor_LCD(uint8_t ind)
{
    /*! 
     * Mostre a cor no LCD conforme o c&oacute;digo de cor
     */
    switch (ind) {
    case 0:
        mandaString(vermelho);
        break;
    case 1:
        mandaString(salmao);
        break;
    case 2:
        mandaString(amareloclaro);
        break;
    case 3:
        mandaString(verde);
        break;
    case 4:
        mandaString(verdefloresta);
        break;
    case 5:
        mandaString(azulberilo);
        break;
    case 6:
        mandaString(azulceleste);
        break;
    case 7:
        mandaString(azul);
        break;
    case 8:
        mandaString(violeta);
        break;
    case 9:
        mandaString(marrom);
        break;
    case 10:
        mandaString(tijolo);
        break;
    case 11:
        mandaString(branco);
        break;
    case 12:
        mandaString(Aquamarine);
        break;
    case 13:
        mandaString(SlateBlue2);
        break;   
    case 14:
        mandaString(navy);
        break;
    case 15:
        mandaString(preto);
        break;

        
    }
}



#define TAM_MAX 1024


void mandaStringUART(char *s) {
	uint8_t x=0;					
	while(s[x]) {
		bufCircular_insere(&buffer_transmissor, s[x]);	/*!escreve o que leu*/		
		enableTEInterrup();  ///< habilita a interrup&ccedil;&atilde;odo transmissor
		x++;
	}
	bufCircular_insere(&buffer_transmissor, '\n');	
	bufCircular_insere(&buffer_transmissor, '\r'); /*! reposiciona a leitura do terminal*/
	enableTEInterrup();  ///< habilita a interrup&ccedil;&atilde;odo transmissor
}

/*!
 * \attention O evento de interrup&ccedil;&atilde;o do canal receptor &eacute; a presen&ccedil;a de 
 * um novo caractere no registrador UARTx_D (de leitura) enquanto o evento que causa uma interrup&ccedil;&atilde;o 
 * no canal de transmissor &eacute; o registrador UARTx_D (de escrita) ficar ocioso. Ap&oacute;s
 * Reset, UARTx_D fica sempre ocioso. 
 */
int main(void)
{
	
	float valor, m, temperatura;
	char s[10];
	ADCConfig config;

	uint8_t i, horas_entrada, minutos_entrada;
	uint8_t icon_sino[8] = {0x1f, 0x00, 0x0a, 0x00, 0x04, 0x10, 0x1e, 0x00} ;
	uint8_t icon_sino_1[8] = {0x00, 0x15, 0x15, 0x15, 0x0e, 0x04, 0x04, 0x04} ;
	

	uint8_t aux;
	uint8_t aux_horas;
	float segundos_entrada, aux_segundos;
	/*!
	 * Defini&ccedil;&atilde;o dos nomes das cores como um vetor de strings
	 */
	char relogio_entrada[15]="HH:MM:SS.S", *endereco;
	

	char mens0[]="Mostrar o horario no LCD";
	char mens1[]="O intervalo do temporizador setado no LCD ...";
	char mens2[]="O ultimo valor cronometrado no LCD ...";
	char mens3[]="configurando temporizador pelo potenciometro";
			
	char str[15];
	
	initLPTMR0(1, 0b0000, 0b01, 300); /*! porque 300?*/
	enableInterrupLPTMR0();
	config.diff = config.adlsmp = config.muxsel = 0;
	config.adtrg = 0;
	config.alternate = 0;
	config.pre = 0;
	config.fonte = 0b1110; 
	config.adco = 0; /*! pagina 473*/
	config.adlsts = 0b00; /*! p. 467*/
	config.mode = 0b01; 
	config.adiclk = 0b01;            
	config.adiv = 0b11; 			 
	config.avge = 1; 
	config.avgs = 0b01; 
	initADC0(0, &config);	
	enableInterrupADC0();
	initPort(B);
	setaMux(&PORTB_PCR1, 0);			///< fun&ccedil;&atildeo ADC0 - canal 9 
	enableNVICLPTMR0(3);
	enableNVICADC0(3);
	flag_ADC0_temp = 0;
	flag_ADC0_pot = 0;
	selecionaCanalADC0 (0x09);

	initLedRGBPWM(0b100, COUNTER_OVF, 0);

	initPort(A);
	setaMux(&PORT_PCR_REG(PORTA_BASE_PTR,5), 0b011);			 ///< TPM0_C2
	setaMux(&PORT_PCR_REG(PORTA_BASE_PTR,12), 0b011);		 ///< TPM1_C0
	setaMux(&PORT_PCR_REG(PORTA_BASE_PTR,4), 0b001);			 ///< GPIO
	enableIRQ(&PORT_PCR_REG(PORTA_BASE_PTR,4), 0b1011);                ///< Interrup&ccedil;&otilde;es em ambas as bordas



	initTPM0Cn(2,0b0011,0);            ///< Input Capture em ambas as bordas - PTA5

	initTPM (1, 0b100, COUNTER_OVF, 0);
	
	initTPM1Cn(0,0b0011,0);          ///< Input Capture em ambas as bordas
    initPort(E);
	
	enableNVICTPM0(3);
	enableTPM0CnInterrup(2);       ///< Habilitar a interrup&ccedil;&atilde;o no canal 2
	enableNVICTPM1(3);
	enableTPM1CnInterrup(0);          ///< Habilitar a interrup&ccedil;&atilde;o no canal 0
	enableNVICPTA(3);
	/*!
	 * Inicializa&ccedil;&otilde;es do ledRGB
	 */

	/*!
	 * Inicializa&ccedil;&otilde;es dos push buttons
	 */

	/*!
	 * Inicializa&ccedil;&otilde;es do LCD
	 */
	initLcdledGPIO();
	initLCD();

	/*!
	 * Cria um novo bitmap no endere&ccedil;o 
	 */

	criaBitmap(0x00,icon_sino);
	criaBitmap(0x01,icon_sino_1);

	/*!
	 * Habilita interrup&ccedil;&atilde;o dos pinos 4, 5 e 12 da PORTA
	 */
//	enablePushbuttonIRQA();

	/*!
	 * Inicializa&ccedil;&atilde;o o interruptor peri&oacute;dico PIT com contagem 
	 * Base de tempo: 20.971.520Hz
	 */
	initPIT(0b000, 2097152);    ///< interrup&ccedil;&atilde;o peri&oacute;dica: 0.1s

	/*! 
	 * Habilita as interrup&ccedil;&otilde;es no controlador NVIC com n&iacute;vel de prioridade  
	 */
	enableNVICPTA(3);
	enableNVICPIT(1);



	/*!
	 * Inicializa&ccedil;&atilde; das vari&aacute;veis
	 */
	indice5 = 0;  ///< cor e intervalo iniciais
	modo = 6; 	  ///< modo = temporizador
	tempo = 640;  ///< valor inicial do temporizador


	/*!
	 * Inicializa vari&aacute;veis de estado
	 */
	flag_muda_cor = flag_muda_tempo = flag_muda_estado = flag_muda_relogio = 0;

	/*!
	 * Inicializa o visor com o modo temporizador
	 */
	RS(0);
	enviaLCD(0b10000000, 4);       ///< reposicione o cursor
	mostra_cor_LCD(indice5); ///< escreva o nome da cor

	RS(0);
	enviaLCD(0b11000000, 4);     ///< posicione o cursor
	RS(1);
	enviaLCD(0x00,4);
	mandaString("00:00.0");

	/*! 
	 * Inicialize os pinos PTA1 e PTA2 como UART0
	 */
	initUART(9, 19200);   ///< Configure: taxa de amostragem 9x e baudrate = 19200
	enableRIEInterrup();


	enableNVICUART(1);

	/*!
	 * Inicializa&ccedil;&atilde;o das vari&aacute;veis
	 */

	bufCircular_cria (&buffer_receptor, TAM_MAX);
	bufCircular_cria (&buffer_transmissor, TAM_MAX);
	flag_recepcao = 0; ///< bandeira de recep&ccedil;&atilde;o baixada 
	
	enablePITInterrup();
	for(;;) {
		
		if ((modo != 3) && (modo != 1)) { /*! escrever a temperatura */
			if (flag_ADC0_temp) {
				valor = (VREFH - VREFL)*buffer_ADC[1]*1.0/MAX;      ///< valor em tens&atile;o
				if(valor >= 703.125) ///< valor em mV com somente uma casa decimal
					m = 1.646;
				else
					m = 1.769;
											
				temperatura = 25 - (valor - 703.125)/(1000.0*m);                ///< valor em graus Celsius
				ConvF2String (temperatura, s);   ///< converter o valor para string
			
				RS(0);
				enviaLCD(0b10001100, 4);     ///< posicione o cursor
				RS(1);
				mandaString(s);
			
				flag_ADC0_temp = 0; ///< baixar a bandeira
			}
		}

		if (flag_muda_estado) {
			if (modo == 4 || modo == 2 || modo == 7 || modo == 8) {
				/*!
				 * Limpa a segunda linha do visor
				 */
				RS(0);
				enviaLCD(0b11000000, 4);     ///< posicione o cursor
				RS(1);
				enviaLCD(0x00,4);
				mandaString("               ");				
			}
			else if (modo == 6) { /*!limpa a primeira linha*/
				RS(0);
				enviaLCD(0b10000000, 4);     ///< posicione o cursor
				RS(1);
				mandaString("               ");
			}
			flag_muda_estado = 0;
		}
		if (flag_muda_tempo) {
			switch (modo) {
			case 4: ///< modo = temporizador_operando
				flag_muda_tempo = 0;
				float tmp = (copia_tempo*16*COUNTER_OVF)/20971520.; ///< Pr&eacute;-escale = 16
				ConvMMSS2String(0,tmp, str);
				RS(0);
				enviaLCD(0b11000000, 4);    ///< posicione o cursor
				RS(1);
				enviaLCD(0x00,4);
				mandaString(str);
				mandaString("         ");
				if (copia_tempo == 0){ 
					modo = 6;   ///< ao atingir 0, passa para o estado temporizador_parado
					
					buzzer = 6400;
					setaMux(&PORT_PCR_REG(PORTE_BASE_PTR,21), 0b011);		///< TPM1_C1
					enableTPM1CnInterrup(1);
					
					flag_muda_estado = 1;
				}
				break;
			case 2: ///< modo = cron&ocirc;metro_correndo
				/*!
				 * Inicializa o visor do LCD para modo cronometro
				 */

				aux = tempo/76800;
				ConvMMSS2String(aux, (float)(tempo-76800*aux)*0.00078124992, str);
				RS(0);
				enviaLCD(0b11000000, 4);    ///< posicione o cursor
				RS(1);
				enviaLCD(0x01,4);
				mandaString(str);

				break;
			case 3: ///< modo = cron&ocirc;metro_parado

				/*!
				 * Inicializa o visor do LCD para modo cronometro
				 */
				RS(0);
				enviaLCD(0b11000000, 4);    ///< posicione o cursor
				RS(1);
				enviaLCD(0x01,4);
				mandaString("Cron. Desligado   ");
				
				modo = 6;
				flag_muda_estado = 1;
				
				break;
			case 6: /*! modo de consulta do relogio*/
				aux_horas = tempo_relogio/36000;
				aux = (tempo_relogio - aux_horas*36000)/600;
				aux_segundos = tempo_relogio-(aux_horas*36000)-(600*aux);
				aux_segundos = (float) (aux_segundos/10.0);
				endereco = &str[0];
				ConvHHMMSS2String(aux_horas, aux, aux_segundos, endereco);


				RS(0);
				enviaLCD(0b10000000, 4);    ///< posicione o cursor
				RS(1);
				enviaLCD(0x01,4);
				mandaString(str);
				break;

			case 7:
				RS(0);
				enviaLCD(0b11000000, 4);    ///< posicione o cursor
				RS(1);
				enviaLCD(0x00,4);
				ConvMMSS2String(0, intervalos[indice5], str);
				mandaString(str);
				modo = 3; /*! volta ao menu */
				break;
			case 8:
				aux = ultimo_tempo/76800;
				ConvMMSS2String(aux, (float)(ultimo_tempo-76800*aux)*0.00078124992, str);

				RS(0);
				enviaLCD(0b11000000, 4);    ///< posicione o cursor
				RS(1);
				enviaLCD(0x00,4);
				mandaString(str);
				modo = 3; /*! volta ao menu */
			}
			flag_muda_tempo = 0;
		}
		if (flag_muda_cor) {			
			/*! Mostra a cor sinalizadora selecionada no LCD */
			RS(0);
			enviaLCD(0b10000000, 4);    ///< reposicione o cursor
			RS(1);
			mostra_cor_LCD(indice5); 	///< escreva o nome da cor		
			flag_muda_cor = 0;			///< abaixar a bandeira
		}

		/*!
		 * Como o processamento deve ser por unidade de informa&ccedil;&atilde;o 
		 * ter algum elemento no buffer de recep&ccedil;&atilde;o n&atilde;o &eacute;
		 * suficiente. &Eacute; necess&aacute;rio aguardar que o usu&aacute;rio 
		 * d&ecirc; <ENTER>
		 */

		if (flag_recepcao && bufCircular_vazia (&buffer_transmissor) && (modo == 3 || modo == 6 || modo == 5)) {
			/*!
			 * Processar somente quando o buffer transmissor estiver vazio para ecoar os caracteres
			 * capturados
			 */
			/*! Extrair a unidade de informa&ccedil;&atilde;o at&eacute; '\r' */
			char buffer[10];
			i = 0;

			while ((buffer[i]=bufCircular_remove(&buffer_receptor)) != '\r') { /*!vai lendo as letras que voce escreve*/ 
				i++;
			}


			/*!
			 * Substituir '\r' pelo terminador usual de uma string '\0'
			 */
			buffer[i] = '\0';

			/*!
			 * Neste caso, como sabemos que &eacute; s&oacute; um caractere.
			 * podemos s&oacute; checar o primeiro caractere da string buffer
			 */

			if (modo == 5) {
				ConvString2HHMMSS (buffer, &horas_entrada, &minutos_entrada, &segundos_entrada);
				disablePITInterrup();
				
				tempo_relogio =  (uint32_t)(horas_entrada)*36000 + (uint32_t)(minutos_entrada)*600 + (uint32_t)(segundos_entrada*10); /*!atualiza o relogio*/
				
				///e habilitar 
				enablePITInterrup();
				
				modo = 6;
				
			} else {
				switch (buffer[0])
				{
				case '1':
						if(modo == 3 || modo == 0 || modo == 6) {  /*! le relogio */
						bufCircular_insere(&buffer_transmissor, '\n');			
						enableTEInterrup();  ///< habilita a interrup&ccedil;&atilde;odo transmissor
						mandaStringUART(relogio_entrada);
				
						modo = 5;
					}
					break;
				case '2':
						if(modo == 3 || modo == 0 || modo == 6) {
						modo = 6;
						
						mandaStringUART(mens0);

						flag_muda_tempo = 1;
						flag_muda_estado = 1;
					}
					break;
				case '3':
						if(modo == 3 || modo == 0 || modo == 6) {
						modo = 7;
						
						mandaStringUART(mens1);

						flag_muda_estado = 1;
						flag_muda_tempo = 1;
					}
					break;
				case '4':
						if(modo == 3 || modo == 0 || modo == 6) {
						modo = 8;

						mandaStringUART(mens2);

						flag_muda_estado = 1;
						flag_muda_tempo = 1;
					}
					break;
				case '5': //configura cor pelo potenciometro
					if((modo != 1) && flag_ADC0_pot) {
						/*!
						 * Dados dispon&iacute;veis nos canais 9 e 26
						 * Como o intervalo de amostragem &eacute; grande, n&atilde;o 
						 * s&atilde;o feitas as c&oacute;pias.
						 */

						/*!
						 * Potenci&ocirc;metro
						 * Convers&atilde;o para tens&atilde;o: buffer[0]
						 * (Se&ccedil;&atilde;o 28.6.1.3 em KL-25 Sub-Family Reference Manual)
						 */
						flag_ADC0_pot = 0;
						mandaStringUART(mens3);
						valor = (VREFH - VREFL)*buffer_ADC[0]*1.0/MAX;  ///<  valor em tens&atilde;o
						indice5 = (int)valor/206.25; /*! divide o intervalo de 0-3300 em 16 partes */
						indice5 = indice5 % 16; /*! caso leia um valor um pouco maior que 330 volta ao vermelho */
						
							
						ConvF2String (valor, s);   ///< converter o valor para string
						mandaStringUART("tensao em mV:");
						mandaStringUART(s);
						flag_muda_cor = 1;
						temporizador = 640*(indice5+1);
						modo = 3;
						//temporizador; ultimo_temporizador

					
					}
					break;
				case 'M':
				case 'm': /*! voltar ao menu se estiver consultando o relogio */
					if(modo == 6) {
						modo = 3;
						flag_muda_estado = 0;
						flag_muda_tempo = 0; /*!tira lixo que possa ter no PIT*/
						RS(0);
						enviaLCD(0b10000000, 4);    ///< reposicione o cursor
						RS(1);
						mostra_cor_LCD(indice5); 	///< escreva o nome da cor
					}
					break;
				}	
			}
			flag_recepcao = 0;  ///< baixar a bandeira
		}

	}
}


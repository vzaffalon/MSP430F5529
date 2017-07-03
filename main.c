#include <msp430.h>
#include <stdio.h>

//DEFINICOES DE PINOS DO LCD
#define LCD_E_OUT P3OUT
#define LCD_E BIT5
#define LCD_NIB 0XF
#define LCD_NIB_OUT P6OUT
#define LCD_RS BIT6
#define LCD_RS_OUT P3OUT
#define PINO  BIT5
#define PINO_OUT P2OUT

//definicao LEDS
#define LED1 (0x0001)
#define LED2 (0x0080)

//DEFINICOES CHAVES
#define SW1 BIT1        //CHAVE SW1
#define SW1_IN  P2IN
#define SW2 BIT1        //CHAVE SW2
#define SW2_IN P1IN

//DEFINICOES LOOP PRINCIPAL
#define VERDADEIRO 1
#define FALSO 0
#define PINO  BIT5
#define PINO_OUT  P2OUT
#define MAXIMOVALOR 63429U  //CCR0 BASE PARA 0,1ms
#define REBOTE 2000    //delay PARA EVITAR BOUNCE
#define ABERTA 1
#define FECHADA 0

//DEFINE TEMPOS DE DELAY PARA LCDD
#define ATZ_100us 3   //104 Usegundos
#define ATZ_500us (9*ATZ_100us) //513 Usegundos
#define ATZ_1ms (2.1*ATZ_500us) //1,08 Msegundos
#define ATZ_5ms (5.2*ATZ_1ms) //5,07 Msegundos
#define ATZ_15ms (3.1*ATZ_5ms) //15,6 Msegundos

//VARIAVEIS DO TEMPO
int decimossegundos; //CONTADOR PARA DECIMO DE SGUNDOS
int segundos;  //CONTADOR PARA segundosUNDOS
int minutos;  //CONTADOR DE minutosUTOS
int horas; //CONTADOR DAS horasS

//CARACTERES DAS LINHAS
char primeiraLinha[20]; //VETOR DE CARACTERES LINHA 1
char segundaLinha[20]; //VETOR DE CARACTERES LINHA 2

//MODIFICADORES DE ESTADO DO PROGRAMA
int running;    //MODIFICA ESTADO PARA running/STOP
int flag_linha_1;  //FLAG PARA IMPRESSAO NA LINHA 1
int flag_linha_2; //FLAG PARA IMPRESSAO NA LINHA 2

//ESTADO DAS CHAVES
int estado_chave_1=ABERTA; //estado anterior do SW1
int estado_chave_2=ABERTA; //estado anterior de SW2

//FUNCOES DO PROGRAMA PRINCIPAL
void timer_a_configuracao(void);
void portas_configuracao(void);

//QUESTIONS
char *line1_questions[5]= {"Qual seu nome?","Qual a blablu?", "Qual a blabla?","Qual a hue hue?","O seu boi?"};
char *line2_questions[5]= {"                ","                ", "                ","                ","                "};
char *line1_options[5]= {"A:JOSE B:PEDRO","A:HAAGA B:PUBUR","A:HAAA B:PUUR","A:HABUA B:PUBUSR","A:HAAGA B:PUBUR"};
char *line2_options[5]= {"C:JESUS D:JUE","C:JESS D:JUEa", "C:JEUS D:JUE","C:JoUS D:JUEE","C:JEASUS D:JAUE"};
int question_answer[5]={0,2,3,1,2};
unsigned int actualQuestion = 0;
int questionMode = 0; //0 for question 1 for options
int numberOfPoints = 0;
unsigned int letterState = 0; //LETTER STATE 0 IS A 1 IS B 2 IS C 3 IS D
unsigned int selectedLetter = 0;

//FUNCOES DO LCD
void lcd_escreve_string(char *pt);
void lcd_muda_cursor(char pos);
void lcd_ativa_cursor(int estado,int blink);
void lcd_caracter(char dado);
void lcd_limpa(void);
void lcd_home(void);
void lcd_spc(void);
void lcd_recebe_comando(char comando);
void lcd_start(void);
void lcd_escreve_byte(char baite);
void lcd_escreve_nib(char nib);
void lcd_config_pinos(void);
void delay(unsigned long timeValue);
void config_leds(void);

//CONFIGURACAO DO CLOCK
//SMCLK = 1.048.576
//SMCLK/2 = 52.4288
//0,1S = 52.429
//ARREDODAN PARA 21.000
void timer_a_configuracao(void){
  TA0CTL = TBSSEL_1 |  MC_1  |  ID_2;       //FONT SMCLK MODO COUT UP DIVISOR POR 2
  TA0CCR0 = MAXIMOVALOR;        //DEFINE VALOR DE CCR0 PARA PERIODO 0,1ms
  TA0CCTL0 = CCIE;     //HABILITA INTERRUPCAO QUANDO ATINGE CCR0
}

// CRONOMETRO INFORMACOES
//SW1 TROCA MODO RUNNING/STOPED
//SW2 CONFIGURA LAP/RESET
//PRECISAO DEFINIDA PARA 0,1 segundos
int main(void){
    WDTCTL = WDTPW | WDTHOLD;   //PARA O WATCHDOG TIMER

    //COMECA EM STOP O CRONOMETRO
    running = FALSO;
    flag_linha_1 = VERDADEIRO;
    flag_linha_2 = VERDADEIRO;

    //CONFIGURACAO LCD
    lcd_config_pinos();
    lcd_start();
    lcd_ativa_cursor(FALSO,FALSO);

    //TIMER E PORTAS CONFIGURACAO
    portas_configuracao();
    timer_a_configuracao();
    config_leds();
    _enable_interrupt();

    while(1){
      //VERIFICA LINHA 1 LIBERADA
      if (flag_linha_1){
        //flag_linha_1 = FALSO;
        //LINHA 1 TRAVADA
        lcd_muda_cursor(0);

        //PRINTA NO FORMATO CORRETO NO LCD
        switch(questionMode){
            case(0):
                     sprintf(primeiraLinha, "%s",line1_questions[actualQuestion]);
                 break;
            case(1):
                     sprintf(primeiraLinha, "%s",line1_options[actualQuestion]);
                 break;
            case(2):
                     sprintf(primeiraLinha, "Sua pontuacao:");
                break;
            case(3):
                    if(selectedLetter == question_answer[actualQuestion]){
                       if(selectedLetter == 0){
                           sprintf(primeiraLinha, "Acertou letra A");
                       }else{
                           if(selectedLetter == 1){
                               sprintf(primeiraLinha, "Acertou letra B");
                       }else{
                           if(selectedLetter == 2){
                               sprintf(primeiraLinha, "Acertou letra C");
                           }else{
                               if(selectedLetter == 3){
                                   sprintf(primeiraLinha, "Acertou letra D");
                               }
                           }
                       }
                       }
                    }else{

                        if(question_answer[actualQuestion] == 0){
                                          sprintf(primeiraLinha, "Errou letra A");
                                      }else{
                                          if(question_answer[actualQuestion] == 1){
                                              sprintf(primeiraLinha, "Errou letra B");
                                      }else{
                                          if(question_answer[actualQuestion] == 2){
                                              sprintf(primeiraLinha, "Errou letra C");
                                          }else{
                                              if(question_answer[actualQuestion] == 3){
                                                  sprintf(primeiraLinha, "Errou letra D");
                                              }
                                          }
                                      }
                                      }
                    }

                    break;
        }

        //CURSOR PARA INICIO
        //ESCREVE NO LCD LINHA 1
        lcd_escreve_string(primeiraLinha);
        lcd_escreve_string("   ");
      }
      //VERIFICA LINHA 2 LIBERADO
      if(flag_linha_2){
        //flag_linha_2 = FALSO;
        //LINHA 2 TRAVADA
        lcd_muda_cursor(0x40);


        switch(questionMode){
                  case(0):
                        sprintf(segundaLinha, "%s",line2_questions[actualQuestion]);
                   break;
                  case(1):
                        sprintf(segundaLinha, "%s",line2_options[actualQuestion]);
                   break;
                  case(2):
                        sprintf(segundaLinha, "%d pontos      ",numberOfPoints);
                          break;
                  case(3):
         if(question_answer[actualQuestion] == 0){
                                                  sprintf(segundaLinha, "%s",line1_options[actualQuestion]);
                                              }else{
                                                  if(question_answer[actualQuestion] == 1){
                                                      sprintf(segundaLinha, "%s",line1_options[actualQuestion]);
                                              }else{
                                                  if(question_answer[actualQuestion] == 2){
                                                      sprintf(segundaLinha, "%s",line2_options[actualQuestion]);
                                                  }else{
                                                      if(question_answer[actualQuestion] == 3){
                                                          sprintf(segundaLinha, "%s",line2_options[actualQuestion]);
                                                      }
                                                  }
                                              }
                                              }

                  break;
              }


        //CURSOR PARA LINHA 2
        //ESCREVE NO LCD LINHA 2
        lcd_escreve_string(segundaLinha);
        lcd_escreve_string("   ");
      }


      //VERIFICA CLIQUE NA CHAVE 2
      //RESPONSAVEL PELO LAP/RESET
      if ((SW2_IN & SW2) == 0){
        if (estado_chave_2 == ABERTA){
          //SALVA NOVO ESTADO DA CHAVE
          estado_chave_2 = FECHADA;

          switch(letterState){
              case(0):
                  letterState = 1;
                  P1OUT &= (~LED1);
                  P4OUT |= (LED2);
                  break;
              case(1):
                  letterState = 2;
                  P1OUT |= (LED1);
                  P4OUT &= (~LED2);
                  break;
              case(2):
                  letterState = 3;
                  P1OUT |= (LED1);
                  P4OUT |= (LED2);
                  break;
              case(3):
                  letterState = 0;
                  P1OUT &= (~LED1);
                  P4OUT &= (~LED2);
                  break;
          }


        }
      }
      else{
        //SALVA ESTADO DA CHAVE COMO ABERTO
        estado_chave_2 = ABERTA;
      }

      //VERIFICA CLIQUE NA CHAVE 1
      //CHAVE 1 TROCA ESTADO DE RUNNING PRA STOP
      if((SW1_IN & SW1) == 0){
        if (estado_chave_1 == ABERTA){
          //VERIFICA ESTADO ANTERIOR DA CHAVE
            estado_chave_1 = FECHADA;
            selectedLetter = letterState;
            P1OUT &= (~LED1);
            P4OUT &= (~LED2);

            delay(5000);
            P1OUT |= (LED1);
            P4OUT |= (LED2);
            delay(5000);
            P1OUT &= (~LED1);
            P4OUT &= (~LED2);
            delay(5000);
            P1OUT |= (LED1);
            P4OUT |= (LED2);
            delay(5000);
            P1OUT &= (~LED1);
            P4OUT &= (~LED2);

            questionMode = 3;

        }
      }
      else{
        //SALVA ESTADO DA CHAVE COMO ABERTO
        estado_chave_1 = ABERTA;
      }
    }
    return 0;
}

void portas_configuracao(void){
  P2DIR |=PINO;           //CONFIGURA PINO
  PINO_OUT &=  ~PINO;     //SE DER ERRO TIRA ESSA LINHA
  P2DIR &=  ~SW1;
  P2REN |= SW1;         //CONFIGURA CHAVE 1
  P2OUT |= SW1;
  P1DIR &=  ~SW2;       //CONFIGURA CHAVE 2
  P1REN |= SW2;
  P1OUT |= SW2;
}

//IMPRIME STRING ATE ACHAR O CONTRA BARRA ZERO
void lcd_escreve_string(char *string){
  while(*string != '\0'){
    lcd_caracter(*string);
    string++;
  }
}

//MUDA POSICAO DO CURSOR PRA POSICAO ESCOLHIDA
void lcd_muda_cursor(char posic){
  lcd_recebe_comando(posic | 0x80);
}

//IMPRIMIR 1 ESPACO NO LCD
void lcd_spc(void){
  lcd_caracter(0x20);
}

//CLEAR NO LCD
void lcd_limpa(void){
  lcd_recebe_comando(1);
}

//LIGA OU DESLIGA CURSOR
void lcd_ativa_cursor(int estado,int blink){
  char aux = 0;
  if (estado == VERDADEIRO){
    aux = 0xE;
  }else{
    aux = 0xC;
  }
  if (blink == VERDADEIRO){
    aux |= 1;
  }
    lcd_recebe_comando(aux);
}

//ESCREVER UM COMANDO NO LCD
void lcd_recebe_comando(char comando){


  LCD_RS_OUT = LCD_RS_OUT & (~LCD_RS);  //RS RECEBE 0 MODO COMANDO
  lcd_escreve_byte(comando);         //ESCREVE BYTE COMANDO NO BUS DO LC


  if ((comando == 2) || (comando == 1)){ //VERIFICA QUAL O COMANDO E CHAMA DELAY APROPRIADO

    delay(2*ATZ_1ms);

  }else{

    delay(ATZ_500us);

  }
}


//INICIALIZANDO O LCD
void lcd_start(void){
  delay(ATZ_15ms);

  LCD_RS_OUT &= (~LCD_RS);      //LCD MODO COMANDO
  lcd_escreve_nib(3);
  delay(ATZ_5ms);     //REPETE COMANDO 3 1X

  lcd_escreve_nib(3);   //REPETE COMANDO 3 2X
  delay(ATZ_100us);

  lcd_escreve_nib(3);   //REPETE COMANDO 3 3X
  delay(ATZ_100us);

  lcd_escreve_nib(3);   //REPETE COMANDO 3 4X
  delay(ATZ_100us);

  lcd_recebe_comando(0x28); //modo 4 BITS 2 LINHAS 5X8
  lcd_recebe_comando(0x8);  //DISPLAY OFF
  lcd_recebe_comando(0x1);  //CLEAR
  lcd_recebe_comando(0x6);  //AC++
  lcd_recebe_comando(0xf);  //DISPLAU ON CURSOR blinkNDO

  lcd_limpa();    //CONFIRMAR APAGAMENTO LCD
}

//Escreve um byte no bus do lcd
void lcd_escreve_byte(char baite){
    lcd_escreve_nib((baite>>4) & 0xf);    //SHIFTA CHAR ESCREVE PRIMEIROS BITS DO BYTE
    lcd_escreve_nib( baite & 0xf);        //ESCREVE PROXIMOS BITS
}

//HOME NO LCD
void lcd_home(void){
  lcd_recebe_comando(2);
}

void config_leds(void){
    //config leds
        P1DIR |= LED1;
        P1OUT &= (~LED1);

        P4DIR |= LED2;
        P4OUT &= (~LED2);
}

void lcd_config_pinos(void){

    P3DIR = P3DIR | LCD_RS;
    LCD_RS_OUT = LCD_RS_OUT & (~LCD_RS);
                                                  //PULSA O E DO LCD
    P3DIR = P3DIR |LCD_E;
    LCD_E_OUT = LCD_E_OUT & (~LCD_E);

    P6DIR = P6DIR | LCD_NIB;
    LCD_NIB_OUT = LCD_NIB_OUT & (~LCD_NIB); //BUS DE 4 BITS INICIA RECEBE 0

    P2DIR = P2DIR | PINO;               //PINO P2 PARA SAIDA
    PINO_OUT = PINO_OUT & (~PINO);
}

//ESCREVER UM DADO NO LCD
void lcd_caracter(char dado){
  LCD_RS_OUT = LCD_RS_OUT | LCD_RS;   //RS = 1
  lcd_escreve_byte(dado);
  LCD_RS_OUT = LCD_RS_OUT & (~LCD_RS);  //RS = 0
  delay(ATZ_1ms);
}

//ESCREVE NIB NO BUS DO LCD
void lcd_escreve_nib(char nib){
  LCD_NIB_OUT = LCD_NIB_OUT & (~LCD_NIB); //ZERA OS 4 NIBBLES
  LCD_NIB_OUT = LCD_NIB_OUT | (nib & 0xf);  //ESCREVE OS 4 NIBBLES NO BUS
  LCD_E_OUT = LCD_E_OUT | LCD_E;
  LCD_E_OUT = LCD_E_OUT & (~LCD_E);
}


//FUNCAO DE GERAR DELAY
void delay(unsigned long timeValue){
  volatile unsigned long cont=timeValue;
  while(cont != 0){
    cont--;
  }
}

//VETOR DE INTERRUPCAO ALTERANDO VALORES DO CRONOMETRO
#pragma vector = TIMER0_A0_VECTOR
__interrupt void timera0_inte(void){
    lcd_limpa();
    if(questionMode == 0){
               questionMode = 1;
           }else{
               if(questionMode == 1 || questionMode == 3){
                   if(selectedLetter == question_answer[actualQuestion]){
                                              numberOfPoints = numberOfPoints + 100;
                                          }
                                          actualQuestion++;
                                          if(actualQuestion < 5){
                                              questionMode = 0;
                                          }else{
                                              questionMode = 2;
                                          }
               }else{

               }
           }
}
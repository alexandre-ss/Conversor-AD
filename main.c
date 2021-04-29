// Alexandre Santana Sousa
// configuração: P6.0 - A0 eixo x
//               P6.2 - A2 eixo y
//               P6.4 - SW
#include <msp430.h>

void ADC_config(void);
void config_I2C(void);
void LCD_inic(void);
void LCD_aux(char dado);
void PCF_STT_STP(void);
int PCF_read(void);
void PCF_write(char dado);
void delay(long limite);

//funções de controle
void lcd_write_string(char *s, char linha);
void lcd_write( unsigned char valor);
void lcd_write_decimal8b(int valor);
void controla_lcd(char valor);
void posiciona_cursor(char posicao);
void char_to_hex8b(char valor);
void char_to_hex16b(int valor);
void write_float(float valor);
void config_timer();
void config_chav();
int  verifica_sw();
void limpa_linha(int linha);
void atualiza_lcd();
void modo_1();
void modo_2();
void modo_3();
void modo_4();

#define BR100K      10
#define RS          BIT0
#define RW          BIT1
#define BIT_E       BIT2    //Enable
#define BIT_BL      BIT3    //Back light
#define first_line  0x80
#define second_line 0xC0
#define all_display 1
#define PCF_ADR     0x27
#define TRUE        1
#define FALSE       0
#define open        1
#define closed      0


//variáveis globais
volatile float voltX = 0, voltY = 0;
volatile int  valorsw = 0;
volatile unsigned int media_y = 0;
volatile unsigned int media_x = 0;
volatile unsigned int flag = 0, novo = 0, velho = 8;

void main(void)
 {

    WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
    config_I2C();
    LCD_inic();
    controla_lcd(BIT3 | BIT2 ); //Cursor apagado

    ADC_config();
    config_timer();
    config_chav();
    __enable_interrupt();

    while(1){
        switch(valorsw){
            case 0:
                    limpa_linha(all_display);
                    lcd_write_string("Modo 1", first_line);
                    delay(50000);
                    limpa_linha(all_display);

                    lcd_write_string("A0: ", first_line);
                    lcd_write_string("A2: ", second_line);
                    posiciona_cursor(first_line + 9);
                    lcd_write('V');
                    posiciona_cursor(second_line + 9);
                    lcd_write('V');

                    while(!verifica_sw()){
                        if(flag == 1){
                            flag = 0;
                            modo_1();
                        }
                    }
                    break;
            case 1:
                    limpa_linha(all_display);
                    lcd_write_string("Modo 2", first_line);
                    delay(50000);
                    limpa_linha(all_display);

                    lcd_write_string("A0: ", first_line);
                    posiciona_cursor(first_line + 9);
                    lcd_write('V');

                    while(!verifica_sw()){
                        if(flag == 1){
                            flag = 0;
                            modo_2();
                        }
                    }
                    break;
            case 2:
                    limpa_linha(all_display);
                    lcd_write_string("Modo 3", first_line);
                    delay(50000);
                    limpa_linha(all_display);

                    lcd_write_string("A2: ", first_line);
                    posiciona_cursor(first_line + 9);
                    lcd_write('V');

                    while(!verifica_sw()){
                        if(flag == 1){
                            flag = 0;
                            modo_3();
                        }
                    }
                    break;
            case 3:
                    limpa_linha(all_display);
                    lcd_write_string("Modo 4", first_line);
                    delay(50000);
                    limpa_linha(all_display);
                    while(!verifica_sw()){
                        if(flag == 1){
                            flag = 0;
                            modo_4();
                        }
                    }
                    break;
            default:
                    valorsw = 0;
                    break;
        }
    }

}

void modo_1(){
        posiciona_cursor(first_line + 4);
        write_float(voltX);
        posiciona_cursor(second_line + 4);
        write_float(voltY);
        posiciona_cursor(first_line + 12);
        char_to_hex16b(media_x);
        posiciona_cursor(second_line + 12);
        char_to_hex16b(media_y);
        atualiza_lcd();
}

void modo_2(){
        volatile int i;
        novo = (media_x >> 8);

        posiciona_cursor(first_line + 4);
        write_float(voltX);
        posiciona_cursor(first_line + 12);
        char_to_hex16b(media_x);

        if(novo < velho){
            for(i = velho; i > novo; i--){
                posiciona_cursor(second_line + i);
                lcd_write(' ');
            }
        }
        else if(novo > velho){
            for(i = velho; i < novo; i++){
                posiciona_cursor(second_line + i);
                lcd_write(0xFF);
            }
        }else{
            for(i = 0; i < novo; i++){
                posiciona_cursor(second_line + i);
                lcd_write(0xFF);
            }
        }
        velho = novo;
        atualiza_lcd();

}
void modo_3(){
        volatile int i;
        novo = (media_y >> 8);

        posiciona_cursor(first_line + 4);
        write_float(voltY);
        posiciona_cursor(first_line + 12);
        char_to_hex16b(media_y);

        if(novo < velho){
            for(i = velho; i > novo; i--){
                posiciona_cursor(second_line + i);
                lcd_write(' ');
            }
        }
        else if(novo > velho){
            for(i = velho; i < novo; i++){
                posiciona_cursor(second_line + i);
                lcd_write(0xFF);
            }
        }else{
            for(i = 0; i < novo; i++){
                posiciona_cursor(second_line + i);
                lcd_write(0xFF);
            }
        }
        velho = novo;
        atualiza_lcd();

}

void modo_4(){
    volatile int meiox = 2048, meioy = 2048, i = 0, tamanho = 0;

        posiciona_cursor(first_line + 7);
        lcd_write(0x7F);
        lcd_write(0xFF);

        posiciona_cursor(second_line + 7);
        lcd_write(0xFF);
        lcd_write(0x7E);

        if(media_x < meiox){
             tamanho = (media_x >> 8);
             posiciona_cursor(first_line + 7);
             lcd_write(0xFF);
             lcd_write(0x7E);

             for(i = tamanho; i > 0; i--){
                 posiciona_cursor(first_line + 7 - i);
                 lcd_write(0xFF);
             }
        }
        else if(media_x > meiox){
               tamanho = ((media_x >> 8) - 8);

               for(i = 0; i < tamanho; i++){
                   posiciona_cursor(first_line + 9 +i);
                   lcd_write(0xFF);
               }
        }

        if( media_y > 2070 && media_y <= 2100){
            // posiciona no meio
            posiciona_cursor(second_line + 7);
            lcd_write(0xFF);
            lcd_write(0x7E);
        }
        else if(media_y < meioy){
                 tamanho = (media_y >> 8);
                 posiciona_cursor(second_line + 7);

                 for(i = tamanho; i > 0; i--){
                     posiciona_cursor(second_line + 7 - i);
                     lcd_write(0xFF);
                 }
        }
        else if(media_y > meioy){
               tamanho = ((media_y >> 8) - 8);
               posiciona_cursor(second_line + 7);
               lcd_write(0x7F);
               lcd_write(0xFF);

               for(i = 0; i < tamanho; i++){
                   posiciona_cursor(second_line + 9 +i);
                   lcd_write(0xFF);
               }
        }
        atualiza_lcd();
        limpa_linha(all_display);
}

#pragma vector = ADC12_VECTOR
__interrupt void adc_int(void){

    volatile unsigned int *pt;
    unsigned int i,soma;
    pt = &ADC12MEM0;
    soma = 0;

    for (i=0; i<8; i++){
     soma += pt[i];
    }
    media_x = soma/8;

    soma = 0;
    for (i=8; i<16; i++){
     soma += pt[i];
    }
    media_y = soma/8;

    voltX = (media_x*3.3)/4095;
    voltY = (media_y*3.3)/4095;
    flag = 1;
}



void lcd_write_string(char *s, char linha) {
    switch(linha){
        case first_line:
            posiciona_cursor(first_line);
            while (*s != '\0'){
                lcd_write(*s++);
            }
            break;
        case second_line:
            posiciona_cursor(second_line);
            while (*s != '\0'){
                lcd_write(*s++);
            }
            break;
    }
}

void lcd_write(unsigned char valor){
    volatile char escrever;
    escrever = (valor&0XF0) | BIT_BL | RS | BIT_E;  //E=1
    PCF_write(escrever);
    escrever = ((valor&0XF0) | BIT_BL | RS) & ~BIT_E;  //E=0
    PCF_write(escrever);

    escrever = ((valor<<4)&0XF0) | BIT_BL | RS | BIT_E;  //E=1
    PCF_write(escrever);
    escrever = (((valor<<4)&0XF0) | BIT_BL | RS) & ~BIT_E;  //E=0
    PCF_write(escrever);
}


void controla_lcd(char valor){
    char escrever;
    // para controlar o lcd preciso do RS = 0
    escrever = ((valor)&0XF0) | BIT_BL | BIT_E;  //E=1
    PCF_write(escrever);
    escrever = (((valor)&0XF0) | BIT_BL ) & ~BIT_E;  //E=0
    PCF_write(escrever);

    escrever = ((valor<<4)&0XF0) | BIT_BL | BIT_E;  //E=1
    PCF_write(escrever);
    escrever = (((valor<<4)&0XF0) | BIT_BL ) & ~BIT_E;  //E=0
    PCF_write(escrever);
}

void posiciona_cursor(char posicao){
    controla_lcd(posicao);
}

void lcd_write_decimal8b(int valor){
    volatile int cem = 0, dezena = 0, unidade = 0;
    if((valor/100) >= 1){
        cem = valor/100;
        dezena = (valor%100)/10;
        unidade = (valor%100)%10;
        cem += 0x30;
        dezena += 0x30;
        unidade += 0x30;

        lcd_write(cem);
        lcd_write(dezena);
        lcd_write(unidade);
    }
    else if((valor/10) >= 1){
        dezena = valor/10;
        unidade = valor%10;

        dezena += 0x30;
        unidade += 0x30;

        lcd_write(dezena);
        lcd_write(unidade);
    }
    else{
        unidade = valor + 0x30;
        lcd_write(unidade);
    }
}

void char_to_hex8b(char valor){
    if(((valor>>4)&0x0F) >= 10){
        lcd_write(((valor>>4)&0x0F) + 0x37);
    }
    else{
        lcd_write(((valor>>4)&0x0F) + 0x30);
    }

    if((valor&0x0F) >= 10){
        lcd_write((valor&0x0F) + 0x37);
    }
    else{
        lcd_write((valor&0x0F) + 0x30);
    }
}

void char_to_hex16b(int valor){
    char_to_hex8b(valor >> 8);
    char_to_hex8b(valor&0xFF);
}

void write_float(float valor){
    int inteiro = (int)valor;
    int fracao = (valor - inteiro)*1000;
    lcd_write(inteiro + 0x30);
    lcd_write(',');
    lcd_write_decimal8b(fracao);

}

void limpa_linha(int linha){
    volatile int i;
    if(linha == first_line){
        posiciona_cursor(first_line);
        for(i = 0; i < 16; i++){
            lcd_write(' ');
        }
    }
    if(linha == second_line){
        posiciona_cursor(second_line);
        for(i = 0; i < 16; i++){
            lcd_write(' ');
        }
    }
    if(linha == all_display){
        posiciona_cursor(first_line);
        for(i = 0; i < 16; i++){
            lcd_write(' ');
        }

        posiciona_cursor(second_line);
        for(i = 0; i < 16; i++){
            lcd_write(' ');
        }
    }
}

void ADC_config(void){
    volatile unsigned char *pt;
    unsigned char i;
    ADC12CTL0  &= ~ADC12ENC;         //Desabilitar para configurar
    ADC12CTL0 = ADC12ON;            //Ligar ADC
    ADC12CTL1 = ADC12CONSEQ_3    |   //Modo autoscan repetido
                ADC12SHS_1       |   //Selecionar timer
                ADC12CSTARTADD_0 |
                ADC12SSEL_3;         //ADC12CLK = SMCLK
    pt = &ADC12MCTL0;
    for (i=0; i<8; i++){
        pt[i]=ADC12SREF_0 | ADC12INCH_0; //ADC12MCTL0 até ADC12MCTL7
    }
    for (i=8; i<16; i++){
        pt[i]=ADC12SREF_0 | ADC12INCH_2; //ADC12MCTL8 até ADC12MCTL15
    }
    pt[15] |= ADC12EOS; //EOS em ADC12MCTL15

    P6SEL |= BIT2|BIT0;
    ADC12CTL0 |= ADC12ENC;            //Habilitar ADC12
    ADC12IE |= ADC12IE15;
}

void config_I2C(void){
    P3SEL |=  BIT1 | BIT0;    // Use dedicated module

    UCB0CTL1 |= UCSWRST;    // UCSI B0 em ressete

    UCB0CTL0 = UCSYNC |     //Síncrono
               UCMODE_3 |   //Modo I2C
               UCMST;       //Mestre

    UCB0BRW = BR100K;       //100 kbps
    UCB0CTL1 = UCSSEL_2;   //SMCLK e remove ressete
}

void LCD_inic(void){
    PCF_STT_STP();      //Colocar PCF em estado conhecido
    UCB0I2CSA = PCF_ADR;    //Endereço Escravo
    UCB0CTL1 |= UCTR    |   //Mestre TX
                UCTXSTT;    //Gerar START
    while ( (UCB0IFG & UCTXIFG) == 0);          //Esperar TXIFG=1
    UCB0TXBUF = 0;                              //Saída PCF = 0;
    while ( (UCB0CTL1 & UCTXSTT) == UCTXSTT);   //Esperar STT=0
    if ( (UCB0IFG & UCNACKIFG) == UCNACKIFG)    //NACK?
                while(1);

    LCD_aux(0);     //RS=RW=0, BL=1
    delay(20000);
    LCD_aux(3);     //3
    delay(10000);
    LCD_aux(3);     //3
    delay(10000);
    LCD_aux(3);     //3
    delay(10000);
    LCD_aux(2);     //2

    // Entrou em modo 4 bits
    LCD_aux(2);     LCD_aux(8);     //0x28
    LCD_aux(0);     LCD_aux(8);     //0x08
    LCD_aux(0);     LCD_aux(1);     //0x01
    LCD_aux(0);     LCD_aux(6);     //0x06
    LCD_aux(0);     LCD_aux(0xF);   //0x0F

    while ( (UCB0IFG & UCTXIFG) == 0)   ;          //Esperar TXIFG=1
    UCB0CTL1 |= UCTXSTP;                           //Gerar STOP
    while ( (UCB0CTL1 & UCTXSTP) == UCTXSTP)   ;   //Esperar STOP
    delay(50);
}

void LCD_aux(char dado){
    while ( (UCB0IFG & UCTXIFG) == 0);              //Esperar TXIFG=1
    UCB0TXBUF = ((dado<<4)&0XF0) | BIT_BL;          //PCF7:4 = dado;
    delay(50);
    while ( (UCB0IFG & UCTXIFG) == 0);              //Esperar TXIFG=1
    UCB0TXBUF = ((dado<<4)&0XF0) | BIT_BL | BIT_E;  //E=1
    delay(50);
    while ( (UCB0IFG & UCTXIFG) == 0);              //Esperar TXIFG=1
    UCB0TXBUF = ((dado<<4)&0XF0) | BIT_BL;          //E=0;
}

void PCF_STT_STP(void){
    int x=0;
    UCB0I2CSA = PCF_ADR;        //Endereço Escravo

    while (x<5){
        UCB0CTL1 |= UCTR    |   //Mestre TX
                    UCTXSTT;    //Gerar START
        while ( (UCB0IFG & UCTXIFG) == 0);  //Esperar TXIFG=1
        UCB0CTL1 |= UCTXSTP;                //Gerar STOP
        delay(200);
        if ( (UCB0CTL1 & UCTXSTP) == 0)   break;   //Esperar STOP
        x++;
    }
    while ( (UCB0CTL1 & UCTXSTP) == UCTXSTP);   //I2C Travado (Desligar / Ligar)
}

int PCF_read(void){
    int dado;
    UCB0I2CSA = PCF_ADR;                //Endereço Escravo
    UCB0CTL1 &= ~UCTR;                  //Mestre RX
    UCB0CTL1 |= UCTXSTT;                //Gerar START
    while ( (UCB0CTL1 & UCTXSTT) == UCTXSTT);
    UCB0CTL1 |= UCTXSTP;                //Gerar STOP + NACK
    while ( (UCB0IFG & UCRXIFG) == 0);  //Esperar RX
    dado=UCB0RXBUF;
    delay(50);
    return dado;
}

void PCF_write(char dado){
    UCB0I2CSA = PCF_ADR;        //Endereço Escravo
    UCB0CTL1 |= UCTR    |       //Mestre TX
                UCTXSTT;        //Gerar START
    while ( (UCB0IFG & UCTXIFG) == 0)   ;          //Esperar TXIFG=1
    UCB0TXBUF = dado;                              //Escrever dado
    while ( (UCB0CTL1 & UCTXSTT) == UCTXSTT)   ;   //Esperar STT=0
    if ( (UCB0IFG & UCNACKIFG) == UCNACKIFG)       //NACK?
                while(1);                          //Escravo gerou NACK
    UCB0CTL1 |= UCTXSTP;                        //Gerar STOP
    while ( (UCB0CTL1 & UCTXSTP) == UCTXSTP)   ;   //Esperar STOP
    delay(50);
}

void delay(long limite){
    volatile long cont=0;
    while (cont++ < limite);
}

void config_timer(){
    TA0CTL = TASSEL_1 | MC_1;
    TA0CCTL1 = OUTMOD_6;
    TA0CCR0 = 32767/32;
    TA0CCR1 = TA0CCR0/2;

    //timer para atraso 1s
    TA1CTL = TASSEL_1 | MC_1 | ID_3;
    TA1CCR0 = 4096; //1s de atraso
}

void config_chav(){
    P6DIR &= ~BIT4;
    P6REN |= BIT4;
    P6OUT |= BIT4;

    P1DIR |= BIT0;
    P1OUT &= ~BIT0;
}

volatile int psw = 1;
int verifica_sw(){
   if((P6IN&BIT4) == closed){
       if(psw == open){
           delay(5000);
           psw = closed;
           valorsw++;
           return TRUE;
       }
       else{
           return FALSE;
       }
   }
   else{
       if(psw == open){
           return FALSE;
       }
       else{
           delay(5000);
           psw = open;
           return FALSE;
       }
   }
}

void atualiza_lcd(){
    TA1CTL |= TACLR; //Zerar o contador
    TA1CTL &= ~TAIFG; //Apagar passado de TAIFG
    while ( (TA1CTL&TAIFG) == 0){
        if(verifica_sw()){
            P1OUT ^= BIT0;
            main();
        }
    }
    P1OUT ^= BIT0;
}

/*
Controle PWM (Teclado + LCD)

MPLAX IDE V6.05
XC8 V2.46

Autor: Matheus D. Cipolotti
*/

#include "config.h"
#include <xc.h>

#define _XTAL_FREQ 20000000

//--------------------------Mapeamento Teclado 4x4------------------------------
#define COL1    PORTBbits.RB0
#define COL2    PORTBbits.RB1
#define COL3    PORTBbits.RB2
#define COL4    PORTBbits.RB3
#define LINA    PORTBbits.RB4
#define LINB    PORTBbits.RB5
#define LINC    PORTBbits.RB6
#define LIND    PORTBbits.RB7

//----------------------------Mapeamento LCD 16X2-------------------------------
#define RS PORTDbits.RD0
#define EN PORTDbits.RD1
#define D4 PORTDbits.RD4
#define D5 PORTDbits.RD5
#define D6 PORTDbits.RD6
#define D7 PORTDbits.RD7

char uart_rd, controle = 0x01;
unsigned char PWM_DUTY = 0, PWM_MANUAL = 0;
char i = 0;

void LCD_Command(unsigned char cmd){
	//dsipdata = (dsipdata & 0x0f) |(0xF0 & cmd);  /*Send higher nibble of command first to PORT*/ 
    D4 = (cmd >> 4) & 0x01;
    D5 = (cmd >> 5) & 0x01;
    D6 = (cmd >> 6) & 0x01;
    D7 = (cmd >> 7) & 0x01;
	RS = 0;  /*Command Register is selected i.e.RS=0*/ 
	EN = 1;  /*High-to-low pulse on Enable pin to latch data*/ 
	NOP();
	EN = 0;
	__delay_ms(1);
    //dsipdata = (dsipdata & 0x0f) | (cmd<<4);  /*Send lower nibble of command to PORT */
    D4 = (cmd >> 0) & 0x01;
    D5 = (cmd >> 1) & 0x01;
    D6 = (cmd >> 2) & 0x01;
    D7 = (cmd >> 3) & 0x01;   
	EN = 1;
	NOP();
	EN = 0;
	__delay_ms(3);
}

void LCD_Init(){   
    __delay_ms(40);     /*15ms,16x2 LCD Power on delay*/
    LCD_Command(0x02);  /*send for initialization of LCD 
                          for nibble (4-bit) mode */
    LCD_Command(0x28);  /*use 2 line and 
                          initialize 5*8 matrix in (4-bit mode)*/
	LCD_Command(0x01);  /*clear display screen*/
    LCD_Command(0x0c);  /*display on cursor off*/
	LCD_Command(0x06);  /*increment cursor (shift cursor to right)*/	   
}

void LCD_Char( char dat){
	//dsipdata = (dsipdata & 0x0f) |(0xF0 & cmd);  /*Send higher nibble of command first to PORT*/ 
    D4 = (dat >> 4) & 0x01;
    D5 = (dat >> 5) & 0x01;
    D6 = (dat >> 6) & 0x01;
    D7 = (dat >> 7) & 0x01;
    
	RS = 1;  /*Data Register is selected*/
	EN = 1;  /*High-to-low pulse on Enable pin to latch data*/
	NOP();
	EN = 0;
	__delay_ms(1);

    D4 = (dat >> 0) & 0x01;
    D5 = (dat >> 1) & 0x01;
    D6 = (dat >> 2) & 0x01;
    D7 = (dat >> 3) & 0x01;       
	EN = 1;  /*High-to-low pulse on Enable pin to latch data*/
	NOP();
	EN = 0;
	__delay_ms(3);
}

void LCD_Char_xy(char row,char pos, char dat){
    unsigned char location=0;       

    switch (row){
        case 0:
           location=(unsigned char)((0x80) + (pos));  
           LCD_Command(location);   
           break;
        case 1:
           location=(unsigned char)((0xC0) + (pos));  
           LCD_Command(location);   
           break;
        case 2:
           location=(unsigned char)((0x94) + (pos));  
           LCD_Command(location);   
           break;
        case 3:
           location=(unsigned char)((0xD4) + (pos));  
           LCD_Command(location);   
           break;
    }
    LCD_Char(dat);
}

void LCD_String(const char *msg){
	while((*msg)!=0)
	{		
        LCD_Char(*msg);
        msg++;	
    }
}

void LCD_String_xy(char row,char pos,const char *msg){
    unsigned  char location=0;
    switch (row){
        case 0:
            location=(unsigned char)((0x80) + (pos));  
            LCD_Command(location);   
            break;
        case 1:
            location=(unsigned char)((0xC0) + (pos));  
            LCD_Command(location);   
            break;
        case 2:
            location=(unsigned char)((0x94) + (pos)); 
            LCD_Command(location);   
            break;
        case 3:
            location=(unsigned char)((0xD4) + (pos));  
            LCD_Command(location);   
            break;
           
    }
    LCD_String(msg);
}

void LCD_Clear(){
   	LCD_Command(0x01);  /*clear display screen*/
    __delay_ms(3);
}

void Manual_Limpa(){
    if(i == 0){
        LCD_Clear();
        LCD_String_xy(0,0,"MANUAL:");
    }
}

void Multiplicador(unsigned char valor){
    switch (i){
        case 0:
            PWM_MANUAL = PWM_MANUAL + (valor * 10);
            break;
        case 1:
            PWM_MANUAL = PWM_MANUAL + (valor * 1);
            break;
    }
}

//---------------------------Controle Teclado 4x4-------------------------------
void __interrupt() teclado(void){
    if(INTCONbits.T0IF){
        INTCONbits.T0IF = 0x00;             //Limpa Flag
        TMR0 = 0x6C;                        //Re-inicia o timer
        
        if(COL1 == 0x01 && controle == 0x01){
            controle = 0x02;
            COL1 = 0x00;
            COL2 = 0x01;
            COL3 = 0x01;
            COL4 = 0x01;
            
            if(LINA == 0x00){
                Manual_Limpa();
                LCD_String_xy(1,i,"1");
                Multiplicador(1);
                i++;
                __delay_ms(300);
            }
            else if(LINB == 0x00){
                Manual_Limpa();
                LCD_String_xy(1,i,"4");
                Multiplicador(4);
                i++;
                __delay_ms(300);
            }
            else if(LINC == 0x00){
                Manual_Limpa();
                LCD_String_xy(1,i,"7");
                Multiplicador(7);
                i++;
                __delay_ms(300);
            }
            else if(LIND == 0x00){
                //UART_Write_ln('*');
                __delay_ms(300);
            }
        }
        else if(COL2 == 0x01 && controle == 0x02){
            controle = 0x03;
            COL1 = 0x01;
            COL2 = 0x00;
            COL3 = 0x01;
            COL4 = 0x01;
            
            if(LINA == 0x00){
                Manual_Limpa();
                LCD_String_xy(1,i,"2");
                Multiplicador(2);
                i++;
                __delay_ms(300);
            }
            else if(LINB == 0x00){
                Manual_Limpa();
                LCD_String_xy(1,i,"5");
                Multiplicador(5);
                i++;
                __delay_ms(300);
            }
            else if(LINC == 0x00){
                Manual_Limpa();
                LCD_String_xy(1,i,"8");
                Multiplicador(8);
                i++;
                __delay_ms(300);
            }
            else if(LIND == 0x00){
                Manual_Limpa();
                LCD_String_xy(1,i,"0");
                Multiplicador(0);
                i++;
                __delay_ms(300);
            }
        }
        else if(COL3 == 0x01 && controle == 0x03){
            controle = 0x04;
            COL1 = 0x01;
            COL2 = 0x01;
            COL3 = 0x00;
            COL4 = 0x01;
            
            if(LINA == 0x00){
                Manual_Limpa();
                LCD_String_xy(1,i,"3");
                Multiplicador(3);
                i++;
                __delay_ms(300);
            }
            else if(LINB == 0x00){
                Manual_Limpa();
                LCD_String_xy(1,i,"6");
                Multiplicador(6);
                i++;
                __delay_ms(300);
                
            }
            else if(LINC == 0x00){
                Manual_Limpa();
                LCD_String_xy(1,i,"9");
                Multiplicador(9);
                i++;
                __delay_ms(300);
            }
            else if(LIND == 0x00){
                PWM_DUTY = PWM_MANUAL;
                i = 0;
                PWM_MANUAL = 0;
                __delay_ms(300);
            }
        }
        else if(COL4 == 0x01 && controle == 0x04){
            controle = 0x01;
            COL1 = 0x01;
            COL2 = 0x01;
            COL3 = 0x01;
            COL4 = 0x00;
            
            if(LINA == 0x00){
                LCD_Clear();
                LCD_String_xy(0,1,"PWM MODO A:");
                PWM_DUTY = 25;
                LCD_String_xy(1,1,"25%");
                __delay_ms(300);
            }
            else if(LINB == 0x00){
                LCD_Clear();
                LCD_String_xy(0,1,"PWM MODO B:");
                PWM_DUTY = 50;
                LCD_String_xy(1,1,"50%");
                __delay_ms(300);
            }
            else if(LINC == 0x00){
                LCD_Clear();
                LCD_String_xy(0,1,"PWM MODO C:");
                PWM_DUTY = 75;
                LCD_String_xy(1,1,"75%");
                __delay_ms(300);
            }
            else if(LIND == 0x00){
                LCD_Clear();
                LCD_String_xy(0,1,"PWM MODO D:");
                PWM_DUTY = 95;
                LCD_String_xy(1,1,"95%");
                __delay_ms(300);
            }
        }
    }
}

void main(void) {
    CMCON = 0x07;
    ADCON0 = 0x00;                  //Desabilita os conversores AD
    ADCON1 = 0x06;                  //Torna todo ADC digital 
    
    OPTION_REG = 0x86;
    INTCONbits.GIE = 0x01;
    INTCONbits.PEIE = 0x01;
    INTCONbits.T0IE = 0x01;
    TMR0 = 0x6C;

/*
------------------------------Configuracao TMR2---------------------------------
    Ciclo de maquina = (1/(_XTAL_FREQ/4)) = (1/(20000000/4)) = 0,00000025 = 0,25uS
 
    periodo = Ciclo de maquina x (PR2 + 1) x prescaler
    periodo = 0,00000025 x (249 + 1) x 16
    frequencia = 1/periodo = 1KHz
    
    PR2 + 1 = TMR2 / CCPR1L:CCP1CON<5:4> 
*/    
    
    PR2 = 0xF9;                             //Comparador PR2 em 249
    T2CON = 0x07;                           //postscaler 1:1 e prescaler 1:16
    CCP1CON = 0x0C;                         //Habilita o modo PWM
    
    TRISA = 0x00;
    TRISB = 0xF0;
    TRISC = 0x00;
    TRISD = 0x00;
    TRISE = 0x00;
    PORTA = 0x00;
    PORTB = 0xFF;
    PORTC = 0x00;
    PORTD = 0x00;
    PORTE = 0x00;
    
    LCD_Init();
    LCD_Clear();
    LCD_String_xy(0,1,"Controle PWM");
    
    while(1){
        CCPR1L = ((PWM_DUTY * 255)/100);        
    }
}

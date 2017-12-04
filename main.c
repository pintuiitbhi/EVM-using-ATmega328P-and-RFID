
#define F_CPU 16000000UL
#include<avr/io.h>
#include<avr/interrupt.h>
#include<avr/eeprom.h>
#include<util/delay.h>
#include<string.h>
#include<stdlib.h>

//#include "lcd.h"



//D4-D7 connected to D4-D7

#define rs PB0    //pin8 
#define en PB1    //pin9
#define USART_BAUDRATE 9600 //9600 default
#define UBRR_VALUE (((F_CPU/(USART_BAUDRATE*16UL)))-1)

int base_address[8]={64,72,80,88,96,104,112,120};

extern void delay(uint8_t, uint8_t, uint8_t);// Assembly function call
void start();
void command(char);
void data(char);
void Send_A_String(char *);
void Send_An_Integer(int);
void setCursor(int,int);
void clearScreen();



void usartinit(void)
{
    // Set baud rate
    UBRR0H = (uint8_t)(UBRR_VALUE>>8);
    UBRR0L = (uint8_t)UBRR_VALUE;
    // Set frame format to 8 data bits, no parity, 1 stop bit
    UCSR0C |= (1<<UCSZ01)|(1<<UCSZ00);
    //enable transmission and reception
    UCSR0B |= (1<<RXEN0)|(1<<TXEN0);
}


unsigned char a;
unsigned char value[15];
unsigned int b,votedA=1,votedB=1,votedC=1,votedD=1;
unsigned int k=0,i=0,j,l,A=0,B=0,C=0,D=0,flag=1,lock=1,revert=0,password=3,pulldown=1;
unsigned char value1[]={"4400222A236F"};            //Unique ID
unsigned char value2[]={"44001C478699"};      
unsigned char value3[]={"MASTER"};                  //Names assigned to ID
unsigned char value4[]={"STUDENT"};
unsigned char value5[]={"Unknown number"};
unsigned char value6[]={"Now Vote"};
unsigned char value7[]={"0123456789"}; 


void update_count ( int candidate_address )
{
int base_address = 100;
unsigned int count = 0;
 
count = eeprom_read_byte ( ( unsigned char * ) ( base_address + candidate_address ) );
count ++;
eeprom_write_byte ( ( unsigned char * ) ( base_address + candidate_address ), count );
}


void reset_count ( void )
{
int base_address = 100;
int i = 0;
for ( i = 0; i < 4; i ++ )
eeprom_write_byte ( ( unsigned char * ) ( base_address + i ), 0 );
 start();
 Send_A_String("Reset [OK]");
 _delay_ms(3000);
 start();
 _delay_ms(2);
 usartinit();
 _delay_ms(1);
 
}


void send_count_out ( void )
{
int base_address = 100;
 clearScreen();
 Send_A_String("Results:");
 int data= eeprom_read_byte ( ( unsigned char * ) ( base_address + 0 ) ) ;
 setCursor(1,0);
 Send_A_String("A=");
 Send_An_Integer(data);
 Send_A_String(" ");
 data= eeprom_read_byte ( ( unsigned char * ) ( base_address + 1 ) ) ;
 Send_A_String("B=");
 Send_An_Integer(data);
 Send_A_String("  ");
 data= eeprom_read_byte ( ( unsigned char * ) ( base_address + 2 ) ) ;
 Send_A_String("C=");
 Send_An_Integer(data);
 Send_A_String(" ");
 data= eeprom_read_byte ( ( unsigned char * ) ( base_address + 3) ) ;
 Send_A_String("D=");
 Send_An_Integer(data);
 Send_A_String(" ");
}

void candidate()
{
clearScreen();
			setCursor(0,0);
			Send_A_String("A");
			setCursor(0,3);
			Send_A_String("Candidates:");
			setCursor(0,15);
			Send_A_String("B");
			setCursor(1,0);
			Send_A_String("C");
			setCursor(1,15);
			Send_A_String("D");	
}




int main ()
{	
	
DDRB = 0x03;
DDRD = 0xF0;
DDRC=0XFF;
	
EICRA |=(1<<ISC00);
EIMSK |=(1<<INT0);
sei();
	
	SREG = 0;
	SPL = (uint8_t)(RAMEND);
	SPH = (uint8_t)(RAMEND>>8);

_delay_ms(200);
	start();

while(lock)
{
	pulldown=1;
	PORTC|=(1<<2);
	_delay_ms(100);
	Send_A_String("LOCKED...Enter");
	_delay_ms(10);
	setCursor(1,0);
	Send_A_String("PWD(B1)  OK(B2) ");
 	_delay_ms(10);
	while((PINB & (1<<PB2)))
	{
		
		if(pulldown==1 && revert==0)
		{
		password=password+1;
		
		pulldown=0;
		}
		if(revert==1)
		{
		clearScreen();
		Send_A_String("Cheating...");
		revert=0;
		password=0;
		_delay_ms(3000);
		clearScreen();
		}
		PORTC|=(1<<0);
	}
	PORTC&=~(1<<0);
	pulldown=1;
	
	while((PINB & (1<<PB3)))
	{
		revert=1;
		if(password==3)
		{
			 clearScreen();
		Send_A_String("   Password");
      		setCursor(1,0);
      		 Send_A_String("   Matched  :)");
		PORTC|=(1<<0);
		PORTD=(1<<2);
		_delay_ms(3000);
		PORTC&=~(1<<0);
		//PORTD=(1<<2)|PORTD;
		lock=0;
		revert=0;
		}
		else
		{
			revert=0;
			password=0;
			clearScreen();
			Send_A_String("  WRONG  Pwd :(");
      			setCursor(1,0);
       			Send_A_String("...Go Slow...");
			PORTC|=(1<<1);
			_delay_ms(2500);
			PORTC&=~(1<<1);
			clearScreen();	
		}
	 
	}
	pulldown=1;	 
}

	PORTC&=~(1<<2);

 	candidate();
	  _delay_ms(3500);
 

 	
	 usartinit();
	  clearScreen();
	 Send_A_String("Please  Show");
      	setCursor(1,0);
      	 Send_A_String("Your  Card...");

  	while(1)
  	{  
	  
  	 while((UCSR0A)&(1<<RXC0))//wait until a byte is ready to read
   	{
    		value[i]=UDR0;//grab the byte from the serial port
    		_delay_ms(1);
    		i++;
    		if(i==12)
   	 {
      		value[i]='\0';
      		for(j=0;value1[j]!='\0';j++)
      		{
       		if(value[j]==value1[j])
       			k++;
      		}
      		if(k==12)                      //Checking the ID with predefined ID
      			{
       clearScreen();
       setCursor(0,0);
       for(l=0;value6[l]!='\0';l++)
       {
        a=value6[l];                 //Assigning names
        data(a);
        _delay_ms(100);
       }
       
       votedA=1;
       votedB=1;
       votedC=1;
       votedD=1;
       while(flag)
       {
		   
       while((PINB & (1<<PB2))){//button3
			clearScreen();		
			A=A+1;
			if(votedA==1)
			{update_count ( 0 );}
		
			
		
			//char *p;
			
			//itoa(C,p,10);
		//Send_A_String(p);
		setCursor(0,0);
		PORTC|=(1<<0);
		Send_A_String("You Voted A");
		setCursor(1,0);
		Send_A_String("Thank You");
		votedA=0;
		
		
		delay(0,0,54);//Assembly function call

		flag=0;
		PORTC&=~(1<<0);
		}
		
		       while((PINB & (1<<PB3))){//button3
			clearScreen();
			
			B=B+1;
			if(votedB==1)
			{update_count (1);}
			
			//char *p;
			
			//itoa(C,p,10);
		//Send_A_String(p);
		setCursor(0,0);
		PORTC|=(1<<0);
		Send_A_String("You Voted B");
		setCursor(1,0);
		Send_A_String("Thank You :)");
		votedB=0;
		
		
		_delay_ms(1000);
		flag=0;
		PORTC&=~(1<<0);
		}
		
		       while((PINB & (1<<PB4))){//button3
			clearScreen();
			
			C=C+1;
			if(votedC==1)
			{update_count ( 2 );}
			
			
			
			
			//char *p;
			
			//itoa(C,p,10);
		//Send_A_String(p);
		setCursor(0,0);
		PORTC|=(1<<0);
		Send_A_String("You Voted C");
		setCursor(1,0);
		Send_A_String("Thank You :)");
		votedC=0;
		
		_delay_ms(1000);
		flag=0;
		PORTC&=~(1<<0);
		}
		
		       while((PINB & (1<<PB5))){//button3
			clearScreen();
			D=D+1;
			if(votedD==1){
			update_count ( 3);}
			
			
			
			//char *p;
			
			//itoa(C,p,10);
		//Send_A_String(p);
		setCursor(0,0);
		PORTC|=(1<<0);
		Send_A_String("You Voted D");
		setCursor(1,0);
		Send_A_String("Thank You :)");
		votedD=0;
		
		delay(0,0,54);//Assembly function call

		flag=0;
		PORTC&=~(1<<0);
		}
		
			}
       
       
       flag=1;
       i=0;
       k=0;
      }
    
     else
      {
		  PORTC|=(1<<1);
      		clearScreen();
    	   setCursor(0,0);
    	   for(l=0;value5[l]!='\0';l++)
    	    {
    	     a=value5[l];
    	     data(a);
    	     _delay_ms(1);
    	    }
    	    i=0;
    	    k=0;
    	    delay(0,0,54);//Assembly function call
    	    PORTC&=~(1<<1);
    	  }
     
      // _delay_ms(10);
      _delay_ms(2000);
       clearScreen();
	 Send_A_String("Please  Show");
      setCursor(1,0);
       Send_A_String("Your  Card...");
    }
   
  }
  while((PINB & (1<<PB5)))
	{
	send_count_out ();
	_delay_ms(2000);
	while(1);	
	}
 }
return 0;
}

 ISR(INT0_vect)//interuupt
{
	lock=1;
	reset_count();
	main();
} 


//LCD Interface function

void start()  
{
	DDRB = 0x03;    // PB0 and PB1 declared as output
	DDRD = 0xF0;    // PD4,PD5,PD6,PD7 declared as output
	command(0x33);
	command(0x32);
	command(0x28);	// To initialize LCD in 2 lines, 5X8 dots and 4bit mode.
	_delay_ms(30);
	
		command(0x01);
	_delay_ms(10);
	
	command(0x0C);	// Display ON cursor OFF. E for cursor ON and C for cursor OFF
	_delay_ms(1);
	command(0x06);	// Entry mode-increment cursor by 1
	_delay_ms(1);
	// Clear screen
	command(0x80);	// Sets cursor to (0,0)		
	
	

}




void command(char LcdCommand)  // Basic function used in giving command
{                              // to LCD
	char UpperHalf, LowerHalf;
	
	UpperHalf = LcdCommand & 0xF0;	// upper 4 bits of command
	PORTD &= 0x0F;                  // flushes upper half of PortD to 0, but retains lower half of PortD
	PORTD |= UpperHalf;
	PORTB &= ~(1<<rs);
	PORTB |= (1<<en);
	_delay_ms(10);
	PORTB &= ~(1<<en);
	_delay_ms(10);
	
	LowerHalf = ((LcdCommand<<4) & 0xF0); //lower 4 bits of command
	PORTD &= 0x0F;                  // flushes upper half of PortD to 0, but retains lower half of PortD
	PORTD |= LowerHalf;
	PORTB &= ~(1<<rs);
	PORTB |= (1<<en);
	_delay_ms(10);
	PORTB &= ~(1<<en);
	_delay_ms(10);
}
	 
void data(char AsciiChar)    // Basic function used in giving data
{                            // to LCD
	char UpperHalf, LowerHalf;
	
	UpperHalf = AsciiChar & 0xF0;	// upper 4 bits of data
	PORTD &= 0x0F;       // flushes upper half of PortD to 0, but retains lower half of PortD
	PORTD |= UpperHalf;
	PORTB |= (1<<rs);
	PORTB |= (1<<en);
	_delay_ms(10);
	PORTB &= ~(1<<en);
	_delay_us(400);
	
	LowerHalf = ((AsciiChar<<4) & 0xF0); //lower 4 bits of data
	PORTD &= 0x0F;       // flushes upper half of PortD to 0, but retains lower half of PortD
	PORTD |= LowerHalf;
	PORTB |= (1<<rs);
	PORTB |= (1<<en);
	_delay_ms(10);
	PORTB &= ~(1<<en);
	_delay_us(400);	
}

	


void Send_A_String(char *StringOfCharacters)     // Takes input a string and displays it
{                                                // max character in a string should be 
	while(*StringOfCharacters > 0)           // less than 16, after 16th character 
	{                                        // everything will be ignored.
		data(*StringOfCharacters++);
		
	}
}
 



void Send_An_Integer(int x)     // Takes an integer as input and displays it
{                               // value of integer should be in between
	char buffer[8];         // the range of "int",
	itoa(x,buffer,10);      //  else it'll print garbage values.
	Send_A_String(buffer);  // It use Send_A_String() for displaying.
}



void setCursor(int row,int column)             // Indexing starts from 0.
{                                              // Therefore,
	switch(row)                            // 0<=row<=1 and 0<=column<=15.
	{                                      //
		case 0:                        // If arguments are outside the
			switch(column)         // the specified range, then function
			{                      // will not work and ignore the values
				case 0:
					command(0x80);break;
				case 1:
					command(0x81);break;
				case 2:
					command(0x82);break;
				case 3:
					command(0x83);break;
				case 4:
					command(0x84);break;
				case 5:
					command(0x85);break;
				case 6:
					command(0x86);break;
				case 7:
					command(0x87);break;
				case 8:
					command(0x88);break;
				case 9:
					command(0x89);break;
				case 10:
					command(0x8A);break;
				case 11:
					command(0x8B);break;
				case 12:
					command(0x8C);break;
				case 13:
					command(0x8D);break;
				case 14:
					command(0x8E);break;
				case 15:
					command(0x8F);break;
				default:
					break;
			}
			break;
		case 1:
			switch(column)
			{
				case 0:
					command(0xC0);break;
				case 1:
					command(0xC1);break;
				case 2:
					command(0xC2);break;
				case 3:
					command(0xC3);break;
				case 4:
					command(0xC4);break;
				case 5:
					command(0xC5);break;
				case 6:
					command(0xC6);break;
				case 7:
					command(0xC7);break;
				case 8:
					command(0xC8);break;
				case 9:
					command(0xC9);break;
				case 10:
					command(0xCA);break;
				case 11:
					command(0xCB);break;
				case 12:
					command(0xCC);break;
				case 13:
					command(0xCD);break;
				case 14:
					command(0xCE);break;
				case 15:
					command(0xCF);break;
				default:
					break;
			}
			break;
		default:
			break;
	}

}




void clearScreen()         // Clears the screen and
{                          // returns cursor to (0,0) position
	command(0x01);
}





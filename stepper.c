#define F_CPU 14745600

#include <stdio.h>
#include <math.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <inttypes.h>

#include "../libnerdkits/delay.h"
#include "../libnerdkits/lcd.h"
#include "../libnerdkits/uart.h"

#define Ob(x)  ((unsigned)Ob_(0 ## x ## uL))
#define Ob_(x) ((x & 1) | (x >> 2 & 2) | (x >> 4 & 4) | (x >> 6 & 8) |		\
	(x >> 8 & 16) | (x >> 10 & 32) | (x >> 12 & 64) | (x >> 14 & 128))

// PIN DEFINITIONS:
// (In order-ish on chip)
// 15 PB1 -- 
// 16 PB2 -- 
// 17 PB3 -- 
// 18 PB4 -- 
// 19 PB5 -- 
// 20 AVCC -- +5V
// 21 AREF -- +5V
// 22 GND
// 23 PC0 -- stepper 0
// 24 PC1 -- stepper 1
// 25 PC2 -- stepper 2
// 26 PC3 -- stepper 3
// 27 PC4 -- 
// 28 PC5 -- 
// 
// 14 PB0 -- Bootloading pin (pull to gnd for programming)
// 13 PD7 -- LCD Register Select (!cmd/data)
// 12 PD6 -- LCD Clock
// 11 PD5 -- LCD Data bit 7
// 10 PB7 -- XTAL2
//  9 PB6 -- XTAL1
//  8 GND
//  7 VCC -- +5V
//  6 PD4 -- LCD Data bit 6
//  5 PD3 -- LCD Data bit 5
//  4 PD2 -- LCD Data bit 4
//  3 PD1 -- UART TX (green)
//  2 PD0 -- UART RX (yellow)
//  1 PC6 -- !reset (+5V)

volatile unsigned int wait_ms = 1;

void try_uart(){
  if(!uart_char_is_waiting()){
    return;
  }
  char tc = UDR0;
  if(tc=='a'){
    wait_ms += 100;
  }
  else if(tc=='z'){
    if(wait_ms > 100){
      wait_ms -= 100;   
    } 
  }
  else if(tc=='s'){
    wait_ms += 10;
  }
  else if(tc=='x'){
    if(wait_ms > 10){
      wait_ms -= 10;   
    } 
  }
  else if(tc=='d'){
    wait_ms += 1;
  }
  else if(tc=='c'){
    if(wait_ms > 1){
      wait_ms -= 1;   
    } 
  }
}

int seq[8] = 
{
0x1, //0b0001,
0x3, //0b0011,
0x2, //0b0010,
0x6, //0b0110,
0x4, //0b0100,
0xc, //0b1100,
0x8, //0b1000,
0x9 //0b1001
};

int seq_idx = 0;
int count = 0;
int rev = 0;
FILE lcd_stream;

int main() {
  //configure I/O pins
  DDRC |= 0xf << PC1;  //enable PC0 thru PC3 as output
  PORTC = 0;

  // start up the LCD
  lcd_init();
  FILE lcd_stream2 = FDEV_SETUP_STREAM(lcd_putchar, 0, _FDEV_SETUP_WRITE);
  lcd_stream = lcd_stream2;
  lcd_home(); 

  // start up the serial port
  uart_init();
  FILE uart_stream = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);
  stdin = stdout = &uart_stream;

  // loop forever!
  while(1) {
    PORTC = seq[seq_idx] << PC1;
    if(++seq_idx == 8){
      seq_idx = 0;
      if(++count == 513){
        count = 0;
        lcd_write_string(PSTR("rev: "));
        lcd_write_int16(++rev);
        lcd_home();
        PORTC = 0;
        delay_ms(1000);
      }
    }
    try_uart();
    delay_ms(wait_ms);
  }
  
  return 0;
}

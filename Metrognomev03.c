// Metronome-v03

//need to program fuse bits with command to get 8MHz internal clock:
//avrdude -c usbtiny -p atmega328p -U lfuse:w:0xE2:m

#define F_CPU 8000000	// Adjust this to get the clock more precise
//#define F_CPU 980000	// Adjust this to get the clock more precise

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define BUZZER1 	1
#define BUZZER1_PORT	PORTB
#define BUZZER2		2
#define BUZZER2_PORT	PORTB

#define sbi(port_name, pin_number)   (port_name |= 1<<pin_number)
#define cbi(port_name, pin_number)   ((port_name) &= (uint8_t)~(1 << pin_number))

uint16_t counter = 0;
uint8_t leftDisplay = 0;		// Initialize output to show 60 BPM
uint8_t rightDisplay = 0;

#define counter_limit 1013

void display(int digit, int number);
void binary_display();

// Interrupt Timer 1 makes the buzzer tick at proper intervals
ISR(TIMER1_COMPA_vect)
{
	counter++;
}

int main()
{
	// set PORTB for Buzzer and buttons
	DDRB = DDRB | 0b00110110;
	PORTB = PORTB | 0b00110000;
	// set PORTC for DIGI select
	DDRC = 0b0000011;
	PINC = 0b0000011;
	// set PORTD for display
	DDRD = 0b11111111;

	// Set 16-bit Timer 1 for clicking
	TCCR1A = 0x00;
	TCCR1B = (_BV(WGM12) | _BV(CS10) );	// no prescaler 1024, CTC mode
	TIMSK1 = _BV(OCIE1A);	// Enable OCR1A interrupt

	sei();	// Enable interrupts

	// Reset counter and adjust compare register
	TCNT1 = 0x00;
	OCR1A = counter_limit;

	while(1)	// Main loop PWM's the two displays at 1kHz
	{
		if( (PINB & (1<<4)) == 0)
			TCCR1B = (_BV(WGM12) | _BV(CS12) | _BV(CS10) );	// divide by 1024, CTC mode
		else
			TCCR1B = (_BV(WGM12) | _BV(CS11) );	// divide by 8, CTC mode

//		binary_display();

		if(counter >= 300)
			counter = 0;
		rightDisplay = counter % 10;
		leftDisplay = (counter/10) % 10;

		display(0, leftDisplay);	// Output to left display
		_delay_us(100);
		display(1, rightDisplay);	// Output to right display
	}
	return 0;
}

void binary_display() {
	cbi(PORTC, 1);	// Turn right display off
	PORTD = counter>>8;
	sbi(PORTC, 0);	// Turn left display on
	_delay_us(100);

	cbi(PORTC, 0);	// Turn left display off
	PORTD = counter;
	sbi(PORTC, 1);	// Turn right display on
	_delay_us(100);
}

// This will output the corresponding
// 'number' to digit 0 (left) or 1 (right)
void display(int digit, int number)
{

	if (digit == 0)
		cbi(PORTC, 1);	// Turn right display off
	else if (digit == 1)
		cbi(PORTC, 0);	// Turn left display off

	switch(number)	// Set PIND, display pins, to correct output
	{
		case 0:
			PORTD = 0b11000000;
			break;
		case 1:
			PORTD = 0b11111001;
			break;
		case 2:
			PORTD = 0b10100100;
			break;
		case 3:
			PORTD = 0b10110000;
			break;
		case 4:
			PORTD = 0b10011001;
			break;
		case 5:
			PORTD = 0b10010010;
			break;
		case 6:
			PORTD = 0b10000010;
			break;
		case 7:
			PORTD = 0b11111000;
			break;
		case 8:
			PORTD = 0b10000000;
			break;
		case 9:
			PORTD = 0b10010000;
			break;
	}
	// Turn decimal point on if above 100 & 200
	if ((digit == 0) && (counter >= 200))
		cbi(PORTD, 7);
	if ((digit == 1) && (counter >= 100))
		cbi(PORTD, 7);

	if (digit == 0)
		sbi(PORTC, 0);	// Ties display to ground
	else if (digit == 1)
		sbi(PORTC, 1);

}

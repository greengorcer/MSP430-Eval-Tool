// Nathan Zimmerman
// MSP430G2553 GUI COMM
// 3/16/12
//Rev 0.5 SPI Implemented

// A GUI to control all basic IO functions on the msp430G2553

#include <msp430g2553.h>
#include "stdbool.h"

////////////////Defines////////////////

#define Green_LED BIT6
#define Button BIT3
#define ERROR BIT0

unsigned int buffer = 0;
unsigned int command = 0;
unsigned int sub_command = 0;
unsigned int data1 = 0;
unsigned int data2 = 0;
unsigned int send_data[8];
unsigned int *sendptr;
unsigned int *recvptr;
unsigned int j = 0; //Define end of string loop int
unsigned int recieved_data[9]={0,0,0,0,0,0,0,0,0};

unsigned int send_data_counter = 0;
unsigned int baudrate=0;
volatile unsigned int chipselect=0;
unsigned int datalenght=0;
bool CS_port2 = false;
bool Command_Ready = false;
unsigned int comm_p = 0;
////////////////Function Protos////////////////

void TX(char *tx_message); // Transmit char string
void DIR_P1(); // 0x2
void OUT_P1(); // 0x3
void REN_P1(); // 0x4
void SEL_P1(); // 0x5
void SEL2_P1(); // 0x6
void DIR_P2(); // 0x7
void OUT_P2(); // 0x8
void REN_P2(); // 0x9
void SEL_P2(); // 0xA
void SEL2_P2(); // 0xB
void PxIN(); // 0xC
void SPI_B_SETUP(); //0xD
void SPI_B_SEND(unsigned int *dataz, unsigned int data_length); //0xE
void SPI_Process_Command();

void process_command(); //Main function for doing actions based upon recieved data

////////////////Main Program////////////////

void main(void) {
//unsigned char buffer[2];

	recvptr=recieved_data;
	sendptr=send_data;

	WDTCTL = WDTPW + WDTHOLD; // Stop WDT

	BCSCTL1 = CALBC1_1MHZ; // Set DCO to 1MHz
	DCOCTL = CALDCO_1MHZ;

////////////////USCI setup////////////////

	P1SEL = BIT1 + BIT2; // Set P1.1 to RXD and P1.2 to TXD
	P1SEL2 = BIT1 + BIT2; //

	UCA0CTL1 |= UCSSEL_2; // Have USCI use SMCLK AKA 1MHz main CLK

	UCA0BR0 = 104; // Baud: 9600, N= CLK/Baud, N= 10^6 / 9600

	UCA0BR1 = 0; // Set upper half of baud select to 0

	UCA0MCTL = UCBRS_1; // Modulation UCBRSx = 1

	UCA0CTL1 &= ~UCSWRST; // Start USCI

	IE2 |= UCA0RXIE; //Enable interrupt for USCI

	P1OUT = BIT1 + BIT2; //Debug LEDs

	__enable_interrupt(); //Guess

////////////////General GPIO Defines////////////////

////////////////Main Loop////////////////

	while (1) {
		process_command();
	}

} // End Main Function

////////////////Interrupts////////////////

#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void) {

	if((IFG2 & UCA0RXIFG))
	{
		buffer = UCA0RXBUF;

		switch (comm_p) {
		case 0:
			command = buffer;

			comm_p++;
			if (buffer == '1') {
				TX("PING");
				comm_p = 0;
				command = 0;
			}
			break;

		case 1:
			data1 = buffer;
			comm_p++;
			break;

		case 2:
			data2 = buffer;
			Command_Ready = true;
			comm_p++;
			break;

		default:
			Command_Ready = false;
		} // End Switch

	}

	if((IFG2 & UCB0RXIFG))
	{
		 // while (!(IFG2 & UCB0TXIFG));              // USCI_A0 TX buffer ready?
		recieved_data[j] = UCB0RXBUF;
	}



}



////////////////Functions////////////////

void TX(char *tx_message) {
	unsigned int i = 0; //Define end of string loop int
	char *message; // message variable
	unsigned int message_num; // define ascii int version variable
	message = tx_message; // move tx_message into message

	while (1) {

		message_num = (int) message[i]; //Cast string char into a int variable

		UCA0TXBUF = message_num; // write INT to TX buffer

		i++; // increase string index

		__delay_cycles(2500); //transmission delay

		if (i > 20) //prevent infinite transmit
				{
//P1OUT |= ERROR;
			break;
		}

		if (message[i] == 0) // If end of input string is reached, break loop.
				{
			__delay_cycles(500);
			UCA0TXBUF = 0;
			break;
		}

	} // End TX Main While Loop
} // End TX Function

////////////////DIR_P1()////////////////

void DIR_P1() {

	if ((data1 & BIT0) == BIT0) {
		P1DIR &= (~data2) | (BIT1 + BIT2);
	} else {
		P1DIR |= data2;
	}

} // End Function

///////////////////////////////////////////

////////////////OUT_P1()////////////////

void OUT_P1() {

	if ((data1 & BIT0) == BIT0) {
		P1OUT &= (~data2) | (BIT1 + BIT2);
	} else {
		P1OUT |= data2;
	}

} // End Function

///////////////////////////////////////////

////////////////REN_P1()////////////////

void REN_P1() {

	if ((data1 & BIT0) == BIT0) {
		P1REN &= (~data2) | (BIT1 + BIT2);
	} else {
		P1REN |= data2;
	}

} // End Function

///////////////////////////////////////////

////////////////SEL1_P1()////////////////

void SEL_P1() {

	if ((data1 & BIT0) == BIT0) {
		P1SEL &= (~data2) | (BIT1 + BIT2);
	} else {
		P1SEL |= data2;
	}

} // End Function

///////////////////////////////////////////

////////////////SEL2_P1()////////////////

void SEL2_P1() {

	if ((data1 & BIT0) == BIT0) {
		P1SEL2 &= (~data2) | (BIT1 + BIT2);
	} else {
		P1SEL2 |= data2;
	}

} // End Function

///////////////////////////////////////////

////////////////DIR_P2()////////////////

void DIR_P2() {

	if ((data1 & BIT0) == BIT0) {
		P2DIR &= (~data2);
	} else {
		P2DIR |= data2;
	}

} // End Function

////////////////OUT_P2()////////////////

void OUT_P2() {

	if ((data1 & BIT0) == BIT0) {
		P2OUT &= (~data2) | (BIT1 + BIT2);
	} else {
		P2OUT |= data2;
	}

} // End Function

///////////////////////////////////////////

////////////////REN_P2()////////////////

void REN_P2() {

	if ((data1 & BIT0) == BIT0) {
		P2REN &= (~data2) | (BIT1 + BIT2);
	} else {
		P2REN |= data2;
	}

} // End Function

///////////////////////////////////////////

////////////////SEL1_P2()////////////////

void SEL_P2() {

	if ((data1 & BIT0) == BIT0) {
		P2SEL &= (~data2) | (BIT1 + BIT2);
	} else {
		P2SEL |= data2;
	}

} // End Function

///////////////////////////////////////////

////////////////SEL2_P2()////////////////

void SEL2_P2() {

	if ((data1 & BIT0) == BIT0) {
		P2SEL2 &= (~data2) | (BIT1 + BIT2);
	} else {
		P2SEL2 |= data2;
	}

} // End Function

///////////////////////////////////////////

////////////////PxIN()////////////////

void PXIN() {
	char databuf[4];
	char *test;

	databuf[0] = P1IN;
	databuf[1] = P2IN;
	databuf[2] = 'A';
	databuf[3] = 0;
	test = databuf;

	TX(test);

} // End Function

///////////////////////////////////////////

////////////////SPI_B_SETUP()////////////////

void SPI_B_SETUP() {

	P1SEL |= BIT5 + BIT6 + BIT7;
	P1SEL2 |= BIT5 + BIT6 + BIT7;

	UCB0CTL0 |= UCCKPL + UCMSB + UCMST + UCSYNC; // 3-pin, 8-bit SPI master

	//UCMSB Most significant bit first
	//UCCKPL The inactive state is high.
	//UCMST Master Mode
	//UCSYNC Synchronous mode

	UCB0CTL1 |= UCSSEL_2; // SMCLK
	//UCB0BR0 |= 0xA; // 1MHZ / 10 = 100khz SCLK
	//UCB0BR1 = 0; // 1MHZ / 10 = 100khz SCLK
	UCB0CTL1 &= ~UCSWRST; // **Initialize USCI state machine**
	IE2 |= UCB0RXIE;

}

///////////////////////////////////////////

////////////////SPI_B_SEND()////////////////

void SPI_B_SEND(unsigned int *dataz, unsigned int data_length)
{
	j = 0;
				//Ensure TXBUF is ready
	if(CS_port2)
			{
				P2OUT &=~chipselect;
			}
			else
			{
				P1OUT &=~chipselect;
			}

	while (j<data_length)
	{
			UCB0TXBUF = dataz[j]; // write INT to TX buffer
			while (!(IFG2 & UCB0TXIFG));
			j++; // increase string index
	}

	while ((UCB0STAT & UCBUSY));

	if(CS_port2)
				{
				P2OUT |=chipselect;
				}
			else
				{
					P1OUT |=chipselect;
				}


	j=0;
}

///////////////////////////////////////////

////////////////SPI_Process_Command()////////////////
void SPI_Process_Command()
{

	switch(data1)
	{

	case 0x0:
		send_data[0]= data2;
		datalenght++;
		break;

	case 0x1:
		send_data[1]= data2;
		datalenght++;
		break;

	case 0x2:
		send_data[2] = data2;
		datalenght++;
		break;

	case 0x3:
		send_data[3] = data2;
		datalenght++;
		break;

	case 0x4:
		send_data[4] = data2;
		datalenght++;
		break;

	case 0x5:
		send_data[5] = data2;
		datalenght++;
		break;

	case 0x6:
		send_data[6] = data2;
		datalenght++;
		break;

	case 0x7:
		send_data[7] = data2;
		datalenght++;
		break;

	case 0x8:
		switch(data2)
		{
			case 0x0:
				baudrate=0x1;
				UCB0BR0 = baudrate;
				UCB0BR1 = 0;
				break;

			case 0x1:
				baudrate=0x2;
				UCB0BR0 = baudrate;
				UCB0BR1 = 0;
				break;

			case 0x2:
				baudrate=0xA;
				UCB0BR0 = baudrate;
				UCB0BR1 = 0;
				break;

			case 0x3:
				baudrate=0x64;
				UCB0BR0 = baudrate;
				UCB0BR1 = 0;
				break;

			case 0x4:
				UCB0BR0 =0xE8;
				UCB0BR1 = 0x3;
				break;

			default:
				baudrate=0x64;
				UCB0BR0 = baudrate;
				UCB0BR1 = 0;
		}
		break;

	case 0x16:
		if(data2<8)
		{
			chipselect=(1<<data2);
			//P1DIR |=(1<<chipselect);
			//P1OUT |=(1<<chipselect);
			CS_port2 = false;

		}
		else
		{
			chipselect=(1<<(data2 - 8));
			//P2DIR |=chipselect;
			//P2OUT |=chipselect;
			CS_port2 = true;
		}
		break;
	case 0x32:
		SPI_B_SETUP();
		SPI_B_SEND(sendptr, datalenght);


		__delay_cycles(2500);
		UCA0TXBUF = recieved_data[1];
		__delay_cycles(2500);
		UCA0TXBUF = recieved_data[2];
		__delay_cycles(2500);
		UCA0TXBUF = recieved_data[3];
		__delay_cycles(2500);
		UCA0TXBUF = recieved_data[4];
		__delay_cycles(2500);
		UCA0TXBUF = recieved_data[5];
		__delay_cycles(2500);
		UCA0TXBUF = recieved_data[6];
		__delay_cycles(2500);
		UCA0TXBUF = recieved_data[7];
		__delay_cycles(2500);
		UCA0TXBUF = recieved_data[8];
		__delay_cycles(2500);
		UCA0TXBUF = 0;

		datalenght=0;


	}


}






////////////////process_command////////////////

void process_command() // Main processing function
{

	while (!(Command_Ready)); //Wait till 2 packets are recieved
	Command_Ready = false;
	sub_command = command;
	command = 0;

	switch (sub_command)
	{

	case 0x0: // Command has already been proccessed

		TX("ERORR");
		break;

	case 0x1: // IF Command = Connect
		TX("Connected");
		command = 0;
		comm_p = 0;
		break;

	case 0x2: // IF command = digital output port 1
		DIR_P1();
		TX("2");
		command = 0;
		comm_p = 0;
		break;

	case 0x3:
		OUT_P1();
		TX("3");
		command = 0;
		comm_p = 0;
		break;

	case 0x4:
		REN_P1();
		TX("4");
		command = 0;
		comm_p = 0;
		break;

	case 0x5:
		SEL_P1();
		TX("5");
		command = 0;
		comm_p = 0;
		break;

	case 0x6:
		SEL2_P1();
		TX("6");
		command = 0;
		comm_p = 0;
		break;

	case 0x7:
		DIR_P2();
		TX("7");
		command = 0;
		comm_p = 0;
		break;

	case 0x8:
		OUT_P2();
		TX("8");
		command = 0;
		comm_p = 0;
		break;

	case 0x9:
		REN_P2();
		TX("9");
		command = 0;
		comm_p = 0;
		break;

	case 0xA:
		SEL_P2();
		TX("A");
		command = 0;
		comm_p = 0;
		break;

	case 0xB:
		SEL2_P2();
		TX("B");
		command = 0;
		comm_p = 0;
		break;
	case 0xC:
		PXIN();
		command = 0;
		comm_p = 0;
		break;
	case 0xE:
		SPI_Process_Command();
		command = 0;
		comm_p = 0;
		break;

	default:
		command = 0;
		TX("ERORR");
		 //Error Case

	} // End Main Switch

} // End Sub Command Function

///////////////////////////////////////////


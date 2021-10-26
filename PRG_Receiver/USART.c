#include <iom8.h>
#include <ina90.h>

#include "main.h"


#define USART_RX_BUFFER_SIZE 32     /* 2,4,8,16,32,64,128 or 256 bytes */
#define USART_TX_BUFFER_SIZE 32     /* 2,4,8,16,32,64,128 or 256 bytes */
#define USART_RX_BUFFER_MASK ( USART_RX_BUFFER_SIZE - 1 )
#define USART_TX_BUFFER_MASK ( USART_TX_BUFFER_SIZE - 1 )
#if ( USART_RX_BUFFER_SIZE & USART_RX_BUFFER_MASK )
#error RX buffer size is not a power of 2
#endif
#if ( USART_TX_BUFFER_SIZE & USART_TX_BUFFER_MASK )
#error TX buffer size is not a power of 2
#endif

/* Static Variables */
static unsigned char USART_RxBuf[USART_RX_BUFFER_SIZE];
static volatile unsigned char USART_RxHead;
static volatile unsigned char USART_RxTail;
static unsigned char USART_TxBuf[USART_TX_BUFFER_SIZE];
static volatile unsigned char USART_TxHead;
static volatile unsigned char USART_TxTail;

static volatile unsigned char tx_complete;

#include "USART.h"


/*
Инициализация UART

корость расчитываеться по формуле UBRR = Fosc/(8*BAUD) - 1
*/
void USART_Init( unsigned int baudrate )
{
  unsigned char x;

  UBRRH = (unsigned char) (baudrate>>8);
  UBRRL = (unsigned char) baudrate;

  UCSRA = (1 << U2X);
  /* Enable UART receiver and transmitter */
  UCSRB = ( ( 1 << RXCIE ) | ( 1 << RXEN ) | ( 1 << TXEN ) | ( 1 << TXCIE ));

  /* Set frame format: 8 data 2stop */
  UCSRC = (1<<URSEL)|(1<<USBS)|(1<<UCSZ1)|(1<<UCSZ0);

  /* Flush receive buffer */
  x = 0;

  USART_RxTail = x;
  USART_RxHead = x;
  USART_TxTail = x;
  USART_TxHead = x;
}


void USART_close(void)
{
  UCSRA = 0;
  UCSRB = 0;
  UCSRC = 0;
}

/* Interrupt handlers */
#pragma vector=USART_RXC_vect
__interrupt void USART_RX_interrupt( void )
{
  unsigned char data;
  unsigned char tmphead;

  /* Read the received data */
  data = UDR;
  /* Calculate buffer index */
  tmphead = ( USART_RxHead + 1 ) & USART_RX_BUFFER_MASK;
  USART_RxHead = tmphead;      /* Store new index */

  if ( tmphead == USART_RxTail )
  {
    /* ERROR! Receive buffer overflow */
  }

  USART_RxBuf[tmphead] = data; /* Store received data in buffer */

}

void wait_until_tx_complete(void)
{
  while (tx_complete != 0 );
}


#pragma vector=USART_TXC_vect
__interrupt void USART_TXC_interrupt( void )
{
  if ((UCSRB & (1<<UDRIE)) == 0) tx_complete = 0;
}


#pragma vector=USART_UDRE_vect
__interrupt void USART_TX_interrupt( void )
{
  unsigned char tmptail;

  /* Check if all data is transmitted */
  tmptail = USART_TxTail;
  if ( USART_TxHead != tmptail )
  {
    /* Calculate buffer index */
    tmptail = ( USART_TxTail + 1 ) & USART_TX_BUFFER_MASK;
    USART_TxTail = tmptail;      /* Store new index */

    UDR = USART_TxBuf[tmptail];  /* Start transmition */
  }
  else
  {
    UCSRB &= ~(1<<UDRIE);         /* Disable UDRE interrupt */
  }
}

/* Read and write functions */
unsigned char USART_Receive( void )
{
  unsigned char tmptail;


  tmptail =  USART_RxTail;
  while ( USART_RxHead == tmptail );
  tmptail = ( USART_RxTail + 1 ) & USART_RX_BUFFER_MASK;/* Calculate buffer index */

  USART_RxTail = tmptail;                /* Store new index */

  return USART_RxBuf[tmptail];           /* Return data */
}

void USART_Transmit( unsigned char data )
{
  unsigned char tmphead;
  /* Calculate buffer index */
  tmphead = ( USART_TxHead + 1 ) & USART_TX_BUFFER_MASK; /* Wait for free space in buffer */
  while ( tmphead == USART_TxTail );

  USART_TxBuf[tmphead] = data;           /* Store data in buffer */
  USART_TxHead = tmphead;                /* Store new index */

  UCSRB |= (1<<UDRIE);                   /* Enable UDRE interrupt */
  tx_complete = 1;
}

unsigned char DataInReceiveBuffer( void )
{
  unsigned char tmptail;

  tmptail =  USART_RxTail;
  return ( USART_RxHead != tmptail ); /* Return 0 (FALSE) if the receive buffer is empty */
}

void USART_sendstr(char *strbuf)
{
  while (*strbuf!=0) USART_Transmit(*strbuf++);

}

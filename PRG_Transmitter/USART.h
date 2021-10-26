/* Prototypes */
void USART_Init( unsigned int baudrate );
void USART_close(void);
unsigned char USART_Receive( void );
void USART_Transmit( unsigned char data );
unsigned char DataInReceiveBuffer( void );
void USART_sendstr(char *strbuf);
void wait_until_tx_complete(void);

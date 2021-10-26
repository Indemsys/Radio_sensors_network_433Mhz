/* Prototypes */
void USART_Init( unsigned int baudrate );
unsigned char USART_Receive( void );
void USART_Transmit( unsigned char data );
unsigned char DataInReceiveBuffer( void );
void USART_sendstr(char *strbuf);


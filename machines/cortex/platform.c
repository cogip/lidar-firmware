#include <stdio.h>

#include "clksys.h"
#include "gpio.h"
#include "usart.h"

#include "console.h"
#include "kos.h"
#include "msched.h"
#include "platform.h"

static void mach_pinmux_setup(void)
{
#if defined(__AVR__)
	/* analog to digital conversion */
	PORTA.DIR = 0x00; /*!< PORTA as input pin */
	PORTA.OUT = 0x00;
	PORTA.PIN0CTRL = PORT_ISC_INPUT_DISABLE_gc;

	/* Port B - Jtag disable (fuse bit required) */
	MCU_MCUCR = MCU_JTAGD_bm; /* Fuse4 bit0 to set to 1 with flasher */

	/* twi configuration pin */
	PORTC.DIRSET = PIN1_bm; /*!< PC1 (SCL) as output pin */
	/* usart configuration pin */
	PORTC.DIRCLR = PIN2_bm; /*!< PC2 (RDX0) as input pin */
	PORTC.DIRSET = PIN3_bm; /*!< PC3 (TXD0) as output pin */
#endif
}

void mach_sched_init()
{
	msched_init(10/*ms*/, &TCC0);
}

void mach_sched_run()
{
	kos_run();
}

console_t usart_lidar_console = {
	.usart = &USART_LIDAR,
	.speed = 115200,
};

console_t usart_stm32_console = {
	.usart = &USART_STM32,
	.speed = 115200,
};

uint8_t buff_rx;

void mach_setup(void)
{
#if F_CPU == 32000000UL
	clksys_intrc_32MHz_setup();
#endif

	/* Debug LED pin direction */
	gpio_set_direction(&PORTB, PIN0_bp, GPIO_DIR_OUT);
	gpio_set_output(&PORTB, PIN0_bp, 0);

	/* LIDAR alimentation */
	gpio_set_direction(&PORTD, PIN0_bp, GPIO_DIR_OUT);
	gpio_set_output(&PORTD, PIN0_bp, 1);
	
	/* usart 0 configuration pin - LIDAR data */
	PORTC.DIRCLR = PIN2_bm; /*!< PC3 (RXC0) as input pin */
	PORTC.DIRSET = PIN3_bm; /*!< PC4 (TXC0) as output pin */
	
	/* usart 1 configuration pin - STM32 data */
	gpio_set_output(&PORTC, PIN7_bp, 1);
	gpio_set_direction(&PORTC, PIN7_bp, GPIO_DIR_OUT);
	PORTC.DIRCLR = PIN6_bm; /*!< PC6 (RXC1) as input pin */
	PORTC.DIRSET = PIN7_bm; /*!< PC7 (TXC1) as output pin */
	
	/* UART initialization */
	lidar_console_init(&usart_lidar_console);
	stm32_console_init(&usart_stm32_console);

	/* Global interrupt enable */
	sei();
	
	/*
	 * TODO: Use USARTS in interrupt mode 
	 */

	uint8_t led_state = 0;
	//uint8_t lidar_data = 0;
	//uint8_t stm32_data = 0;

	//for(;;) {
		gpio_set_output(&PORTB, PIN0_bp, led_state);
		led_state = !led_state;
		for (uint32_t i = 0; i < 0x1ffff; i++)
			__asm__ volatile ("nop");
		//dump_lidar();
		//stm32_data = stm32_getchar();
		//if(stm32_data == 0x64) {
		//stm32_putchar(0x64);
		//stm32_putchar(0x99);
		//stm32_putchar(0xBB);
		//	//dump_lidar();
		//}
	//}

	while(1) 
	{
	}

	return;
}

void dump_lidar(void)
{
	//uint8_t values[10];
	uint32_t index = 0;
	uint8_t tmp = 0;
	
	
	//stm32_putchar(0xEE);
	for (index = 0; index < 42; index++) {
		tmp = (uint8_t) lidar_getchar();
		stm32_putchar(tmp);
	}
	stm32_putchar('\n');
}

/* USARTC1 reception interrupt - STM32 Input */
ISR(USARTC1_RXC_vect)
{
	//if (irq_tcc0_ovf_handler)
	//	irq_tcc0_ovf_handler();
	char recvByte;
	gpio_set_output(&PORTB, PIN0_bp, 1);
	
	recvByte = USARTC1.DATA;

	stm32_putchar(recvByte);
	//dump_lidar();
	//stm32_putchar('\n');
	//if (recvByte == 't') {
	////	///dump_lidar();
	//	stm32_putchar('c');
	//} else {
	//	stm32_putchar('t');
	//}
}

/* USARTC1 transmission interrupt - STM32 Output */
ISR(USARTC1_TXC_vect)
{
	//TODO Is it necessary ? 
}

/* USARTC1 reception interrupt - Lidar Input */
ISR(USARTC0_RXC_vect)
{
	//TODO
}
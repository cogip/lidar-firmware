#include <stdio.h>

#include "clksys.h"
#include "gpio.h"
#include "usart.h"

#include "console.h"
#include "kos.h"
#include "msched.h"
#include "platform.h"

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
	
	uint8_t led_state = 0;
	uint8_t lidar_data = 0;
	uint8_t status = 0;
	uint8_t strength_warning = 0;
	uint8_t invalid_data = 0;
	lidar_frame frame = { 0 };

	//for(;;) {
		//gpio_set_output(&PORTB, PIN0_bp, led_state);
		//led_state = !led_state;
		//for (uint32_t i = 0; i < 0x1ffff; i++)
		//	__asm__ volatile ("nop");
		//dump_lidar();
		//stm32_data = stm32_getchar();
		//if(stm32_data == 0x64) {
		//stm32_putchar(0x64);
		//stm32_putchar(0x99);
		//stm32_putchar(0xBB);
		//	//dump_lidar();
		//}
	//}

#if 0
	while(1) 
	{
		lidar_data = lidar_getchar();
		switch(status) {
			case 0:
				if(lidar_data == 0xFA) {
					status = 1;
				}
				break;
			case 1:
				//if(lidar_data >= 0xA0 && lidar_data <= 0xF9) {
				if(lidar_data == 0xA0) {
					frame.index = lidar_data;// - 0xA0;
					status = 2;
					//stm32_putchar(index);
				} else {
					if(lidar_data != 0xFA) {
						status = 0;
					}
				}
				break;
			case 2:
				frame.speed = lidar_data;
				frame.speed <<= 8;
				frame.speed |= lidar_getchar();

				for(int i = 0; i < 4; i++)
				{
					frame.angles[i].distance = lidar_getchar();
					frame.angles[i].distance <<= 8;
					frame.angles[i].distance |= lidar_getchar();

					frame.angles[i].signal_strength = lidar_getchar();
					frame.angles[i].signal_strength <<= 8;
					frame.angles[i].signal_strength |= lidar_getchar();

					strength_warning = 0x40 & (frame.angles[i].distance >> 8);
					strength_warning >>= 6;

					invalid_data = 0x80 & (frame.angles[i].distance >> 8);
					invalid_data >>= 7;

					//frame.angles[i].distance &= ~0xC000;
				}
				frame.checksum = lidar_getchar();
				frame.checksum <<= 8;
				frame.checksum |= lidar_getchar();

				stm32_printf("\nframe:\n");
				stm32_printf("\tindex:0x%X\n", frame.index);
				stm32_printf("\tspeed:0x%X\n", frame.speed);
				for(int i = 0; i < 4; i++) {
					stm32_printf("\tangle[%d]:0x%X - brut: 0x%X\n", (frame.index - 0xA0) * 4 + i, frame.angles[i].distance & ~0xC000, frame.angles[i].distance);
					stm32_printf("\t\tsignal_strength:0x%X, warning:0x%X, invalid_data:0x%X\n", frame.angles[i].signal_strength, strength_warning, invalid_data);
				}
				stm32_printf("\tchecksum:0x%X\t calculated:0x%X\t", frame.checksum, checksum(frame));

				//stm32_putchar(angle1);
				//stm32_putchar(angle2);
				//stm32_putchar(angle3);
				//stm32_putchar(angle4);

				/* TODO:
					* checksum calcul
					* "printfize" those ugly putchar and getchar
					* get angle values
					* Store angle values into an array of 360
				*/

				// Reset status
				status = 0;
				while (1) {};
				break;
			default:
				// Reset status
				status = 0;
				break;
		}
	}
#else
print_raw_lidar_data();
#endif

	return;
}

uint16_t checksum(lidar_frame frame)
{
	uint32_t chk32 = 0;
	uint16_t word;
	uint8_t* data = &frame;
	int i;

	frame.start = 0xFA;

	for(i=0; i<10; i++) { 
		stm32_printf("chks - 0x%X - 0x%X\n", data[2*i], data[2*i+1]);
		word=data[2*i] + (data[2*i+1] << 8);
		chk32 = (chk32 << 1) + word;
	}
	
	uint32_t checksum=(chk32 & 0x7FFF) + (chk32 >> 15);
	return checksum & 0x7FFF;	
}

void print_raw_lidar_data(void)
{
	uint8_t status = 0;
	uint8_t index = 0;
	uint8_t lidar_b1 = 0;
	uint8_t lidar_b2 = 0;
	uint8_t lidar_data = 0;

	while (1) {
		// switch(status) {
		// 	case 0:
		// 		lidar_b1 = lidar_getchar();
		// 		lidar_b2 = lidar_getchar();
		// 		if(lidar_b1 == 0xFA && (lidar_b2 == 0xA0 || lidar_b2 == 0xA1)) {
		// 			stm32_printf("%02X%02X\t", lidar_b1, lidar_b2);
		// 			status = 2;
		// 		}
		// 		break;
		// 	case 1:
		// 		//if(lidar_data >= 0xA0 && lidar_data <= 0xF9) {
		// 		if(lidar_data == 0xA0) {
		// 			stm32_printf("FA%02X\t", lidar_data);
		// 			status = 2;
		// 			//stm32_putchar(index);
		// 		} else {
		// 			if(lidar_data != 0xFA) {
		// 				status = 0;
		// 			}
		// 		}
		// 		break;
		// 	case 2:
		// 		lidar_data = lidar_getchar();
		// 		stm32_printf("%02X", lidar_data);
		// 		index++;
		// 		if(index % 2 == 0) {
		// 			stm32_printf("\t");
		// 		}
		// 		if(index == 26) {
		// 			stm32_printf("\n");
		// 			status = 0;
		// 		}
		// 		break;
		// 	default:
		// 		break;
		// }
		printf("%02X\n", lidar_getchar());
	}
}

void dump_lidar(void)
{
	//uint8_t values[10];
	uint16_t index = 0;
	uint8_t tmp = 'a';
	
	//stm32_putchar(0xEE);
	//for (index = 0; index < 26; index++) {
	//	stm32_putchar(tmp);
	//	tmp++;
	//}
	//stm32_putchar('\n');
	stm32_printf("This is a test i:%d,%d c:%c s:%s !\n", index, 7, 'r', "coucou");
}

/* USARTC1 reception interrupt - STM32 Input */
ISR(USARTC1_RXC_vect)
{
	//if (irq_tcc0_ovf_handler)
	//	irq_tcc0_ovf_handler();
	char recvByte;
	gpio_set_output(&PORTB, PIN0_bp, 1);
	
	recvByte = USARTC1.DATA;

	//stm32_putchar(recvByte);
	//dump_lidar();
	//stm32_putchar('\n');
	if (recvByte == 't') {
	    dump_lidar();
	}
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
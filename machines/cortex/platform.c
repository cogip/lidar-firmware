#include <stdio.h>

#include "clksys.h"
#include "gpio.h"
#include "usart.h"

#include "console.h"
#include "hwtimer.h"
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
	
	/* Setup PWM to manage rotation speed of the lidar */
	timer_pwm_mode_setup(&TCD0, 200, TC_CLKSEL_DIV8_gc);
	timer_pwm_enable(&TCD0, PIN0_bp);

	/* We want it at 300rpm - full modulation (255) = 480rpm -> 300rpm = 155 */
	timer_pwm_duty_cycle(&TCD0, PIN0_bp, 155);

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
	uint8_t data = 0;
	uint8_t status = 0;
	uint8_t strength_warning = 0;
	uint8_t invalid_data = 0;
	uint8_t frame[22];

	is_reading = FALSE;
	distance[10] = 10;

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

#if 1
	while(1) 
	{
        switch(status){
            case 0:
                data = lidar_getchar();
                if(data == 0xFA) {
                    //printf("%02X\n", data);
                    if(is_reading == FALSE)
                         status = 1;
                }
                break;
            case 1:
                data = lidar_getchar();
                if(data >= 0xA0 && data <= 0xF9) {
                //if(data == 0xD1) {
                    //printf("FA\n");
                    //printf("%02X\n", data);
                    frame[0] = 0xFA;
                    frame[1] = data;
                    status = 2;
                } else {
                    status = 0;
                }
                break;
            case 2:
                for(uint8_t i = 0; i < 20; i++) {
                    data = lidar_getchar();
                    frame[i+2] = data;
                    //printf("%02X\n", data);
                }

                //for(int i = 0; i < 22; i++) {
                //    printf("%02X\n", frame[i]);
                //}
                uint16_t crc_frame = frame[20] + (frame[21] << 8);
                if(crc_calc(frame) == crc_frame) {
                    //printf("crc OK !\n");
                    uint16_t speed = frame[2] + (frame[3] << 8);
                    //stm32_printf("speed: %04X - %u - %u tr/min\n", speed, speed, speed / 60);
                    //uint16_t distance[4];
                    for(uint8_t i = 1; i < 5; i++) {
                        //printf("dist: %02X - bit7: %02X\n", frame[4*i+1], frame[4*i+1] & 0x80);
                        if((frame[4*i+1] & 0x80) == 0x80) {
                            //stm32_printf("No angle - trame: %02X\n", frame[1]);
                            //stm32_printf("x0: %02X - x1: %02X - x2: %02X - x3: %02X\n", frame[i*4], frame[4*i+1], frame[4*i+2], frame[4*i+3]);
                        } else {
                            //stm32_printf("Angle ok - trame: %02X\n", frame[1]);
                            uint8_t tmp = frame[4*i+1] & 0x3F;
                            uint16_t dist = frame[4*i] + (tmp << 8);
                            uint16_t index = frame[1] - 0xA0;
                            index = index * 4 + (i - 1);
                            distance[index] = dist;
                            if(index == 0)
                                stm32_printf("0: %u\n", distance[0]);
                            //stm32_printf("b0: %02X - b1: %02X - tmp: %02X - dist: %04X -> %u\n", frame[i*4], frame[4*i+1], tmp, dist, dist);
                        }
                    }
                } else {
                    //stm32_printf("crc KO ! - trame: %02X\n", frame[1]);
                }
                //exit(0);
                status = 0;
                break;
            default:
                break;
        }
    }
#else
print_raw_lidar_data();
#endif

	return;
}

uint16_t crc_calc(unsigned char frame[]) {
    uint32_t chk32 = 0;
    uint16_t word = 0;

    for(uint8_t i = 0; i < 10; i++) {
        word = frame[2*i] + (frame[2*i+1] << 8);
        chk32 = (chk32 << 1) + word;
    }

    uint32_t checksum = (chk32 & 0x7FFF) + (chk32 >> 15);

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
	char recvByte;
	
	recvByte = USARTC1.DATA;

	if (recvByte == 'd') {
		is_reading = TRUE;
		stm32_printf("\nDUMP Distance array:\n");
		for(uint16_t i=0; i<360; i++) {
			stm32_printf("%u\t", distance[i]);
			if(i % 10 == 0 && i != 0)
				stm32_printf("\n");
		}
		is_reading = FALSE;
	}
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
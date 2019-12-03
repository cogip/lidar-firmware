#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdio.h>

#include "usart.h"

typedef struct {
	usart_t *usart;
	uint32_t speed;
} console_t;

#define printf_ratelimited(...) \
do { \
	static uint8_t _cpt = (25); \
\
	if (! --_cpt) { \
		_cpt = (25); \
		printf(__VA_ARGS__); \
	} \
} while(0)

#if defined(__AVR__)
#define cons_printf printf
#define cons_scanf scanf
#else
#define cons_printf printf
int cons_scanf(const char *fmt, ...);
#endif

/* LIDAR USART */
int lidar_getchar();

int lidar_putchar(char c);

int lidar_is_data_arrived();

void lidar_console_init(console_t *con);

/* STM32 USART */
int stm32_getchar();

int stm32_putchar(char c);

int stm32_is_data_arrived();

void stm32_console_init(console_t *con);

#endif /* CONSOLE_H */

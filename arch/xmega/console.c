#include <stdio.h>

#include "console.h"
#include "usart.h"
#include "kos.h"

static int lidar_uart_putchar(char c, FILE *stream);
static int lidar_uart_getchar(FILE *stream);

static int stm32_uart_putchar(char c, FILE *stream);
static int stm32_uart_getchar(FILE *stream);

static FILE lidar_uart_fdstream = FDEV_SETUP_STREAM(lidar_uart_putchar, lidar_uart_getchar,
					      _FDEV_SETUP_RW);

static FILE stm32_uart_fdstream = FDEV_SETUP_STREAM(stm32_uart_putchar, stm32_uart_getchar,
					      _FDEV_SETUP_RW);


static console_t *lidar_console = NULL;
static console_t *stm32_console = NULL;

static int lidar_uart_putchar(char c, FILE *stream)
{
	if (lidar_console) {
		if (c == '\n')
			lidar_uart_putchar('\r', stream);

		usart_send(lidar_console->usart, c);
		return 1;
	}
	return 0;
}

static int lidar_uart_getchar(FILE *stream)
{
	if (lidar_console) {
		return usart_recv(lidar_console->usart);
	}

	return -1;
}


static int stm32_uart_putchar(char c, FILE *stream)
{
	if (stm32_console) {
		if (c == '\n')
			stm32_uart_putchar('\r', stream);

		usart_send(stm32_console->usart, c);
		return 1;
	}
	return 0;
}

static int stm32_uart_getchar(FILE *stream)
{
	if (stm32_console) {
		return usart_recv(stm32_console->usart);
	}

	return -1;
}

/* LIDAR USAR MGMT */

int lidar_putchar(char c)
{
	return lidar_uart_putchar(c, &lidar_uart_fdstream);
}

int lidar_getchar()
{
	return lidar_uart_getchar(&lidar_uart_fdstream);
}

int lidar_is_data_arrived()
{
	return lidar_console ? usart_is_data_arrived(lidar_console->usart) : -1;
}

void lidar_console_init(console_t *con)
{
	lidar_usart_setup(con->usart, con->speed);

	if (!lidar_console) {
			lidar_console = con;

			stdout = &lidar_uart_fdstream;
			stdin = &lidar_uart_fdstream;
	}
}

/* STM32 USART MGMT */

int stm32_putchar(char c)
{
	return stm32_uart_putchar(c, &stm32_uart_fdstream);
}

int stm32_getchar()
{
	return stm32_uart_getchar(&stm32_uart_fdstream);
}

int stm32_is_data_arrived()
{
	return stm32_console ? usart_is_data_arrived(stm32_console->usart) : -1;
}

void stm32_console_init(console_t *con)
{
	stm32_usart_setup(con->usart, con->speed);

	if (!stm32_console) {
		stm32_console = con;

		stdout = &stm32_uart_fdstream;
		stdin = &stm32_uart_fdstream;
	}
}

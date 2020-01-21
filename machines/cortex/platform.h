#ifndef PLATFORM_H_
#define PLATFORM_H_

#include <stdint.h>

#define USART_LIDAR			USARTC0
#define USART_STM32			USARTC1

typedef struct {
    uint16_t distance;
    uint16_t signal_strength;
} angle_data;

typedef struct {
    uint8_t start;
    uint8_t index;
    uint16_t speed;
    angle_data angles[4];
    uint16_t checksum;
} lidar_frame;

void mach_setup(void);

void mach_tasks_init();
void mach_sched_init();
void mach_sched_run();

void dump_lidar();
uint16_t checksum(lidar_frame frame);

#endif /* PLATFORM_H_ */

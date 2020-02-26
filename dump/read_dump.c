#include <stdio.h>
#include <stdlib.h>

#include "read_dump.h"

#define DUMP_FILE "lidar.dump"
FILE* dump_file;
int status = 0;

int main(void) {
    printf("This program will read lidar.dump in the current directory and parse it as lidar data.\n");
    printf("Thus, it will be easier to test required algorithm for lidar's real use case\n");

    char data_str[3];
    unsigned char data;

    dump_file = fopen(DUMP_FILE, "r");

    read_lidar();

    fclose(dump_file);
}

unsigned char get_data() {
    char data_str[3];

    fscanf(dump_file, "%s\n", data_str);

    return (unsigned char)strtol(data_str, NULL, 16);
}

void read_lidar() {

    unsigned char data;
    unsigned char frame[22];

    while(1) {
        switch(status){
            case 0:
                data = get_data();
                if(data == 0xFA) {
                    //printf("%02X\n", data);
                    status = 1;
                }
                break;
            case 1:
                data = get_data();
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
                for(int i = 0; i < 20; i++) {
                    data = get_data();
                    frame[i+2] = data;
                    //printf("%02X\n", data);
                }

                //for(int i = 0; i < 22; i++) {
                //    printf("%02X\n", frame[i]);
                //}
                u_int16_t crc_frame = frame[20] + (frame[21] << 8); 
                if(crc_calc(frame) == crc_frame) {
                    //printf("crc OK !\n");
                    u_int16_t speed = frame[2] + (frame[3] << 8);
                    //printf("speed: %04X - %u - %u tr/min\n", speed, speed, speed / 60);
                    u_int16_t distance[4];
                    for(int i = 1; i < 5; i++) {
                        //printf("dist: %02X - bit7: %02X\n", frame[4*i+1], frame[4*i+1] & 0x80);
                        if((frame[4*i+1] & 0x80) == 0x80) {
                            //printf("No angle - trame: %02X\n", frame[1]);
                        } else {
                            printf("Angle ok - trame: %02X\n", frame[1]);
                            unsigned char tmp = frame[4*i+1] & 0x3F;
                            u_int16_t dist = frame[4*i] + (tmp << 8);
                            printf("b0: %02X - b1: %02X - tmp: %02X - dist: %04X -> %u\n", frame[i*4], frame[4*i+1], tmp, dist, dist);

                        }
                    }

                } else {
                    printf("crc KO ! - trame: %02X\n", frame[1]);
                }
                //exit(0);
                status = 0;
                break;
            default:
                break;
        }
    }
}

u_int16_t crc_calc(unsigned char frame[]) {
    u_int32_t chk32 = 0;
    u_int16_t word = 0; 

    for(int i = 0; i < 10; i++) {
        word = frame[2*i] + (frame[2*i+1] << 8);
        chk32 = (chk32 << 1) + word;
    }

    u_int32_t checksum = (chk32 & 0x7FFF) + (chk32 >> 15);
    
    return checksum & 0x7FFF;
}
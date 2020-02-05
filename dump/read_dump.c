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

    while(1) {
        switch(status){
            case 0:
                data = get_data();
                if(data == 0xFA) {
                    printf("%02X\n", data);
                    status = 1;
                }
                break;
            case 1:
                data = get_data();
                if(data == 0xA0) {
                    printf("%02X\n", data);
                    status = 2;
                }
                break;
            case 2:
                for(int i = 0; i < 20; i++) {
                    data = get_data();
                    printf("%02X\n", data);
                }
                exit(0);
                break;
            default:
                break;
        }
    }
}
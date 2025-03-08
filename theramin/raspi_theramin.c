
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>


#define TOF1020_DEV "/dev/tof1020"

int main(int argc, char const *argv[])
{
    int tof1020file = open(TOF1020_DEV, O_RDONLY, 0644);
    uint16_t val;
    if (tof1020file < 0) {
        fprintf(stderr, "Failed to open %s\n",TOF1020_DEV);
        return -1;
    }
    
    if (pread(tof1020file, &val,sizeof(val),0) == sizeof(val)) {
        printf("Read from ToF value: 0x%x\n",val);
    } else {
        fprintf(stderr, "Failed to read 2 bytes from %s\n",TOF1020_DEV);
    }
}
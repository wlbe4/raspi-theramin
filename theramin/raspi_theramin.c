
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <alsa/asoundlib.h>

#define TOF1020_DEV "/dev/tof1020"

// Yamaha Clavinova USB ID 0499:1613
// amidi -l
// Dir Device    Name
// IO  hw:0,0,0  Clavinova MIDI 1
#define MIDI_PORT_NAME "hw:0,0,0"

int main(int argc, char const *argv[])
{
    printf("Build on %s %s\n",__DATE__, __TIME__);
    int tof1020file = open(TOF1020_DEV, O_RDONLY, 0644);
    int status;
    int mode = SND_RAWMIDI_SYNC;
    snd_rawmidi_t * midiout = NULL;
    const char * portname = MIDI_PORT_NAME;
    uint16_t val;

    if (tof1020file < 0) {
        fprintf(stderr, "Failed to open %s\n",TOF1020_DEV);
        return -1;
    }

    /** MIDI SETUP */
    if ((status = snd_rawmidi_open(NULL, &midiout, portname, mode)) < 0) {
        fprintf(stderr, "Failed to open MIDI: %s\n",snd_strerror(status));
        return -1;
    }
    printf("MIDI port %s was open successfuly\n", portname);

    if (pread(tof1020file, &val,sizeof(val),0) == sizeof(val)) {
        printf("Read from ToF value: 0x%x\n",val);
    } else {
        fprintf(stderr, "Failed to read 2 bytes from %s\n",TOF1020_DEV);
    }

    // Close MIDI device
    if ((status = snd_rawmidi_close(midiout)) < 0) {
        fprintf(stderr, "Failed to close MIDI: %s\n",snd_strerror(status));
        return -1;
    }
}
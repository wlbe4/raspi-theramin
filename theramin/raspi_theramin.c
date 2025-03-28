
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <alsa/asoundlib.h>
#include "pitch_to_note.h"

#define TOF1020_DEV "/dev/tof1020"
// Yamaha Clavinova USB ID 0499:1613
// amidi -l
// Dir Device    Name
// IO  hw:0,0,0  Clavinova MIDI 1
#define MIDI_PORT_NAME "hw:0,0,0"
typedef struct {
    char cmd[3];
}midiEventPacket_t;

#define MAX_TOF_VAL     0x180
#define MOUT_CH         2 // MIDI 0-15 is mapped to device midi channel 1-16
#define NORM_VELOCITY   64
const char notes_to_play[] = {
    pitchC3, pitchD3, pitchE3, pitchF3, pitchG3,
    pitchA3, pitchB3, pitchC4
};
#define TOTAL_NOTES (sizeof(notes_to_play) / sizeof(notes_to_play[0]))

// The channel range is 0-15
// The control range is 0-119.
// The value range is 0-127.
int controlChange(snd_rawmidi_t * midiout, char channel, char control, char value) {
    midiEventPacket_t event = {0xB0 | channel, control, value};
    return snd_rawmidi_write(midiout, &event, 3);
}

// MIDI Stuff
// First parameter is the event type (0x09 = note on, 0x08 = note off).
// Second parameter is note-on/note-off, combined with the channel.
// Channel can be anything between 0-15. Typically reported to the user as 1-16.
// Third parameter is the note number (48 = middle C).
// Fourth parameter is the velocity (64 = normal, 127 = fastest).

int noteOn(snd_rawmidi_t * midiout, char channel, char pitch, char velocity) {
    midiEventPacket_t noteOn = {0x90 | channel, pitch, velocity};
    return snd_rawmidi_write(midiout, &noteOn, 3);
}

int noteOff(snd_rawmidi_t * midiout, char channel, char pitch, char velocity) {
    midiEventPacket_t noteOff = {0x80 | channel, pitch, velocity};
    return snd_rawmidi_write(midiout, &noteOff, 3);
}

void play_note(snd_rawmidi_t * midiout, char note)
{
    int status;
    // char note = (char)notePitch[idx];
    if ( (status = noteOn(midiout, MOUT_CH, note, NORM_VELOCITY)) < 0 ) {
        fprintf(stderr, "Failed to send MIDI noteOn command\n");
    }
    usleep(100*1000);
    if ( (status = noteOff(midiout, MOUT_CH, note, NORM_VELOCITY)) < 0 ) {
        fprintf(stderr, "Failed to send MIDI noteOn command\n");
    }
}

void play_val(snd_rawmidi_t * midiout, uint16_t val, uint16_t max_val)
{
    int note_idx = 0 + (TOTAL_NOTES * val / max_val);
    char note = notes_to_play[note_idx];
    play_note(midiout, note);
}

int main(int argc, char const *argv[])
{
    printf("Build on %s %s\n",__DATE__, __TIME__);
    int tof1020file = open(TOF1020_DEV, O_RDONLY, 0644);
    int status;
    int mode = SND_RAWMIDI_SYNC;
    snd_rawmidi_t * midiout = NULL;
    const char * portname = MIDI_PORT_NAME;
    uint16_t val = 1;
    uint16_t prev_val = 1;
    if (tof1020file < 0) {
        fprintf(stderr, "Failed to open %s\n",TOF1020_DEV);
        return -1;
    }
    /** MIDI SETUP */
    if ((status = snd_rawmidi_open(NULL, &midiout, portname, mode)) < 0) {
        fprintf(stderr, "Failed to open MIDI: %s\n",snd_strerror(status));
        return -1;
    }

    while (val) {
        if (pread(tof1020file, &val,sizeof(val),0) == sizeof(val)) {
            if (val == -1) {
                val = 1;
                continue;
            }
            if (abs(prev_val-val) > 50) {
                printf("Read from ToF value: 0x%x\n",val);
                prev_val = val;
                if (val < MAX_TOF_VAL) {
                    play_val(midiout, val, MAX_TOF_VAL);
                }
            }
        } else {
            fprintf(stderr, "Failed to read 2 bytes from %s\n",TOF1020_DEV);
        }
        usleep(50*1000); // Sample distance with 20Hz
    }

    // Close MIDI device
    if ((status = snd_rawmidi_close(midiout)) < 0) {
        fprintf(stderr, "Failed to close MIDI: %s\n",snd_strerror(status));
        return -1;
    }    
}
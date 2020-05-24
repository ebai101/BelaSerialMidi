/*
This is an example program for the SerialMidi class.
To use this, you'll need to connect a MIDI device to a free UART port on the Bela. This might require you to wire up a MIDI input circuit if you want to use a 5-pin DIN connector.

If you want to test the code in a more straightforward manner, you could program a microcontroller (Teensy, Arduino, etc) to output simple MIDI messages on one of its serial ports, and connect that to the UART port on Bela.

More info can be found in this thread: https://forum.bela.io/d/620-bela-mini-midi-input-on-uart0
*/

#include <Bela.h>
#include <libraries/Midi/Midi.h>
#include "SerialMidi.hpp"

Midi midi;
SerialMidi sm;

// print out any incoming MIDI messages to the terminal
void midiCallback (MidiChannelMessage message, void *arg)
{
	message.prettyPrint();
}

bool setup(BelaContext *context, void *userData)
{
    // set up the MIDI device normally
    // the readFrom port should be defined, but it does NOT correspond to the serial port the SerialMidi instance will read from
	midi.readFrom ("hw:1,0,0");
    midi.enableParser (true);
    midi.setParserCallback (midiCallback);

    // initialize the SerialMidi reader
    // this sets it up to feed into the midi device we just initialized. the arguments `dev` and `bd` are supplied the default values.
    // UART4 (/dev/ttyS4) is the default serial port, and 31250 the default baud rate
	sm.setup (&midi);

    // here's how you can override the defaults:
    // sm.setup (&midi, dev="/some/device", bd=12345);

	return true;
}

void render(BelaContext *context, void *userData)
{
    // no code is needed in render, the parser runs in an AuxiliaryTask.

	// Audio passthrough, just a placeholder
	for(unsigned int n = 0; n < context->audioFrames; n++) {
		for(unsigned int ch = 0; ch < 2; ch++){
			audioWrite(context, n, ch, audioRead(context, n, ch));
		}
	}
}

void cleanup(BelaContext *context, void *userData)
{
	Bela_stopAllAuxiliaryTasks();

    // don't forget to run this, otherwise the file descriptor won't be closed
	sm.stopRead();
}
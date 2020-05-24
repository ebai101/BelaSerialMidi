# BelaSerialMidi

This class reads from a UART port on the Bela and passes all traffic to a MidiParser instance. It's a simple and straightforward way to integrate hardware MIDI input into a Bela project. Only MIDI input is implemented at the moment, but ouput would be fairly straightforward to implement.

Refer to `example-render.cpp` for usage.
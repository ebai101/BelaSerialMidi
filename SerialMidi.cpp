/*
* SerialMidi.cpp
* Created by Ethan Bailey, 03/14/20
*/

#include "SerialMidi.hpp"

void SerialMidi::setup (Midi *m, const char dev[], int bd)
{
    _midi = m;
    _serial_fd.fd = setupSerialDevice (dev, bd);
    _serial_fd.events = POLLIN;
    startRead();
}

int SerialMidi::setupSerialDevice (const char dev[], int bd)
{
    /*
    derived from this code:
    https://gist.githubusercontent.com/peterhurley/fbace59b55d87306a5b8/raw/220cfc2cb1f2bf03ce662fe387362c3cc21b65d7/anybaud.c
    */

    int fd;
    struct termios2 tio;

    // open device for read
    fd = open(dev, O_RDONLY | O_NOCTTY | O_ASYNC);

    //if can't open file
    if (fd < 0) {
        //show error and exit
        perror(dev);
        return (-1);
    }

    if (ioctl(fd, TCGETS2, & tio) < 0)
        perror("TCGETS2 ioctl");

    tio.c_cflag &= ~CBAUD;
    tio.c_cflag |= BOTHER | CS8 | CLOCAL | CREAD; // Baud rate, 8N1, local modem, receive chars
    tio.c_iflag = IGNPAR; // ignore parity errors
    tio.c_oflag = 0; // raw output
    tio.c_lflag = 0; // non-canonical
    tio.c_cc[VTIME] = 0; // don't use inter-char timer
    tio.c_cc[VMIN] = 1; // block read until 1 char arrives
    tio.c_ispeed = bd;
    tio.c_ospeed = bd;

    if (ioctl(fd, TCSETS2, & tio) < 0)
        perror("TCSETS2 ioctl");

    return fd;
}

void SerialMidi::readSerialDevice()
{
    midi_byte_t buf[3];
	
    // read midi message
    while (!gShouldStop)
    {
        // 1 second timeout on MIDI messages
        poll (&_serial_fd, 1, 1000);
        if (_serial_fd.revents & POLLIN)
        {
            int i = 1;
            while (i < 3)
            {
                read(_serial_fd.fd, buf + i, 1);
                if (buf[i] >> 7 != 0) {
                    // status byte
                    buf[0] = buf[i];
                    i = 1;
                } else {
                    // data byte
                    if (i == 2) {
                        i = 3;
                    } else {
                        // if the message is a program change or mono key pressure, it only uses 2 bytes
                        if ((buf[0] & 0xF0) == 0xC0 || (buf[0] & 0xF0) == 0xD0)
                            i = 3;
                        else
                            i = 2;
                    }
                }
            }

            // send message to the parser
            // could this be done with a realtime pipe?
            _midi->getParser()->parse(buf, 3);
        }
    }
}

/*
Bela aux task callback for serial reads
*/
void readTask (void *arg)
{
    SerialMidi *sm = (SerialMidi *) arg;
    sm->readSerialDevice();
}

void SerialMidi::startRead()
{
    Bela_scheduleAuxiliaryTask (Bela_createAuxiliaryTask (readTask, 85, "serial MIDI", this));
}

void SerialMidi::stopRead()
{
    close(_serial_fd.fd);
}
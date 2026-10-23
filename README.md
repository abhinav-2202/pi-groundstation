# Pi Ground Station: ADS-B Receiver from Raw I/Q

A software-defined receiver for aircraft ADS-B broadcasts at 1090 MHz, written from scratch.
It takes raw I/Q samples from an RTL-SDR recording and turns them into live flight
information: aircraft identity, callsign, altitude, speed, heading and position.

No SDR or ADS-B decoding libraries are used. Every step, from pulse detection to
position decoding, is implemented in this repository.

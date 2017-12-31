GOES xRIT Demodulator
=========================================

This is a GNU Radio Flow for Demodulating BPSK signals from GOES LRIT. 
The flow contains alternatives for Airspy R2 (default), Airspy Mini, GQRX Record and RTLSDR. 
The Base processing Sample Rate is 1msps, so you can adapt the flow to get your favorite SDR IQ to fit 1msps at 1691MHz. Have fun!

The demodulator application works for both HRIT / LRIT (set at compile time) and it's adapted for Airspy R2 and Airspy Mini

![Screenshot](demodulator.png)

## Dependencies

* [librtlsdr](https://github.com/librtlsdr/librtlsdr)
* [libairpy](https://github.com/airspy/airspyone_host/tree/master/libairspy)
* [libusb](https://github.com/libusb/libusb)
* [hackrf](https://github.com/mossmann/hackrf/tree/master/host)

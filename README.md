# QR code generator

IDE used to test concept/logic: Netbeans - will probably move to Eclipse for ESP8266 port.

Protoype involves getting something barebones working first, then optimising. Not using C QR libraries that exist - see references.
Developing in C for PC first using limited libraries then will port for esp8266 libraries available - I have built the ESP8266 toolchain on my machine, but debugging not configured yet, only capable of generating binaries for the ESP8266. I used the esp-open-sdk for my testing.

Current solution limited to L level error correction and for byte mode only. Also, only up to version 13 - no block splitting. My general approach is to verify basic functionality then build and expand code capability.

###### Optimisations:
- Memory usage - use actual bits instead of bytes to represent byte stream. This was a problem I faced a some time ago when implementing AX25 communications protocol on an STM32F4. The housekeeping is a bit tedious but worthwhile when limited RAM available.
- Reed Solomon polynomial generator can be implemented as a table instead of algorithm if sufficient space available on embedded system.

Due to time constraints I could not produce the work as I would normally do. Please see some of my other coding projects to get an idea of how I code for abstraction here:
https://github.com/rddyas002  
In particular see https://github.com/rddyas002/MPLABX_ThermalControl which was a big embedded system project - I modified the baseline for thermal control here, but I originally developed it for the helicopter control system - so drivers are for the sensors/actuators.

###### References:
Very good libraries that are tried and tested. For a product implementation, best to port this code for ESP8266 and verify.  
C++ library: https://github.com/nayuki/QR-Code-generator/tree/master/cpp  
c library: https://github.com/fukuchi/libqrencode  
Get going: http://hackaday.com/2015/03/18/how-to-directly-program-an-inexpensive-esp8266-wifi-module/  
https://github.com/pfalcon/esp-open-sdk

Add path to env:  
export PATH=$PATH:/home/yashren/NetBeansProjects/qrcoder/esp-open-sdk/xtensa-lx106-elf/bin






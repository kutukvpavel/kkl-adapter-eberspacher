# kkl-adapter-eberspacher

An implementation of Eberspacher K-Line diagnostics adapter modchip, as described in
https://t4-wiki.de/wiki/Diagnoseinterface_fuer_Standheizungen and https://www.drive2.ru/b/493589462480060493/

I used ATtiny13A microcontroller; the code can be compiled with avr-gcc 10 and make.

In short: this chip is meant to pull K-Line to ground for a certain amount of time, when RTS serial port line is asserted 
(seconds, depends on the number of RTS assertions).
I used an external NPN transistor as a current buffer.

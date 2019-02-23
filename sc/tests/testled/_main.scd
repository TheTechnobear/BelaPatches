/*******************
Blink a LED
Connect a LED+resistor to digital pin 0

DigitalOut takes three arguments:
ar (digitalPin: 0, output: 0, writeMode: 0, mul: 1, add: 0)

digitalPin:  Digital pin number to write to. Pin numbers begin at 0. This value cannot be modulated.
Pin numbers are labled in the Pin Diagram tab in the Bela IDE.

output:  Value to write out to the pin - the value will be 1 when the argument is larger than 0, otherwise 0.

writeMode:	Mode of writing to the output, this can be 0 (only when value changes) or 1 (continuously, at audio rate). This value cannot be modulated.

(c) 2017: Jonathan Reus, Marije Baalman, Giulio Moro, Andrew McPherson
*/

s = Server.default;

s.options.numAnalogInChannels = 2;
s.options.numAnalogOutChannels = 2;
s.options.numDigitalChannels = 16;
s.options.maxLogins = 16;  	   // set max number of clients

s.options.blockSize = 16;
s.options.numInputBusChannels = 2;
s.options.numOutputBusChannels = 2;

s.waitForBoot{
	SynthDef('blinker', {
    	var pwm = SinOsc.ar(freq:0.1,mul:0.5,add:0.5);
		// DigitalOut.ar(digitalPin:7, output:LFPulse.ar( freq:(44100/32), width: pwm), writeMode:1);
		DigitalIO.ar(digitalPin:7, pinMode:1, output:LFPulse.ar( freq:(44100/32), width:0.75));
		DigitalIO.ar(digitalPin:2, pinMode:1, output:LFPulse.kr(freq:0.3));
		DigitalIO.ar(digitalPin:4, pinMode:1, output:LFPulse.ar(freq:0.4));
		DigitalIO.kr(digitalPin:8, pinMode:1, output:LFPulse.kr(freq:0.2));
		DigitalIO.kr(digitalPin:9, pinMode:1, output:0);
		}).add;
	
	s.sync;
	
	a = Synth('blinker', );
};


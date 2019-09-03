s = Server.default;

s.options.numAnalogInChannels = 8;
s.options.numAnalogOutChannels = 8;
s.options.numDigitalChannels = 16;

s.options.blockSize = 16;
s.options.numInputBusChannels = 2;
s.options.numOutputBusChannels = 2;

s.options.postln;

s.waitForBoot({

"Server Booted".postln;
s.sync;

~leds = [6,7,10,2,3,0,1,4,5,8];
~digitalIn = [11,9,13,12];
~buttonIn = [15,14];

~gain = 6;

~ledCheck = {
	DigitalOut.kr(~leds[0],DigitalIn.kr(~digitalIn[0]));
	DigitalOut.kr(~leds[1],DigitalIn.kr(~digitalIn[1]));
	DigitalOut.kr(~leds[2],DigitalIn.kr(~digitalIn[2]));
	DigitalOut.kr(~leds[3],DigitalIn.kr(~digitalIn[3]));

	DigitalOut.kr(~leds[8],DigitalIn.kr(~buttonIn[0]));
	DigitalOut.kr(~leds[9],DigitalIn.kr(~buttonIn[1]));
}.play;

SynthDef(\kick, {|out = 0|
var pan = 0;
var amp = AnalogIn.kr(0) * ~gain;
var g = DigitalIn.kr(~digitalIn[0]) + DigitalIn.kr(~buttonIn[0]);
var env = EnvGen.kr(Env.perc(0.001, 0.2, 1, -4), g, doneAction:0);
var sig = SinOsc.ar(80) + Crackle.ar(1, 0.5);
Out.ar(out, Pan2.ar(sig*env, pan, amp));
}).add;

~kick=Synth(\kick,[\out,0]);


SynthDef(\snare, {|out = 0|
var pan = 0;
var amp = AnalogIn.kr(2) * ~gain;
var g = DigitalIn.kr(~digitalIn[1]) + DigitalIn.kr(~buttonIn[1]);
var env = EnvGen.kr(Env.perc(0.001, 0.1, 1, -5), g, doneAction:0);
var sig = SinOsc.ar(120) - WhiteNoise.ar(0.5, 0.5);
Out.ar(out, Pan2.ar(sig*env, pan, amp));
}).add;

~snare=Synth(\snare,[\out,0]);


SynthDef(\hat, {|out = 0|
var pan = 0;
var amp = AnalogIn.kr(4) * 4 * ~gain;
var g = DigitalIn.kr(~digitalIn[2]);
var env = EnvGen.kr(Env.perc(0.002, 0.3, 1, -2), g, doneAction:0);
var sig = Klank.ar(`[ 	[ 6563, 9875 ],
						[ 0.6, 0.5 ],
						[ 0.002, 0.003] 
						], PinkNoise.ar(1));
Out.ar(out, Pan2.ar(sig*env, pan, amp));
}).add;

~hat=Synth(\hat,[\out,0]);


SynthDef(\tom, {|out = 0|
var pan = 0;
var amp = AnalogIn.kr(6) * ~gain;
var g = DigitalIn.kr(~digitalIn[3]);
var env = EnvGen.kr(Env.perc(0.001, 0.1, 1, -5), g, doneAction:0);
var sig = SinOsc.ar(440); 
Out.ar(out, Pan2.ar(sig*env, pan, amp));
}).add;

~tom=Synth(\tom,[\out,0]);



});

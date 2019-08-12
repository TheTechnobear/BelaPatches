/*

Granular processor (w/ some LFOs)
based on orignal patch by  Hyppasus : https://github.com/Hyppasus/supercollider-eurobela


CV/Pots
-------
1: Start
2: Duration 
3: Rate
4: Density

Trigs/Switches
--------------
1 (S) 	: record
1 (T) 	: record
2 		:
3 		:
4 		:

LED
--------------
1: trig
2:
3:
4:

Audio Input
-----------
1 : live input
2 : 

Audio Output
------------
1 : live output
2 :

CV Outputs
----------
1:
2:
3:
4: 
5: 
6:
7:
8:

Trig Outputs
------------
1:
2:
3:
4:


Notes:
length and density of grains increase CPU dramatically, you will loose connection to GUI if you set to max (but patch will continue to run)
*/



s = Server.default;

s.options.numAnalogInChannels = 8; // can be 2, 4 or 8
s.options.numAnalogOutChannels = 8; // can be 2, 4 or 8
s.options.numDigitalChannels = 16;
s.options.maxLogins = 4;
s.options.sampleRate = 44100;

s.options.pgaGainLeft = 0;     // sets the pga gain of the left channel to 0 dB
s.options.pgaGainRight = 0;    // sets the pga gain of the right channel to 0 dB
s.options.headphoneLevel = 0; // sets the headphone level to 0 dB
s.options.speakerMuted = 1;    // the speakers are muted
s.options.dacLevel = 0;       // sets the gain of the dac to 0 dB
s.options.adcLevel = 0;       // sets the gain of the adc to 0 dB
s.options.numMultiplexChannels = 0; // do not enable multiplexer channels
s.options.belaPRU = 0;         // select the PRU on which Bela audio will run

s.options.blockSize = 16;
s.options.numInputBusChannels = 2;
s.options.numOutputBusChannels = 2;

// s.options.numAudioBusChannels = 1024;
// s.options.memSize = 8192;
// s.options.memSize = 8192 * 16; // the default is 8192 which is quite small


s.waitForBoot{

"Server Booted".postln;



// input
~sw_in=6;
~t1_in=15;
~t2_in=14;
~t3_in=1;
~t4_in=3;

// output
~led_pwm=7;
~led1=2;
~led2=4;
~led3=8;
~led4=9;

~t1_out=0;
~t2_out=5;
~t3_out=12;
~t4_out=13;

~pots_low = 0.0;
~pots_high = 1.0;

~t2_state = 1.0;

Ndef(\led_pwm,{DigitalIO.ar(~led_pwm, pinMode:1, output:LFPulse.ar( freq:(44100/32), width: 0.5))});

/*
Buffer.alloc(s, s.sampleRate * 4, 1, bufnum:10);
Buffer.alloc(s, s.sampleRate * 2, 1, bufnum:10);
Buffer.alloc(s, s.sampleRate * 1, 1, bufnum:10);
Buffer.alloc(s, s.sampleRate * 0.5, 1, bufnum:10);
*/
Buffer.alloc(s, s.sampleRate * 1, 1, bufnum:10);

s.sync;


SynthDef(\grains4,{
		arg out = 0, bufNum = 10, amp = 1, interp = 4, wet=1;

		var input,output,grains; 
		var posLo, posHi, granDur, rate, density;
		var trig, recLoop=1; 
		var pos, bufDur, bufDurSection, posDif;
		var posRate=0.01, pan =0;
		var trigk, recLoopk;

		bufDur = BufDur.kr(bufNum);
		input=HPF.ar(SoundIn.ar(0), 80);

		posLo 	= AnalogIn.ar(0).linlin(~pots_low,~pots_high,0,0.5);
		posHi 	= AnalogIn.ar(0).linlin(~pots_low,~pots_high,1,0.5); 
		granDur = AnalogIn.ar(1).linlin(~pots_low,~pots_high,0.001,1.0);
		rate 	= AnalogIn.ar(2).linlin(~pots_low,~pots_high,-2,2);
		density = AnalogIn.ar(3).linlin(~pots_low,~pots_high,1,60);

		recLoop = DigitalIn.ar(~t1_in) + DigitalIn.ar(~sw_in);
		// trig = DigitalIn.ar(0);

		trig = Impulse.ar(density);

		posDif = posHi - posLo;
		bufDurSection = bufDur * posDif;

		pos = posLo * bufDur + (Phasor.ar(0, BufRateScale.kr(bufNum) * posRate / SampleRate.ir, posLo * bufDur, posHi * bufDur) ).mod(bufDurSection);

		BufWr.ar(input*EnvGen.ar(Env([0,1,0],[0.01,density.reciprocal-0.01]),trig), bufNum, Phasor.ar(0, BufRateScale.kr(bufNum)*recLoop, 0, BufFrames.kr(bufNum)), recLoop);
		grains = TGrains.ar(2, trig, bufNum, rate, pos, granDur, pan, amp, interp);
		output = (input * (1 - (wet))) + (grains * (wet));

		recLoopk = A2K.kr(recLoop);
		DigitalIO.ar(~led1,output:0, pinMode:recLoopk);
		trigk = A2K.kr(recLoop);
		DigitalIO.ar(~t1_out,output:trigk, pinMode:1 );
		// DigitalIO.ar(~led1,output:0, pinMode:trigk);

		Out.ar(out, output);

}).add;

~granular = Synth(\grains4,[\out, 0, \bufNum, 10, \amp, 1, \interp, 4, \wet, 1]);


/**
 * Use CV outs for something useful :) 
 */
SynthDef( \lfo1, {
	arg fpin, opin;
	var freq, sig;
	freq = AnalogIn.ar(fpin).exprange(0.5,60);
	sig = SinOsc.ar(freq);
	AnalogOut.ar(opin,sig);
}).add();

SynthDef( \lfo2, {
	arg fpin, opin;
	var freq, sig;
	freq = AnalogIn.ar(fpin).exprange(0.5,60);
	sig = LFSaw.ar(freq);
	AnalogOut.ar(opin,sig);
}).add();


SynthDef( \lfo3, {
	arg fpin, opin;
	var freq, sig;
	freq = AnalogIn.ar(fpin).exprange(0.5,60);
	sig = LFPulse.ar(freq);
	AnalogOut.ar(opin,sig);
}).add();


SynthDef( \lfo4, {
	arg fpin, opin;
	var freq, sig;
	freq = AnalogIn.ar(fpin).exprange(0.5,60);
	sig = LFDNoise0.ar(freq);
	AnalogOut.ar(opin,sig);
}).add();



~lfo1 = Synth(\lfo1,[\fpin,4 , \opin, 4]);
~lfo2 = Synth(\lfo2,[\fpin,5 , \opin, 5]);
~lfo3 = Synth(\lfo3,[\fpin,6 , \opin, 6]);
~lfo4 = Synth(\lfo4,[\fpin,7 , \opin, 7]);


"Server READY".postln;


}; /* boot */


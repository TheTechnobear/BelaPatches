/*******************
Remotely Control the Bela

Use this project to boot scsynth on the Bela.

This script simply starts scsynth, which then waits for messages.
Use the code in the other .scd files in this project to interact with the board
from the SuperCollider IDE running on the host.

IMPORTANT: you will need to add the class files for the Bela-specific UGens to your
computer in order to run these scripts. You can get them here
https://github.com/sensestage/bela-remote

(c) 2017: Jonathan Reus, Marije Baalman, Giulio Moro, Andrew McPherson

Please note that support for SuperCollider on Bela is still experimental,
so feel free to report issues here: https://github.com/sensestage/supercollider/issues
*/

s = Server.default;

s.options.numAnalogInChannels = 8; // can be 2, 4 or 8
s.options.numAnalogOutChannels = 8; // can be 2, 4 or 8
s.options.numDigitalChannels = 16;
s.options.maxLogins = 4;

s.options.pgaGainLeft = 4;     // sets the gain for the left audio input to 4 dB
s.options.pgaGainRight = 5;    // sets the gain for the left audio input to 5 dB
s.options.headphoneLevel = -6; // sets the headphone level to -6 dB
s.options.speakerMuted = 0;    // enable the speaker amp
s.options.dacLevel = 0;       // sets the gain of the dac to 0 dB
s.options.adcLevel = 0;       // sets the gain of the adc to 0 dB
s.options.numMultiplexChannels = 0; // do not enable multiplexer channels

s.options.blockSize = 16;
// number of audio channels: values greater than 2 will use the first few analog
// channels as audio channels.
// Any additional analog channels will be available through the AnalogIn/Out Ugens
// e.g.:
// s.options.numInputBusChannels = 4;
// means that bus in 0/1 are the audio L/R channels and 2/3 are the analog inputs 0/1
// analog inputs 2-7 will still be available through the AnalogIn Ugen.
s.options.numInputBusChannels = 2; // Use only the L/R audio channels
s.options.numOutputBusChannels = 2; // Use only the L/R audio channels
s.options.numAudioBusChannels = 1024;
s.options.memSize = 8192 * 16; // the default is 8192 which is quite little
// s.boot;



s.waitForBoot{
	"Server Booted".postln;

	MIDIClient.init;
	MIDIIn.connectAll;

/*
SynthDef("quick2", {
    arg freq = 440, amp = 0.1, gate = 1;
	var snd, env;
	env = Env.adsr(0.01, 0.1, 0.3, 2, amp).kr(2, gate);
	snd = Saw.ar([freq, freq*1.5], env);	
	Out.ar(0, snd)
}).add;

~noteArray = Array.newClear(128); // array has one slot per possible MIDI note

MIDIdef.noteOn(\myKeyDown, {arg vel, note;
	~noteArray[note] = Synth("quick2", [\freq, note.midicps, \amp, vel.linlin(0, 127, 0, 1)]);
	["NOTE ON", note].postln;
});
	
MIDIdef.noteOff(\myKeyUp, {arg vel, note;
	~noteArray[note].set(\gate, 0);
	["NOTE OFF", note].postln;
});
*/

/*
	Ndef(\midich1, { 
			arg freq = 440, amp = 0.1, gate = 0;
			var snd, env;
			env = Env.adsr(0.01, 0.1, 0.3, 2, amp).kr(2, gate);
			snd = Saw.ar([freq, freq*1.5], env);	
			Out.ar(0, snd)
	});


	MIDIdef.noteOn(\noteon1, { 
		arg vel, note;
		Ndef(\midich1).set(\freq,note.midicps);
		Ndef(\midich1).set(\gate,1);
    	Ndef(\midich1).fadeTime=0;	
	},chan:0);
	
	MIDIdef.noteOff(\noteoff1, { 
		arg vel, note;
    	Ndef(\midich1).fadeTime=0;	
		Ndef(\midich1).set(\freq,note.midicps);
		Ndef(\midich1).set(\gate,0);
	},chan:0);
*/
/*
	SynthDef(\tiled, {
		arg freq = 440, amp = 0.1, gate = 0;
		var snd, env;
		env = Env.adsr(0.01, 0.1, 0.3, 2, amp).kr(2, gate);
		snd = Saw.ar([freq, freq*1.5], env);	
		Out.ar(0, snd)
	}).add;
	
	~s = Synth(\tiled);




	MIDIdef.noteOn(\noteon1, { 
		arg vel, note;
		~s.set(\freq,note.midicps);
		~s.set(\gate,vel>0);
	},chan:0);
	
	MIDIdef.noteOff(\noteoff1, { 
		arg vel, note;
		~s.set(\freq,note.midicps);
		~s.set(\gate,0);
	},chan:0);
*/


	s.sync;


};


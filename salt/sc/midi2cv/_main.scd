
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

s.sync;



// MIDI to CV
MIDIClient.init;
MIDIIn.connectAll;


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

Ndef(\led_pwm,{DigitalIO.ar(~led_pwm, pinMode:1, output:LFPulse.ar( freq:(44100/32), width: 0.5))});

SynthDef(\cvout, {
    arg opin, tpin, lpin, gate=0,note=64;
	AnalogOut.kr(opin, note.linlin(0,120,0,1));
	DigitalIO.kr(tpin,output:gate, pinMode:1 );
	// DigitalIO.kr(lpin,output:0, pinMode:gate );
}).add;

~synths = [
	Synth(\cvout,[\opin, 0, \tpin, ~t1_out, \lpin, ~led1] ),
	Synth(\cvout,[\opin, 1, \tpin, ~t2_out, \lpin, ~led2] ),
	Synth(\cvout,[\opin, 2, \tpin, ~t3_out, \lpin, ~led3] ),
	Synth(\cvout,[\opin, 3, \tpin, ~t4_out, \lpin, ~led4] )
];


MIDIdef.noteOn(\noteon1, {
	arg vel, note, chan;
	// [note, vel, chan, "note on"].postln;
	~synths[chan].postln;
	~synths[chan].set(\note, note);
	~synths[chan].set(\gate, vel>0);
},chan:[0,1,2,3]);

MIDIdef.noteOff(\noteoff1, {
	arg vel, note, chan;
	~synths[chan].set(\note, note);
	~synths[chan].set(\gate, 0);
},chan:[0,1,2,3]);

// ENV
SynthDef( \adenv, {
	arg tpin, lpin, apin, dpin, opin;
	var attack = AnalogIn.kr(apin).range(0,5);
	var release  = AnalogIn.kr(dpin).range(0,5);
	var g = DigitalIn.kr(tpin);
	var sig = EnvGen.ar(Env.perc(attack, release), g);
	DigitalIO.kr(lpin,output:0, pinMode:g );
	AnalogOut.ar(opin,sig);
}).add();


~env1 = Synth(\adenv,[\tpin,~t1_in , \lpin, ~led1, \apin, 0 , \dpin, 1, \opin , 4]);
~env2 = Synth(\adenv,[\tpin,~t2_in , \lpin, ~led2, \apin, 2 , \dpin, 3, \opin , 5]);



// LFO


SynthDef( \lfo, {
	arg fpin, opin;
	var freq, sig;
	freq = AnalogIn.ar(fpin).exprange(0.5,60);
	sig = LFTri.ar(freq);
	AnalogOut.ar(opin,sig);
}).add();


~lfo1 = Synth(\lfo,[\fpin,4 , \opin, 6]);
~lfo2 = Synth(\lfo,[\fpin,6 , \opin, 7]);



};


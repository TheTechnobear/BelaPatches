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

// LFO
SynthDef( \lfo, {
	arg fpin, opin;
	var freq, sig;
	freq = AnalogIn.ar(fpin).exprange(1,60);
	sig = LFTri.ar(freq);
	AnalogOut.ar(opin,sig);
}).add();


~lfo1 = Synth(\lfo,[\fpin,0 , \opin, 0]);
~lfo2 = Synth(\lfo,[\fpin,1 , \opin, 1]);
~lfo3 = Synth(\lfo,[\fpin,2 , \opin, 2]);
~lfo4 = Synth(\lfo,[\fpin,3 , \opin, 3]);



// ENV / VCA
SynthDef( \adenv, {
	arg tpin, lpin, apin, dpin, opin, ai, ao;
	var attack = AnalogIn.kr(apin).range(0,5);
	var release  = AnalogIn.kr(dpin).range(0,5);
	var g = DigitalIn.kr(tpin);
	var env = EnvGen.ar(Env.perc(attack, release), g);
	var sig = SoundIn.ar(ai) * env;
	DigitalIO.kr(lpin,output:0, pinMode:g );
	AnalogOut.ar(opin,env);
	Out.ar(ao, sig);
}).add();


~env1 = Synth(\adenv,[\tpin,~t3_in , \lpin, ~led3, \apin, 4 , \dpin, 6, \opin , 4, \ai, 0, \ao, 0]);
~env2 = Synth(\adenv,[\tpin,~t4_in , \lpin, ~led4, \apin, 5 , \dpin, 7, \opin , 5, \ai, 1, \ao, 1]);




};


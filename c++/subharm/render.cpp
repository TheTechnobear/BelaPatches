#include <Bela.h>

#include <math.h>

#include <libraries/Scope/Scope.h>

#include <libraries/Oscillator/Oscillator.h>

// pepper pin allocations
static constexpr unsigned numDigIn = 4;
static constexpr unsigned digInPins[numDigIn] = {11, 9, 13, 12};
static constexpr unsigned numButton = 2;
static constexpr unsigned buttonPins[numButton] = {15, 14};
static constexpr unsigned numLed = 10;
static constexpr unsigned ledPins[numLed] = {6, 7, 10, 2, 3, 0, 1, 4, 5, 8 };


static constexpr float voltOutRange = 5.0f;
static constexpr float BASE_FREQ = 880.0f;

//data about a particular sequencer
struct SubHarmOscillator {
	SubHarmOscillator() {
		for (unsigned i = 0; i < 4; i++) {
			cvVals_[i] = 0.0f;
			timeVals_[i] = 1.0f;
		}
	}

	static constexpr unsigned MAX_TARGET = 8;
	float baseMain_ = 0.0f;
	float baseS1_ = 1.0f;
	float baseS2_ = 1.0f;

	unsigned	curStep_ = 0;
	unsigned    seqTarget_ = 0; // binary osc+s1+s2  0..7
	float		cvVals_[4];
	float		timeVals_[4];


	float decayT_ = 0.0001f;
	float env_ = 0.0f;

	// oscillator info
	float levelMain_ = 0.0f;
	float levelS1_ = 0.0;
	float levelS2_ = 0.0f;
};

//state about SubHarm
struct SubHarm {
	static constexpr unsigned MAX_OSC = 2;
	SubHarmOscillator osc_[MAX_OSC];
	unsigned curOsc_ = 0;
	bool baseMode_ = true;

	// data about main clock
	bool     clkState_ = false;
	unsigned clkCount_ = 0;
	unsigned clkSamples_ = 0;

	// misc
	bool b1State_ = false;
	bool b2State_ = false;
	bool ignoreB1Rel_ = false;


	// audio output
	Oscillator audioOsc_[MAX_OSC][3];
};


Scope gScope;
SubHarm gSubHarm;
unsigned  gLedClkFlash = 0;

float freqToCV(float f) {
	float po  = log2(f / BASE_FREQ) + 2.5f; // unipolar output
	return (po / voltOutRange) ;
}

float cvToFreq(float cv, bool quantize = true) {
	// -1 to 1 input
	float po = 0.0f;
	if (quantize) {
		static constexpr float semiRange = (voltOutRange / 2.0f) * 12.0f;
		float p = floorf(cv * semiRange);
		po = p / 12.0f;
	} else {
		static constexpr float octRange = (voltOutRange / 2.0f);
		po = cv * octRange;
	}
	return BASE_FREQ * powf(2.0f, po);
}

float cvToHarm(float v) {
	// -1 to 1 input
	// return 0  1/1 (~1.0) -  1/16 (0)
	float h  =  floorf(8.0f - (v * 8.0f));
	h = constrain(h, 1.0f, 16.0f);
	if (h > 0.0f) {
		return 1.0f / h;
	}
	return 1.0f;
}

float cvToTime(float v) {
	// -1 to 1 input
	// 0 , 1/1 to 1/16 out
	float h  =  floorf(8.0f - (v * 8.0f));
	h = constrain(h, 0.0f, 16.0f);
	if (h > 0.0f) {
		return 1.0f / h;
	}
	return 0.0f;
}

bool setup(BelaContext *context, void *userData) {
	rt_printf("subharm::setup\n");

	// for simplicity assume analogframes are half audio/digital (default)
	if (	context->analogFrames != ( context->digitalFrames / 2)
	        || context->audioFrames != context->digitalFrames ) {
		rt_printf("configuration incorrect:\n"
		          "digitalFrames : %d (16)\n"
		          "audioFrames : %d (16)\n"
		          "analogFrames : %d (8)\n"
		          ,
		          context->digitalFrames, context->audioFrames, context->analogFrames);
		return false;
	}


	gScope.setup(2, context->audioSampleRate);

	//setup digital pins
	for (unsigned n = 0; n < numLed; n++) {
		pinMode(context, 0, ledPins[n], OUTPUT);
	}
	for (unsigned n = 0; n < numButton; n++) {
		pinMode(context, 0, buttonPins[n], INPUT);
	}

	//setup audio oscillators
	for (unsigned n = 0; n < SubHarm::MAX_OSC; n++) {
		gSubHarm.audioOsc_[n][0].setup(context->audioSampleRate, Oscillator::sine);
		gSubHarm.audioOsc_[n][1].setup(context->audioSampleRate, Oscillator::sine);
		gSubHarm.audioOsc_[n][2].setup(context->audioSampleRate, Oscillator::sine);
	}


	gLedClkFlash = context->digitalSampleRate / 10;
	return true;
}

void cleanup(BelaContext *context, void *userData) {
	rt_printf("subharm::cleanup\n");
}



void render(BelaContext *context, void *userData)
{
	auto& s = gSubHarm;

	// note: context->digitalFrames == context->audioFrames !
	for (unsigned n = 0; n < context->digitalFrames; n++) {

		// handle incoming clock on pin 4
		auto clk = digitalRead(context, n, digInPins[3]);
		if (s.clkState_ != clk && clk) { //rise edge trig
			s.clkSamples_ = s.clkCount_;
			s.clkCount_ = 0;
		} else {
			s.clkCount_++;
		}
		s.clkState_ = clk;


		auto b1 = digitalRead(context, n, buttonPins[0]);
		auto b2 = digitalRead(context, n, buttonPins[1]);

		// UI
		// B1 = toggle base/seq
		// B2 = select seq target
		// B1 + B2 = next osc

		if (s.b1State_ == b1) {
			// v1 steady state
			if (b1) {
				// b1 held down
				if (s.b2State_ != b2 && !b2) {
					//falling edge trig on b2
					s.curOsc_ = (s.curOsc_ + 1) % SubHarm::MAX_OSC;
					s.ignoreB1Rel_ = true;
				}
			} else {
				// b1 up
				if (s.b2State_ != b2 && !b2) {
					auto &curOsc = s.osc_[s.curOsc_];
					curOsc.seqTarget_ = (curOsc.seqTarget_ + 1) % SubHarmOscillator::MAX_TARGET;
				}
			}
		} else {
			// v1 transition state
			if (!b1 && !s.ignoreB1Rel_) { // button release
				s.baseMode_ = !s.baseMode_;
			}
			s.ignoreB1Rel_ = false;
		}

		s.b1State_ = b1;
		s.b2State_ = b2;


		auto &curOsc = s.osc_[s.curOsc_];

		// led display
		// mode
		digitalWriteOnce(context, n, ledPins[0], s.curOsc_);
		digitalWriteOnce(context, n, ledPins[1], s.baseMode_);
		//target
		digitalWriteOnce(context, n, ledPins[2], curOsc.seqTarget_ & 0b100);
		digitalWriteOnce(context, n, ledPins[3], curOsc.seqTarget_ & 0b010);
		digitalWriteOnce(context, n, ledPins[4], curOsc.seqTarget_ & 0b001);
		//clk in
		digitalWriteOnce(context, n, ledPins[5], s.clkCount_ < gLedClkFlash);
		//step
		digitalWriteOnce(context, n, ledPins[6], curOsc.curStep_ == 0);
		digitalWriteOnce(context, n, ledPins[7], curOsc.curStep_ == 1);
		digitalWriteOnce(context, n, ledPins[8], curOsc.curStep_ == 2);
		digitalWriteOnce(context, n, ledPins[9], curOsc.curStep_ == 3);



		// note: context->analogFrames == context->digitalFrames / 2 !
		if (n % 2 == 0)  {
			unsigned f = n / 2;


			// read analog values
			static constexpr float mult = 2.0f + 0.09f;
			static constexpr float centre = (mult / 2.0f);
			if (s.baseMode_) {

				// base frequencies/harmonics
				curOsc.baseMain_ = (analogRead(context, f, 0) * mult) - centre;
				curOsc.baseS1_ = (analogRead(context, f, 1) * mult) - centre;
				curOsc.baseS2_ = (analogRead(context, f, 2) * mult) - centre;

				// oscillator type
				unsigned nOT = (analogRead(context, f, 3) - 0.1) * float(Oscillator::numOscTypes);
				Oscillator::Type ot = (Oscillator::Type ) constrain(nOT, 0, (float) Oscillator::numOscTypes);
				for (int oi = 0; oi < 3; oi++) {
					s.audioOsc_[s.curOsc_][oi].setType(ot);
				}

				// levels
				curOsc.levelMain_ = analogRead(context, f, 4);
				curOsc.levelS1_ = analogRead(context, f, 5);
				curOsc.levelS2_ = analogRead(context, f, 6);
				curOsc.decayT_ = (exp(  analogRead(context, f, 7) * -5.0f) / 1000.f);

			} else {
				// sequencer values
				curOsc.cvVals_[0] = (analogRead(context, f, 0) * mult) - centre;
				curOsc.cvVals_[1] = (analogRead(context, f, 1) * mult) - centre;
				curOsc.cvVals_[2] = (analogRead(context, f, 2) * mult) - centre;
				curOsc.cvVals_[3] = (analogRead(context, f, 3) * mult) - centre;

				curOsc.timeVals_[0] = (analogRead(context, f, 4) * mult) - centre;
				curOsc.timeVals_[1] = (analogRead(context, f, 5) * mult) - centre;
				curOsc.timeVals_[2] = (analogRead(context, f, 6) * mult) - centre;
				curOsc.timeVals_[3] = (analogRead(context, f, 7) * mult) - centre;
			}

			// write outputs
			for (unsigned i = 0; i < SubHarm::MAX_OSC; i++) {
				auto& osc = s.osc_[i];
				auto baseout = i * 4;
				auto trig = false;

				// jitter in clock, avoid trigger in last frame
				if (s.clkCount_ < (s.clkSamples_ - context->digitalFrames) ) {
					for (int c = 0; c < 4 && !trig ; c++ ) {
						float t = cvToTime(osc.timeVals_[c]);
						unsigned samp = unsigned (float(s.clkSamples_) * t);

						trig |= (	samp > 0
						            &&	( ( s.clkCount_  % samp) < 2)
						        );
					}
				}

				// if trigged, advance step, and raise env
				if (trig) {
					osc.curStep_ = (osc.curStep_ + 1) % 4;
					osc.env_ = 1.0f;
				}

				// analog rate=SR/2 , we want 2ms (in analog samples)
				osc.env_ *= (1.0f - osc.decayT_);

				auto& cvVal = osc.cvVals_[osc.curStep_];

				float mFreq = cvToFreq(osc.baseMain_ + (osc.seqTarget_ & 0b001 ?  cvVal : 0));

				float s1H = cvToHarm(osc.baseS1_ + (osc.seqTarget_ & 0b010 ? cvVal : 0));
				float f1 = mFreq * s1H;

				float s2H = cvToHarm(osc.baseS2_ + (osc.seqTarget_ & 0b100 ? cvVal : 0));
				float f2 = mFreq * s2H;

				// write outputs
				analogWriteOnce(context, f, baseout, 		freqToCV(mFreq));
				analogWriteOnce(context, f, baseout + 1,	freqToCV(f1));
				analogWriteOnce(context, f, baseout + 2,	freqToCV(f2));
				analogWriteOnce(context, f, baseout + 3,	osc.env_);
				
				
				// update audio oscillators frequencies
				s.audioOsc_[i][0].setFrequency(mFreq);
				s.audioOsc_[i][1].setFrequency(f1);
				s.audioOsc_[i][2].setFrequency(f2);
			}
		} // analog rate

		// generate audio : left = osc 1 , right = osc 2
		float a0 = (
		    s.audioOsc_[0][0].process() * s.osc_[0].env_ * s.osc_[0].levelMain_
		    + s.audioOsc_[0][1].process() * s.osc_[0].env_ * s.osc_[0].levelS1_
		    + s.audioOsc_[0][2].process() * s.osc_[0].env_ * s.osc_[0].levelS2_
		    ) * 0.5;

		float a1 = (
		    s.audioOsc_[1][0].process() * s.osc_[1].env_ * s.osc_[1].levelMain_
		    + s.audioOsc_[1][1].process() * s.osc_[1].env_ * s.osc_[1].levelS1_
		    + s.audioOsc_[1][2].process() * s.osc_[1].env_ * s.osc_[1].levelS2_
		    ) * 0.5;


		audioWrite(context, n, 0, a0);
		audioWrite(context, n, 1, a1);
		gScope.log(a0, a1);
	}
}






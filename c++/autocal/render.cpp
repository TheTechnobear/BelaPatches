#include <Bela.h>

#include <memory>
#include <vector>

#include <q/support/literals.hpp>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdefaulted-function-deleted"
#include <q/pitch/pitch_detector.hpp>
#pragma GCC diagnostic pop

namespace q = cycfi::q;
using namespace q::literals;

#include "platform_defs.h"

#include <cmath>
static constexpr float PI2 = 2 * (float) M_PI;

static constexpr float BASE_FREQ = 220.0f;

float t_ptFreq = 0.0f;
float t_testFreq = 0.0f;

float prevFreq = 0.0f;
void generateTestTone(BelaContext *context, void *userData) {
	static float t_gPhase = 0.0;
	static float t_gInverseSampleRate = 0.0f;
	if (t_gInverseSampleRate < 1.0f) {
		t_gInverseSampleRate = 1.0f / context->audioSampleRate;
	}

	if (t_testFreq != prevFreq) {
		rt_printf("test freq %f\n", t_testFreq);
		prevFreq = t_testFreq;
	}

	float t_amplitude = 1.0f;
	for (unsigned int n = 0; n < context->audioFrames; n++) {

		// test tone, frequency by pot 1
		float out1 = t_amplitude * sinf(t_gPhase);
		audioWrite(context, n, 0, out1);
		t_gPhase += PI2 * t_testFreq * t_gInverseSampleRate;
		if (t_gPhase > PI2) t_gPhase -= PI2;
	}
}

static std::unique_ptr<q::pitch_detector> t_pitchDetector;

static constexpr unsigned MAX_RES = 5;
struct {
	unsigned stage_ = 0;
	float currentV_ = 0.0;
	float tunedV_[MAX_RES + 1];
	float minV_;
	float maxV_;
	bool calibrating_;
} t_Outputs[8];

unsigned t_Output = 0;

void setupPitchDetector(BelaContext *context,float tfreq) {
	// q::frequency lowest_freq = tfreq * 0.5f ;
	// q::frequency highest_freq = tfreq * 2.0f;
	q::frequency lowest_freq = 100;
	q::frequency highest_freq = 10000;
	rt_printf("PD setup: %f  - %f/%f hZ\n", tfreq, lowest_freq, highest_freq);
	t_pitchDetector = std::make_unique<q::pitch_detector>(lowest_freq, highest_freq , context->audioSampleRate, -45_dB);
}

bool setup(BelaContext *context, void *userData) {
	initDigital(context);
	setupPitchDetector(context,BASE_FREQ);
	return true;
}

void cleanup(BelaContext *context, void *userData) {
	t_pitchDetector = nullptr;
}

static unsigned curOutputN = 0xff;

void updateLEDs(BelaContext* context) {
	static unsigned counter = 0;
	counter++;

#ifdef PEPPER
	if (curOutputN != t_Output) {
		for (unsigned led = 0; led < 8; led++) setLed(context, led, 0);
		setLed(context, t_Output, 1);
		curOutputN = t_Output;
	}
#endif

	if (counter % 500 == 0) {
		setLed(context, numLeds - 1, t_Outputs[t_Output].calibrating_ && counter % 1000);
	}
}

float convertPitchToDec(float p) {
	return p;
}

bool is_ready = false;

void render(BelaContext *context, void *userData)
{
	static unsigned counter = 0;

	dsRead(context);
	if (dsIsButtonTrig(0)) {
		auto& o = t_Outputs[t_Output];
		if (!o.calibrating_) {
			for (unsigned n = 0; n < MAX_RES + 1; n++) {
				o.tunedV_[n] = n / 10.0f;
			}
			o.stage_ = 0;
			o.minV_ = voltOutMin;
			o.maxV_ = voltOutMax;
			o.calibrating_ = true;
			o.currentV_ = float(o.stage_) / float(MAX_RES);

			float p = (o.currentV_ * voltOutRange);
			t_testFreq = BASE_FREQ * powf(2, p);
			t_ptFreq = 0.0f;
			counter = 0;
			is_ready = false;
			// setupPitchDetector(context,t_testFreq);

		// } else {
		// 	o.calibrating_ = false;
		}
	}

	if (dsIsButtonTrig(1)) {
		// next output
		t_Outputs[t_Output].calibrating_ = false;
		t_Output++;
		t_Output = t_Output % 8;
	}


	updateLEDs(context);

	for (unsigned n = 0; n < context->analogFrames; n++) {
		analogWrite(context, n, t_Output, t_Outputs[t_Output].currentV_);
	}


	if (t_Outputs[t_Output].calibrating_) {
		counter++;
		for (unsigned n = 0; n < context->audioFrames; n++) {
			float v0 = audioRead(context, n, 0);
			is_ready = (*t_pitchDetector)(v0);
		}
	}


	// check calibration
	if (t_Outputs[t_Output].calibrating_)
	{
		auto& o = t_Outputs[t_Output];
		if (is_ready || (counter % 10000 ==0) ) {
		// if (counter % 2000 == 0) {

			if (is_ready) {
				// t_ptFreq = t_pitchDetector->get_frequency();
				t_ptFreq = t_pitchDetector->predict_frequency();
			} else {
				t_ptFreq =0.0;
			}

			// end of test
			if (o.stage_ == 0) {
				o.minV_ = convertPitchToDec(t_ptFreq);
			} else if (o.stage_ == MAX_RES) {
				o.maxV_ = convertPitchToDec(t_ptFreq);
			}

			o.tunedV_[o.stage_] = convertPitchToDec(t_ptFreq);


			rt_printf("done test %d %f %f : %f\n", o.stage_, o.currentV_, t_testFreq, t_ptFreq);
			rt_printf("get_frequency %f predict_frequency %f ,  periodicity %f\n", 
				t_pitchDetector->get_frequency(), 
				t_pitchDetector->predict_frequency(), 
				t_pitchDetector->periodicity());

			o.stage_++;
			o.stage_ = o.stage_ % (MAX_RES + 1);
			o.currentV_ = float(o.stage_) / float(MAX_RES);

			float p = (o.currentV_ * voltOutRange);
			t_testFreq = BASE_FREQ * powf(2, p);
			t_ptFreq = 0.0f;
			counter = 0;
			// setupPitchDetector(context,t_testFreq);
			t_pitchDetector->reset();
			is_ready = false;

			if (o.stage_ == 0) {
				o.calibrating_ = false;

				rt_printf("calibrated output : %d\n", t_Output);
				for (unsigned n = 0; n < MAX_RES + 1; n++) {
					rt_printf("tunedV [%d] : %f \n", n, o.tunedV_[n]);
				}
				rt_printf("\n");
			}
		}
	}


	generateTestTone(context, userData);
	drivePwm(context);
}






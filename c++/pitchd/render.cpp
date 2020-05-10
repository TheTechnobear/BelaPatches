#include <Bela.h>

#include <memory>
#include <vector>


// SELECT ONE OF THESE, COMMENT OUT OTHERS
// #define DYWAPITCHTRACKER 1
#define Q_LIB_PITCHTRACKER 1



//// test only
#include <cmath>
float   t_ptPitch =0.0f;
float 	t_gPhase[2];
float 	t_gInverseSampleRate;
int 	t_gAudioFramesPerAnalogFrame = 0;
int 	t_gSensorInputFrequency = 0;
int 	t_gSensorInputAmplitude = 1;
static constexpr float PI2 = 2 * (float) M_PI;


void generateTone(BelaContext *context, void *userData) {
	// test only
	float t_frequency = 440.0f;
	float t_amplitude = 0.5f;
	for (unsigned int n = 0; n < context->audioFrames; n++) {
		if (t_gAudioFramesPerAnalogFrame && !(n % t_gAudioFramesPerAnalogFrame)) {
			t_frequency = map(analogRead(context, n / t_gAudioFramesPerAnalogFrame, t_gSensorInputFrequency), 0, 1, 100, 1000);
			t_amplitude = analogRead(context, n / t_gAudioFramesPerAnalogFrame, t_gSensorInputAmplitude);
		}

		// test tone, frequency by pot 1
		float out1 = t_amplitude * sinf(t_gPhase[0]);
		audioWrite(context, n, 0, out1);
		t_gPhase[0] += PI2 * t_frequency * t_gInverseSampleRate;
		if (t_gPhase[0] > PI2) t_gPhase[0] -= PI2;

		// tracked frequency from audio input 1
		float out2 = t_amplitude * sinf(t_gPhase[1] );
		if (t_ptPitch>100) audioWrite(context, n, 1, out2);
		t_gPhase[1] += PI2 * t_ptPitch * t_gInverseSampleRate;
		if (t_gPhase[1] > PI2) t_gPhase[1] -= PI2;
	}
}




////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///DYWAPITCHTRACKER
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef DYWAPITCHTRACKER


// TODO 
// - implement rolling window (change dywapitchtrack)
// - pitchd task, pass pitch via pipe? but still we share the buffer

static constexpr int PT_PITCH_WINDOW = 1024;
static constexpr int PT_MAX_BUF_SZ = PT_PITCH_WINDOW;

static int pt_smpCount = 0;
static int pt_smpReadPos = 0;
static int pt_smpWritePos = 0;


#define TB 1
#ifdef TB
	// #pragma message("using tb_dywapitchtrack")
	// technobear version : use floats, only allocate memory once
	#include "tb_dywapitchtrack.h"
	#define PT_TYPE technobear::dywapitchtracker
	#define PT_INIT technobear::dywapitch_inittracking
	#define PT_COMPUTE technobear::dywapitch_computepitch
	#define PT_DEINIT(x) technobear::dywapitch_deinittracking(x)
	PT_TYPE pt_tracker_;
	float pt_smpBuf_[PT_MAX_BUF_SZ];
	float  pt_pitch_ = 0.0f;
#else 
	// #pragma message("using dywapitchtrack")
	#include "dywapitchtrack.h"
	#define PT_TYPE dywapitchtracker
	#define PT_INIT dywapitch_inittracking
	#define PT_COMPUTE dywapitch_computepitch
	#define PT_DEINIT(x) 
	PT_TYPE pt_tracker_;
	double pt_smpBuf_[PT_MAX_BUF_SZ];
	double pt_pitch_ = 0.0f;
#endif

#include <AuxTaskNonRT.h>
std::unique_ptr<AuxTaskNonRT> pt_task_;

static void pitchd_task() {
	while (!gShouldStop) {
		if (pt_smpCount >= PT_MAX_BUF_SZ)  {
			pt_pitch_ = PT_COMPUTE(&pt_tracker_, pt_smpBuf_, pt_smpReadPos, PT_PITCH_WINDOW);
			//next window... 
			pt_smpCount = 0;
		}
	}
}


bool setup(BelaContext *context, void *userData)
{
	pt_smpCount = 0;
	pt_smpReadPos = 0;
	pt_smpWritePos = 0;
	PT_INIT(&pt_tracker_);
	pt_task_ = std::unique_ptr<AuxTaskNonRT>(new AuxTaskNonRT());
	pt_task_->create(std::string("pt_task_"), pitchd_task);
	pt_task_->schedule();

	// test only
	if (context->analogFrames) t_gAudioFramesPerAnalogFrame = context->audioFrames / context->analogFrames;
	t_gInverseSampleRate = 1.0 / context->audioSampleRate;
	t_gPhase[0] = 0.0;
	t_gPhase[1] = 0.0;
	return true;
}

void cleanup(BelaContext *context, void *userData)
{
	PT_DEINIT(&pt_tracker_);
}

void render(BelaContext *context, void *userData)
{
	// pitch tracking code
	for (unsigned int n = 0; n < context->audioFrames; n++) {
		float v0 = audioRead(context, n, 0);
		pt_smpBuf_[pt_smpWritePos] = v0;
		pt_smpWritePos++;
		pt_smpWritePos = pt_smpWritePos < PT_MAX_BUF_SZ ? pt_smpWritePos : 0;
	}

	if (pt_smpCount < PT_MAX_BUF_SZ) {
		pt_smpCount += context->audioFrames;
	}

	// for test
	t_ptPitch = pt_pitch_;
	generateTone(context,userData);
}

#endif //DYWAPITCHTRACKER


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///Q_LIB_PITCHTRACKER
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef Q_LIB_PITCHTRACKER 


#include <q/support/literals.hpp>
#include <q/pitch/pitch_detector.hpp>

namespace q = cycfi::q;
using namespace q::literals;
// using std::fixed;
constexpr auto pi = q::pi;
constexpr auto sps = 44100;
constexpr auto verbosity = 0;


q::frequency actual_frequency=0;
q::frequency lowest_freq = 100;
q::frequency highest_freq = 4000;

q::pitch_detector    pd(lowest_freq, highest_freq, sps, -45_dB);

bool setup(BelaContext *context, void *userData)
{
	// test only
	if (context->analogFrames) t_gAudioFramesPerAnalogFrame = context->audioFrames / context->analogFrames;
	t_gInverseSampleRate = 1.0 / context->audioSampleRate;
	t_gPhase[0] = 0.0;
	t_gPhase[1] = 0.0;
	return true;
}

void cleanup(BelaContext *context, void *userData)
{
}

void render(BelaContext *context, void *userData)
{
  	bool is_ready = false; 

	// pitch tracking code
	for (unsigned int n = 0; n < context->audioFrames; n++) {
		float v0 = audioRead(context, n, 0);
		is_ready=pd(v0);
	}

	t_ptPitch = pd.get_frequency();
	generateTone(context,userData);
}



#endif // Q_LIB_PITCHTRACKER




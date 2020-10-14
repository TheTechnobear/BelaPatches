#include <Bela.h>
#include <libraries/Midi/Midi.h>

#include <ableton/Link.hpp>
#include <ableton/link/HostTimeFilter.hpp>

#include <memory>
#include <vector>


std::unique_ptr<ableton::Link> link_;

ableton::link::HostTimeFilter<ableton::link::platform::Clock> time_filter;

AuxiliaryTask task_;

std::vector<Midi*> midiPorts_;
bool midiEnabled_=false;

static unsigned blockTimeuSec_=0;
float sampleTime_=0.0f;

/// pepper pins
// ~digitalIn = [11,9,13,12];

const unsigned ledPins[] = {6,7,10,2,3,0,1,4,5,8 };
const unsigned buttonPins [] = {15,14};

static midi_byte_t midiClock=0xF8;
static midi_byte_t midiStart=0xFA;
// static midi_byte_t midiContinue=0xFB;
static midi_byte_t midiStop=0xFC;

float quantum_=4.0f;
float steps_per_beat_ = 1.0f;
float ppqn_per_beat=24.0f;

volatile unsigned step_=0; 
volatile bool clockTrig_=false;
volatile bool barTrig_=false;


bool requestStartStop_=false;
bool clockWhenStopped_=true;
volatile bool scheduleStart_=false;
volatile bool playing_=false;

volatile bool writeMidiStart=false;
volatile bool writeMidiClock=false;
volatile bool writeMidiStop=false;

int latency_=0.0f;


static void link_task(void* arg) {
    
	std::chrono::microseconds latency_offset(latency_ * 1000); 

	auto sessionState = link_->captureAudioSessionState();
    std::chrono::microseconds prev_time = time_filter.sampleTimeToHostTime(sampleTime_) + latency_offset;
	sampleTime_ += blockTimeuSec_; /// increase sample time based on time of single callback
    std::chrono::microseconds curr_time = time_filter.sampleTimeToHostTime(sampleTime_) + latency_offset;

	if(requestStartStop_) {
		bool start=!playing_;
		requestStartStop_=false;
		if(start) {
			sessionState.setIsPlaying(true, curr_time);
			scheduleStart_ = true;
		} else {
		    writeMidiStop =true;
			sessionState.setIsPlaying(false, curr_time);
		}
	}

	if (!playing_ && sessionState.isPlaying())	{
		sessionState.requestBeatAtStartPlayingTime(0,quantum_);
		playing_ = true;
	}
	else if (playing_ && !sessionState.isPlaying())	{
		playing_ = false;
	}
    link_->commitAudioSessionState(sessionState);


    clockTrig_=false;
    barTrig_=false;
	const auto curr_beat_time = sessionState.beatAtTime(curr_time, quantum_);
	// const auto prev_beat_time = sessionState.beatAtTime(prev_time, quantum_);
	if (curr_beat_time >= 0.) { // are we pre-roll or not!
		const auto curr_phase = sessionState.phaseAtTime(curr_time, steps_per_beat_);
		const auto prev_phase = sessionState.phaseAtTime(prev_time, steps_per_beat_);
		const auto curr_step = floor(sessionState.phaseAtTime(curr_time, quantum_));
		// const auto prev_step = floor(sessionState.phaseAtTime(prev_time, quantum_));
		if (curr_phase < prev_phase) {
			// beat
			step_ = floor(curr_step);
			clockTrig_=true;
		}
		if(curr_step == 0 ) {
			// bar beat
			if(scheduleStart_) {
				scheduleStart_=false;
				writeMidiStart=true;
				writeMidiClock=true;
			}
			barTrig_=true;
		}

		// //this gives in accurate clock... perhaps ppqn calc is not accurate enough
		// const auto curr_ppqn = sessionState.phaseAtTime(curr_time, steps_per_beat_/ppqn_per_beat);
		// const auto prev_ppqn = sessionState.phaseAtTime(prev_time, steps_per_beat_/ppqn_per_beat);
		// if (curr_ppqn < prev_ppqn) {
		// 	writeMidiClock=true;
		// }

	    //calc ppqn, off prev_phase results in inaccurate/missed ticks?
		const int curr_ppqn = fmod(curr_phase * ppqn_per_beat , ppqn_per_beat);
		static int prev_ppqn=0;
		if(curr_ppqn !=prev_ppqn) {
			writeMidiClock=true;
		}
		prev_ppqn=curr_ppqn;

	} else {
		; // pre-roll
	}
}

void startStopCallback(bool isPlaying) {
	// this called when THIS client wants to start/stop, not synced
	rt_printf("start/stop - %d numpeers\n", (int) link_->numPeers());

}

void tempoCallback(double bpm) {
	rt_printf("tempo %f %d numpeers\n",bpm, (int) link_->numPeers());
}


bool setup(BelaContext *context, void *userData)
{
	Midi::createAllPorts(midiPorts_,false);
	midiEnabled_=midiPorts_.size()>0;
	
    link_ = std::unique_ptr<ableton::Link>(new ableton::Link(120));
    link_->setStartStopCallback(startStopCallback);
    link_->setTempoCallback(tempoCallback);
    link_->enable(true);
    link_->enableStartStopSync(true);
    
    auto now = link_->clock().micros();
    auto ss=link_->captureAppSessionState();
    ss.setIsPlaying(false,now);
    link_->commitAppSessionState(ss);
	
	blockTimeuSec_ = ( 1000.0f * 1000.0f  * context->audioFrames) / context->audioSampleRate;

	for(int i=0;i<2;i++) {
		pinMode(context, 0, buttonPins[i],INPUT);
	}
	
	for(int i=0;i<10;i++) {
		pinMode(context, 0, ledPins[i], OUTPUT);
	}
	
	task_=Bela_createAuxiliaryTask(link_task, 95, "link task",0);
	return true;
}


void render(BelaContext *context, void *userData)
{
	static bool buttonState[2] { false, false};
	static unsigned clockCounter=0; 
	static unsigned blinkTime=750;
	static unsigned trigTime=10;

	static unsigned trigCounter=0; 
	static unsigned barCounter=0; 

	// controls
	int latency = ((1.0f - analogRead(context,0,0) ) * 32.0f) * -1.0f ; // 0-32ms
	if(latency!=latency_) {
		latency_=latency;
		rt_printf("latency offset %d\n",latency_);
	}

	bool but0 = digitalRead(context,0,buttonPins[0]);
	if(but0 && !buttonState[0]) {
		requestStartStop_=true;
	}
	buttonState[0]=but0;

	bool but1 = digitalRead(context,0,buttonPins[1]);
	if(but1 && !buttonState[1]) {
		clockWhenStopped_=!clockWhenStopped_;
	}
	buttonState[1]=but1;


	// trig handling
	bool sendclock = clockWhenStopped_ || (playing_  && !scheduleStart_);
	if(sendclock) {
		if(clockTrig_) {
			trigCounter=trigTime;
		    clockCounter=blinkTime;
		} 
		if(barTrig_) {
			barCounter=trigTime;
		}
	}

	if(trigCounter>0) trigCounter--;
	if(barCounter>0) barCounter--;
	if(clockCounter>0) clockCounter--;

	// leds
	for(int i=0;i<4;i++) {
		digitalWrite(context, 0, ledPins[i], 0);
	}
    if(playing_  && !scheduleStart_) digitalWrite(context, 0, ledPins[step_],1);

	digitalWrite(context,0,ledPins[7],clockWhenStopped_); // send clock when stopped?
	digitalWrite(context,0,ledPins[8],playing_ || scheduleStart_ ); //play or cued  / stop 
	digitalWrite(context,0,ledPins[9],clockCounter>0); //clock
	
	for(unsigned int n = 0; n < context->analogFrames; n++) {
		analogWriteOnce(context,n,0,(trigCounter>0)); // trig on beat
		analogWriteOnce(context,n,1,playing_); // playing gate
		analogWriteOnce(context,n,2,(barCounter>0)); // trig on bar
	}

	/// midi
	if(midiEnabled_) {
		for(auto dev : midiPorts_) {
		  if(dev->isOutputEnabled()) {
		  	if(writeMidiStart) dev-> writeOutput(&midiStart, 1);
		  	if(writeMidiClock && sendclock) dev-> writeOutput(&midiClock, 1);
		  	if(writeMidiStop)  dev-> writeOutput(&midiStop, 1);
		  }
		}
	}
	writeMidiStart=false;
	writeMidiClock=false;
	writeMidiStop=false;

	Bela_scheduleAuxiliaryTask(task_);
}



void cleanup(BelaContext *context, void *userData)
{
	//Midi::destroyPorts(midiPorts_);
	link_=nullptr;
}

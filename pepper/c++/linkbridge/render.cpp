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

bool reset_flag_=true;
static unsigned blockTimeuSec_=0;
float sampleTime_=0.0f;

/// pepper pins
// ~digitalIn = [11,9,13,12];

const unsigned ledPins[] = {6,7,10,2,3,0,1,4,5,8 };
const unsigned buttonPins [] = {15,14};
	
unsigned quantum_=4;
double steps_per_beat_ = 1;
double ppqn_per_beat=24;
float curr_beat_time=0.f,prev_beat_time=0.0f;

unsigned step_=0;
unsigned curr_ppqn=0,prev_ppqn=24;

static midi_byte_t midiClock=0xF8;
static midi_byte_t midiStart=0xFA;
// static midi_byte_t midiContinue=0xFB;
static midi_byte_t midiStop=0xFC;

bool requestStartStop_=false;
bool playing_=false;


bool writeMidiStart=false;
bool writeMidiClock=false;
bool writeMidiStop=false;

int latency_=0.0f;


static void link_task(void* arg) {
    
	std::chrono::microseconds latency_offset(latency_ * 1000); 

	auto sessionState = link_->captureAudioSessionState();
	sampleTime_ += blockTimeuSec_; /// increase sample time based on time of single callback
    std::chrono::microseconds curr_time = time_filter.sampleTimeToHostTime(sampleTime_) + latency_offset;

	if(requestStartStop_) {
		bool start=!playing_;
		requestStartStop_=false;
		sessionState.setIsPlayingAndRequestBeatAtTime(start,curr_time,0,quantum_);
	}


    if( reset_flag_) {
    	sessionState.requestBeatAtTime(prev_beat_time, curr_time, quantum_);
    	curr_beat_time = sessionState.beatAtTime(curr_time, quantum_);
    	reset_flag_=false;
    } else {
    	curr_beat_time = sessionState.beatAtTime(curr_time, quantum_);
    }

    const double curr_phase = fmod(curr_beat_time, quantum_);
    
    if (curr_beat_time > prev_beat_time) {
		const double prev_phase = fmod(prev_beat_time, quantum_);
		const double prev_step = floor(prev_phase * steps_per_beat_);
		const double curr_step = floor(curr_phase * steps_per_beat_);
		
		if (prev_phase - curr_phase > quantum_ / 2 || prev_step != curr_step) {
			//?
		}

		if(step_!=fmod(curr_phase,quantum_)) {
			step_=fmod(curr_phase,quantum_);
		}
		
		const unsigned prev_ppqn = unsigned(floor(prev_phase * ppqn_per_beat));
		const double curr_ppqn = unsigned(floor(curr_phase * ppqn_per_beat));
		if(playing_ && curr_ppqn !=prev_ppqn) {
			writeMidiClock=true;
		}
	
    }
    prev_beat_time = curr_beat_time;


    link_->commitAudioSessionState(sessionState);
    
}

void startStopCallback(bool isPlaying) {
	if(isPlaying)	{
		writeMidiStart=true;
	} else {
		writeMidiStop=true;
	}
	playing_ = isPlaying;
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
	static unsigned lastStep_ = 4;
	static unsigned clockCounter=0; 
	static unsigned blinkTime=1500;

	static unsigned trigCounter=0; 

	for(int i=0;i<4;i++) {
		digitalWrite(context, 0, ledPins[i], 0);
	}
	
    if(playing_) digitalWrite(context, 0, ledPins[step_],1);
	
	bool clockTrig = lastStep_!=step_;
	if(clockTrig) {
		// rt_printf("clockTrig\n");
		lastStep_=step_;
		
		if(playing_) trigCounter=10; // only fire trig if playing
		clockCounter=blinkTime/2;
	}
	if(trigCounter>0) trigCounter--;
	if(clockCounter>0) clockCounter--;

	bool but0 = digitalRead(context,0,buttonPins[0]);
	if(but0 && !buttonState[0]) {
		requestStartStop_=true;
	}
	buttonState[0]=but0;

	bool but1 = digitalRead(context,0,buttonPins[1]);
	if(but1 && !buttonState[1]) {
		midiEnabled_=!midiEnabled_;
	}
	buttonState[1]=but1;



	digitalWrite(context,0,ledPins[9],midiEnabled_); // miid
	digitalWrite(context,0,ledPins[8],clockCounter>0); //clock
	digitalWrite(context,0,ledPins[9],playing_); //play/stop
	
	for(unsigned int n = 0; n < context->analogFrames; n++) {
		analogWriteOnce(context,n,0,(trigCounter>0));
		analogWriteOnce(context,n,1,playing_);
	}
	// latency
	int latency = ((analogRead(context,0,0) - 0.5 ) * 32.0f); // +/- 16ms
	if(latency!=latency_) {
		latency_=latency;
		rt_printf("latency offset=%d\n",latency_);
	}
	
	
	/// midi
	if(midiEnabled_) {	
		for(auto dev : midiPorts_) {
		  if(dev->isOutputEnabled()) {
		  	if(writeMidiStart) dev-> writeOutput(&midiStart, 1);
		  	if(writeMidiClock) dev-> writeOutput(&midiClock, 1);
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

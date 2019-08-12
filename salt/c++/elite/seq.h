#pragma once

#include <iostream>
#include <assert.h>

class Step {
public:
	Step() : Step(false,0.0f, 0.0f, 0.0f, 0.0f) {
		;
	}

	Step(bool a,float n,float x, float y, float z) :  
		active_(a), note_(n), x_(x), y_(y), z_(z) {
		;
	}

	Step(const Step& s) {
		active_=s.active_;
		note_=s.note_;
		x_=s.x_;
		y_=s.y_;
		z_=s.z_;
	}

	Step& operator=(const Step& s) {
		active_=s.active_;
		note_=s.note_;
		x_=s.x_;
		y_=s.y_;
		z_=s.z_;
		return *this;
	}
	
	bool active() { return active_;}
	float note() { return note_;}
    float x() { return x_;}
    float y() { return y_;}
    float z() { return z_;}

private:
	bool active_;
	float note_;
	float x_,y_,z_;
};

class Sequencer {
public:
	Sequencer() {
		tempo(120.0f);
	}

	Step& step(unsigned i) {
		assert(i<MAX_STEPS);
		return steps_[i];
	}

	void step(unsigned i, const Step& s) {
		assert(i<MAX_STEPS); 
		steps_[i] = s;
	}

	void beatDiv(unsigned bd) {
		beatDiv_ = bd;
		tempo(tempo_);
	}

	unsigned endStep() { return endStep_;}
	void endStep(unsigned es) {
		assert(es<MAX_STEPS);
		endStep_ = es;
	}


	void tempo(float t) {
		tempo_ = t;
		float div = (tempo_ / 60.0f) * (float(beatDiv_) / 4.0f);
		nSamples_ = SAMPLERATE / div;
	}

	Step& tick() {
		sampleC_ = (sampleC_+ 1) % nSamples_;
		if(sampleC_==0) {
			// beat, so advance step
			curStep_ = (curStep_+ 1) % endStep_;
		}
		return steps_[curStep_];
	}
	unsigned curStep() { return curStep_;}

private:
	static constexpr float SAMPLERATE = 48000.0f;
	static constexpr unsigned MAX_STEPS = 16;
	Step steps_[MAX_STEPS];
	unsigned curStep_=0;
	unsigned sampleC_=0;
	float    tempo_=120.0f;
	unsigned nSamples_=0;
	unsigned beatDiv_ = 16;
	unsigned endStep_ = MAX_STEPS;

};




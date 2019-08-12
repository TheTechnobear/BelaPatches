/***** splayout1.h *****/
#pragma once

#include "splayout.h"

class ZoneLayout_1 : public SPLayout {
public:
	ZoneLayout_1() {
		zoneT_.zone_=0;
	}

	unsigned signature() override { return calcSignature(1,0);}

	void touch(SPTouch& t) override{
		t.zone_=0;

		switch(pitchMode()) {
			case PitchMode::NONE : {
				t.x_ = t.x_ / 30.0f;
				break;
			}
			case PitchMode::SINGLE : {
				t.pitch_= t.x_ ;
			    t.y_=(t.y_ - 2.5f) / 2.5f; // -1...1
				t.x_ = t.x_ / 30.0f;
				if(!processTouch(t)) return;
				break;
			}
			case PitchMode::FOURTHS : {
				int row = t.y_;
				t.y_= ((t.y_ - row) * 2.0f) - 1.0f;
				t.pitch_= t.x_ + ((row - 2) * 5.0f) ;
				t.x_ = t.x_ / 30.0f;
				if(!processTouch(t)) return;
				break;
			}
		}
		SPTouch& lT = lastTouch_[t.tId_];
		lT=t;
		output(t);
	}
	
	void output(const SPTouch& t) override {
    	// pitch, y, z, x 
    	zoneT_ = t;
	}

	void render(BelaContext *context) override {
		//unused
		for(unsigned int n = 0; n < context->digitalFrames; n++) {
			digitalWriteOnce(context, n,trigOut2 ,0);	
			digitalWriteOnce(context, n,trigOut3 ,0);	
			digitalWriteOnce(context, n,trigOut4 ,0);	
		}
		for(unsigned int n = 0; n < context->analogFrames; n++) {
			analogWriteOnce(context, n, 4,0.0f);
			analogWriteOnce(context, n, 5,0.0f);
			analogWriteOnce(context, n, 6,0.0f);
			analogWriteOnce(context, n, 7,0.0f);
		}
		
		render(context,zoneT_);
	}

	void render(BelaContext *context, const SPTouch& t) {

		float pitch = t.x_;
		if(pitchMode()!=PitchMode::NONE) {
			pitch = transpose(t.pitch_, int((analogRead(context, 0, 0) - 0.5) * 6) ,-3);
		}
		
		float y = scaleY(t.y_,analogRead(context, 0, 0) * 2);
		float z = pressure(t.z_,analogRead(context, 0, 1) * 2);
		float amp = audioAmp(t.z_,analogRead(context, 0, 2) * 2);

		for(unsigned int n = 0; n < context->digitalFrames; n++) {
			digitalWriteOnce(context, n, 0 ,t.active_);	
		}

		for(unsigned int n = 0; n < context->analogFrames; n++) {
			analogWriteOnce(context, n, 0,pitch);
			analogWriteOnce(context, n, 1,y);
			analogWriteOnce(context, n, 2,z);
			analogWriteOnce(context, n, 3,t.x_);
		}

		for(unsigned int n = 0; n < context->audioFrames; n++) {
			float v0 = audioRead(context, n, 0) * amp;
			audioWrite(context, n, 0, v0);
			float v1 = audioRead(context, n, 1) * amp;
			audioWrite(context, n, 1, v1);
		}
	}

private:
	SPTouch zoneT_;
};
/***** splayout4.h *****/
#pragma once

#include "splayout.h"

class ZoneLayout_4 : public SPLayout {
public:

	ZoneLayout_4() {
		zoneT_[0].zone_=0;
		zoneT_[1].zone_=1;
		zoneT_[2].zone_=2;
		zoneT_[3].zone_=3;
	}

	unsigned signature() override { return calcSignature(2,2);}

	void touch(SPTouch& t) override {
	    float startZone=0.0f;
	    float endZone=30.0f;
	    
        if(t.x_ < zone1_) {
        	t.zone_ = 0;
			startZone=0;
			endZone=zone1_;
        } else if(t.x_ < zone2_) {
        	t.zone_ = 1;
			startZone=zone1_;
			endZone=zone2_;
        } else if(t.x_ < zone3_) {
        	t.zone_ = 2;
			startZone=zone2_;
			endZone=zone3_;
        } else {
        	t.zone_ = 3;
			startZone=zone3_;
			endZone=30.0f;
        }

		checkAndReleaseOldTouch(t);

		if(t.zone_ == 0 || t.zone_==3) {
		    t.y_=(t.y_ - 2.5f) / 2.5f; // -1...1
		} else {
			switch(pitchMode()) {
				case PitchMode::NONE : {
				    t.y_=(t.y_ - 2.5f) / 2.5f; // -1...1
					t.x_ = partialX(t.x_, startZone, endZone );
					break;
				}
				case PitchMode::SINGLE : {
				    t.y_=(t.y_ - 2.5f) / 2.5f; // -1...1
					t.pitch_= (t.x_ - startZone);
					t.x_ = partialX(t.x_, startZone, endZone );
					if(!processTouch(t)) return;
					break;
				}
				case PitchMode::FOURTHS : {
					int row = t.y_;
					t.y_= ((t.y_ - row) * 2.0f) - 1.0f;
					t.pitch_= (t.x_ - startZone) + ((row - 2)* 5.0f) ;
					if(!processTouch(t)) return;
					break;
				}
			}
		}

		SPTouch& lT = lastTouch_[t.tId_];
		lT=t;
		output(t);
	}


	void output(const SPTouch& t) override {
		zoneT_[t.zone_] = t;
	}

	
	void render(BelaContext *context) override {
		//unused
		for(int i=0;i<4;i++) renderTouch(context,zoneT_[i]);
	}

	void renderTouch(BelaContext *context,const SPTouch& t) {
		switch(t.zone_) {
			case 0 : {
				float y = scaleY(t.y_,analogRead(context, 0, 3)* 2);

				for(unsigned int n = 0; n < context->digitalFrames; n++) {
					digitalWriteOnce(context, n,trigOut2 ,t.active_);	
				}

				for(unsigned int n = 0; n < context->analogFrames; n++) {
					analogWriteOnce(context, n, 3,y);
				}
				
				break;	
			}
			case 1 : 
			case 2 : {
				int zoneOffset = (t.zone_ - 1)  * 4;
				int zoneTranspose = t.zone_ == 1 ? -1 : -3;
				
				float pitch = t.x_;
				if(pitchMode()!=PitchMode::NONE) {
					pitch = transpose(t.pitch_, int((analogRead(context, 0, 0 + zoneOffset) - 0.5) * 6) ,zoneTranspose);
				}
				float y = scaleY(t.y_,analogRead(context, 0, 1 + zoneOffset)* 2);
				float z = pressure(t.z_,analogRead(context, 0, 2 + zoneOffset) * 2);
				float amp = audioAmp(t.z_,analogRead(context, 0, 2 + zoneOffset) * 2);
				unsigned trigPin = t.zone_==1 ? trigOut1 : trigOut3;
				for(unsigned int n = 0; n < context->digitalFrames; n++) {
					digitalWriteOnce(context, n,trigPin ,t.active_);	
				}
		
				for(unsigned int n = 0; n < context->analogFrames; n++) {
					analogWriteOnce(context, n, 0 + zoneOffset,pitch);
					analogWriteOnce(context, n, 1 + zoneOffset,y);
					analogWriteOnce(context, n, 2 + zoneOffset,z);
				}
		
				for(unsigned int n = 0; n < context->audioFrames; n++) {
					float v = audioRead(context, n, t.zone_) * amp;
					audioWrite(context, n, t.zone_, v);
				}			
				break;	
			}
			case 3 : {
				float y = scaleY(t.y_,analogRead(context, 0, 7) * 2);

				for(unsigned int n = 0; n < context->digitalFrames; n++) {
					digitalWriteOnce(context, n,trigOut4 ,t.active_);	
				}

				for(unsigned int n = 0; n < context->analogFrames; n++) {
					analogWriteOnce(context, n, 7 ,y);
				}
				
				break;	
			}
			default: ;
		}
	}
	
	SPTouch zoneT_[4];

	static constexpr float zone1_ = 2.0f;
	static constexpr float zone2_ = 12.0f;
	static constexpr float zone3_ = 28.0f;
};

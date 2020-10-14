/***** splayout4xy.h *****/
#pragma once

#include "splayout.h"
#include <math_neon.h>

class ZoneLayout_4XY : public SPLayout {
public:
	ZoneLayout_4XY() {
		zoneT_[0].zone_=0;
		zoneT_[1].zone_=1;
		zoneT_[2].zone_=2;
		zoneT_[3].zone_=3;
	}

	unsigned signature() override { return calcSignature(0,2);}

	void touch(SPTouch& t) override {
	    // t.z_=t.z_;
	    float startZone=0.0f;
	    float endZone=30.0f;
        if(t.x_ < zone1_) {
			t.zone_=0;
			startZone = 0.0f;
			endZone = zone1_;
        } else if( t.x_ < zone2_) {
			t.zone_=1;
			startZone = zone1_;
			endZone = zone2_;
        } else if( t.x_ < zone3_) {
			t.zone_=2;
			startZone = zone2_;
			endZone = zone3_;
        } else {
			t.zone_=3;
		    startZone=zone3_;
		    endZone=30.0f;
        }
        
		checkAndReleaseOldTouch(t);

	    t.x_=gridX(t.x_, startZone, endZone);
	    t.y_=gridY(t.y_, 0.0f, 5.0f);

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
        unsigned xInOut = (t.zone_ * 2);
        unsigned yInOut = 1 + (t.zone_ * 2);
        float xCvIn = analogRead(context, 0, xInOut);
        float yCvIn = analogRead(context, 0, yInOut);
        
        float x = 0.0f;
        float y = 0.0f;
        float ampX = 0.0f;
        float ampY = 0.0f;
        switch (customMode_) {
            case 0: { // scaling 
                x = constrain(scaleX(t.x_,xCvIn * 2),0, 1);
                y = constrain(scaleY(t.y_,yCvIn * 2),0, 1);
                if(t.zone_==0) {
                    ampX = constrain(audioAmp((t.x_ + 1.0) / 2.0f, xCvIn * 2),0, 1);
                    ampY = constrain(audioAmp((t.y_ + 1.0) / 2.0f, xCvIn * 2),0, 1);
                }
                break;
            }
            case 1: { // offset 
                x = constrain(offsetX(t.x_, (xCvIn * 2.0f) - 1.0f), 0, 1);
                y = constrain(offsetY(t.y_, (yCvIn * 2.0f) - 1.0f), 0, 1);
                if(t.zone_==0) {
                    ampX = constrain(audioAmp((t.x_ + 1.0) / 2.0f, x),0, 1);
                    ampY = constrain(audioAmp((t.y_ + 1.0) / 2.0f, y),0, 1);
                }
                break;
            }
            case 2:
            default: {
                // this assume a unipolar input for angle!
                float angle= ((xCvIn -0.5f)* 2.0f)  * float(M_PI) * 2.0f;
                float x1 = sinf_neon(angle) * yCvIn;
                float y1 = cosf_neon(angle) * yCvIn;
                x = constrain(offsetX(t.x_, x1), 0, 1);
                y = constrain(offsetY(t.y_, y1), 0, 1);
                if(t.zone_==0) {
                    ampX = constrain(audioAmp((t.x_ + 1.0) / 2.0f, x), 0, 1);
                    ampY = constrain(audioAmp((t.y_ + 1.0) / 2.0f, y), 0, 1);
                }
                break;
            }
        }
        
        unsigned trigPin=trigOut1;
        switch(t.zone_) {
            case 0 : trigPin=trigOut1;break;
            case 1 : trigPin=trigOut2;break;
            case 2 : trigPin=trigOut3;break;
            case 3 : trigPin=trigOut4;break;
            default: break;
        }
		for(unsigned int n = 0; n < context->digitalFrames; n++) {
			digitalWriteOnce(context, n,trigPin ,t.active_);	
		}
        for(unsigned int n = 0; n < context->analogFrames; n++) {
            analogWriteOnce(context, n, xInOut,x);
            analogWriteOnce(context, n, yInOut,y);
        }

        if(t.zone_==0) {	
            for(unsigned int n = 0; n < context->audioFrames; n++) {
                float v0 = audioRead(context, n, 0) * ampX;
                audioWrite(context, n, 0, v0);
                float v1 = audioRead(context, n, 1) * ampY;
                audioWrite(context, n, 1, v1);
            }
        }
    }
	
	
	SPTouch zoneT_[4];

	static constexpr float zone1_ = 8.0f;
	static constexpr float zone2_ = 15.0f;
	static constexpr float zone3_ = 22.0f;
};

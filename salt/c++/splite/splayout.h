/***** splayout.h *****/
#pragma once

#include "defs.h"
#include "sptouch.h"

struct QuantMode {
	enum {
		NONE,
		GLIDE,
		FULL
	};
};

struct PitchMode {
	enum {
		NONE,
		SINGLE,
		FOURTHS
	};
};



class SPLayout {
public:
	SPLayout() : 
		quantMode_(QuantMode::NONE), 
		pitchMode_(PitchMode::SINGLE),
		customMode_(0)
		{ ; }
	
	virtual void touch(SPTouch& touch) = 0;
	virtual void render(BelaContext *context) = 0;
	virtual void output(const SPTouch& touch) = 0;
	virtual unsigned signature() = 0;


    void quantMode(unsigned v) { quantMode_ = v;}
    void pitchMode(unsigned v) { pitchMode_ = v;}
    unsigned pitchMode() { return pitchMode_;}

	unsigned customMode() { return customMode_;}
    void customMode(unsigned v) { customMode_ = v;}
protected:
	unsigned calcSignature(unsigned keyz, unsigned modz) {
		return (modz << 2) + keyz;
	}
	
	virtual bool processTouch(SPTouch& touch);

	void checkAndReleaseOldTouch(SPTouch& t) {
		SPTouch& lT = lastTouch_[t.tId_];
		if(lT.active_ && lT.zone_!=t.zone_) {
			SPTouch rT = lT;
			rT.active_=false;
			rT.z_=0;
			output(rT); 
		}
	}

    // increasing makes slower, more subtle light touches
	static constexpr float PRESSURE_CURVE=0.5f;

	float audioAmp(float z, float mult ) {
		return powf(z, PRESSURE_CURVE) * mult;
	}

	float pressure(float z, float mult ) {
		return ( (powf(z, PRESSURE_CURVE) * mult * ( 1.0f-ZERO_OFFSET) ) ) + ZERO_OFFSET;	
	}
	

	float scaleY(float y, float mult) {
		return ( (y * mult)  * ( 1.0f-ZERO_OFFSET) )  + ZERO_OFFSET ;	
	}

	float scaleX(float x, float mult) {
		return ( (x * mult)  * ( 1.0f-ZERO_OFFSET) )  + ZERO_OFFSET ;	
	}
	
	float gridY(float y, float start, float end) {
		float sz = (end-start) / 2.0f;
		return ((y -start ) - sz) / sz;	
	}

	float gridX(float x, float start, float end) {
		float sz = (end-start) / 2.0f;
		return ((x -start ) - sz) / sz;	
	}

	
	float offsetY(float y, float offset) {
		return ( (y + offset) * ( 1.0f-ZERO_OFFSET) )  + ZERO_OFFSET ;	
	}

	float offsetX(float x, float offset) {
		return ( (x + offset) * ( 1.0f-ZERO_OFFSET) )  + ZERO_OFFSET ;	
	}


	float partialX(float x, float startX, float endX) {
		return  (x - startX ) / (endX - startX);
	}

	
	float transpose (float pitch, int octave, int semi) {
		return pitch + (((( START_OCTAVE + octave) * 12 ) + semi) *  semiMult_ );
	}


	unsigned quantMode_;
	unsigned pitchMode_;
	unsigned customMode_;
	SPTouch lastTouch_ [MAX_TOUCH];

#ifdef SALT
	static constexpr float 	OUT_VOLT_RANGE=10.0f;
	static constexpr float 	ZERO_OFFSET=0.5f;
	static constexpr int   	START_OCTAVE=5;
#else 
	static constexpr float 	OUT_VOLT_RANGE=5.0f;
	static constexpr float 	ZERO_OFFSET=0;
	static constexpr int 	START_OCTAVE=1.0f;
#endif 
	static constexpr float semiMult_ = (1.0f / (OUT_VOLT_RANGE * 12.0f)); // 1.0 = 10v = 10 octaves 
};

bool SPLayout::processTouch(SPTouch& t)  {
	// has to be called after 'zoning'

	SPTouch& lT = lastTouch_[t.tId_];
	if(t.active_) {
		if(!lT.active_) {
			// new touch
			if(quantMode_!=QuantMode::NONE) {
				t.pitch_ = int(t.pitch_) * semiMult_;
			} else {
				t.pitch_ = (t.pitch_ - 0.5f)  * semiMult_ ;
			}
			t.scalePitch_ = t.pitch_;
			t.vibPitch_ = t.pitch_;
		} else {
			// continued touch;
			switch(quantMode_) {
				case QuantMode::FULL : { 
					t.pitch_ = int(t.pitch_) * semiMult_;
					t.scalePitch_ = t.pitch_;
					t.vibPitch_ = t.pitch_;
					break;
				}	
				case QuantMode::GLIDE : { 
					float pitch = t.pitch_* semiMult_;
					float scaleDiff = ((int(t.pitch_) * semiMult_) - lT.scalePitch_) * 0.025f;
					float vibDiff = (pitch - lT.vibPitch_) * 0.25f;
					t.vibPitch_ = lT.vibPitch_ + vibDiff;
					t.scalePitch_ = lT.scalePitch_ + scaleDiff;
					float vib = (pitch - t.vibPitch_) * 5.0f;
					t.pitch_ = t.scalePitch_ + vib;
					break;
				}
				
				case QuantMode::NONE : 
				default: {
					t.pitch_ = (t.pitch_ - 0.5f)  * semiMult_ ;
					t.scalePitch_ = t.pitch_;
					t.vibPitch_ = t.pitch_;
					break;
				}
			}
		}
	} else {
		if(lT.active_) {
			if(quantMode_!=QuantMode::NONE) {
				t.pitch_ = int(t.pitch_) * semiMult_;
			} else {
				t.pitch_ = (t.pitch_ - 0.5f)  * semiMult_ ;
			}
		} else {
			return false;
		}
	}
	return true;
}


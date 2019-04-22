#include <Bela.h>

#include <SPLiteDevice.h>

#include <chrono>
#include <math.h>

AuxiliaryTask gSPLiteProcessTask;


std::chrono::time_point<std::chrono::system_clock>  gStartTime;
std::chrono::time_point<std::chrono::system_clock>  gLastErrorTime;

#include <iostream>

#define MAX_TOUCH 4

enum
{
	switch1 = 6,
	trigIn1 = 15,
	trigIn2 = 14,
	trigIn3 = 1,
	trigIn4 = 3,
	trigOut1 = 0,
	trigOut2 = 5,
	trigOut3 = 12,
	trigOut4 = 13,
	ledOut1 = 2,
	ledOut2 = 4,
	ledOut3 = 8,
	ledOut4 = 9,
	pwmOut = 7,
};

void setLed(BelaContext* context, int ledPin,  int color)
{
	if(color == 0)
	{
		pinMode(context, 0, ledPin, INPUT);
		return;
	}
	pinMode(context, 0, ledPin, OUTPUT);
	digitalWrite(context, 0, ledPin, color - 1);
}

void drivePwm(BelaContext* context, int pwmPin)
{
	static unsigned int count = 0;
	pinMode(context, 0, pwmPin, OUTPUT);
	for(unsigned int n = 0; n < context->digitalFrames; ++n)
	{
		digitalWriteOnce(context, n, pwmPin, count & 1);
		count++;
	}
}



class BelaIO {
public: 
	BelaIO() {
		digitalOutPin_[0] = trigOut1;
		digitalOutPin_[1] = trigOut2;
		digitalOutPin_[2] = trigOut3;
		digitalOutPin_[3] = trigOut4;
	}

	unsigned numDigitalOuts() {return MAX_DIGITAL_OUT;}
	bool 	 digitalOut(unsigned idx) { return digitalOut_[idx];}
	void 	 digitalOut(unsigned idx, bool value) { digitalOut_[idx]=value;}
	unsigned digitalOutPin(unsigned idx) { return digitalOutPin_[idx];}
	unsigned numAnalogOut() {return MAX_ANALOG_OUT;}
	float 	 analogOut(unsigned idx) { return analogOut_[idx];}
	void 	 analogOut(unsigned idx, float value) { analogOut_[idx]=value;}
		
	
private:
	static constexpr unsigned MAX_DIGITAL_OUT=4;
	static constexpr unsigned MAX_ANALOG_OUT=8;
	bool digitalOut_[MAX_DIGITAL_OUT];
	unsigned digitalOutPin_[MAX_DIGITAL_OUT];
	float analogOut_[MAX_ANALOG_OUT];
};

static BelaIO belaio_;

struct SPTouch {
	SPTouch() : 
		SPTouch(0,false,
				0.0f,0.0f,0.0f) {
		;
	}
	
	SPTouch(unsigned  tId, bool active,float x, float y, float z) 
	: 	tId_(tId), active_(active),
		x_(x), y_(y), z_(z) , 
		pitch_(0.0f), 
		scalePitch_(0.0f), vibPitch_(0.0f), 
		zone_(0) {
		;		
	}
	
	SPTouch(const SPTouch& t) 
	: 	tId_(t.tId_), active_(t.active_),
		x_(t.x_), y_(t.y_), z_(t.z_) , 
		pitch_(t.pitch_),  
		scalePitch_(t.scalePitch_), vibPitch_(t.vibPitch_),
		zone_(t.zone_) {
		;		
	}

	SPTouch& operator=(const SPTouch& t) {
		tId_ = t.tId_;
		active_=t.active_;
		x_=t.x_;
		y_=t.y_;
		z_=t.z_;
		pitch_=t.pitch_;
		scalePitch_=t.scalePitch_;
		vibPitch_=t.vibPitch_;
		zone_= t.zone_;
		return *this;
	}
	
	unsigned tId_;
	bool active_;
	float x_;
	float y_;
	float z_;
	
	float pitch_;
	float scalePitch_;
	float vibPitch_;
	unsigned  zone_;
	float vib_;
};


struct QuantMode {
	enum {
		NONE,
		GLIDE,
		FULL
	};
};

class SPLayout {
public:
	SPLayout() : quantMode_(QuantMode::NONE) { ; }
	
	// called with touch
	virtual void processTouch(SPTouch& touch);
	virtual void touch(SPTouch& touch) = 0;
	
	// prepare outputs
	virtual void output(const SPTouch& touch) = 0;

    void quantMode(unsigned v) { quantMode_ = v;}

protected:

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

	float pressure(float z) {
		return (powf(z, PRESSURE_CURVE) * ( 1.0f-ZERO_OFFSET))  + ZERO_OFFSET;	
	}
	
	float fullY(float y) {
		return y / numRows_;	
	}
	float partialX(float x, float startX, float endX) {
		return  (x - startX ) / (endX - startX);
	}

	
	float transpose (int cSemiPos) {
		return ( (octaveTranspose_ * 12 ) + cSemiPos) *  semiMult_;
	}


	unsigned quantMode_;
	SPTouch lastTouch_ [MAX_TOUCH];
	int octaveTranspose_ = DEFAULT_OCTAVE;
	int semiTranspose_ = -3 ; // SP starts on A, in full layout
	float numRows_ = 5.0f;

#ifdef SALT
	static constexpr float OUT_VOLT_RANGE=10.0f;
	static constexpr float ZERO_OFFSET=0.5f;
	static constexpr int   DEFAULT_OCTAVE=5;
#else 
	static constexpr float OUT_VOLT_RANGE=5.0f;
	static constexpr float ZERO_OFFSET=0;
	static constexpr int DEFAULT_OCTAVE=1f;
#endif 
	static constexpr float semiMult_ = (1.0f / (OUT_VOLT_RANGE * 12.0f)); // 1.0 = 10v = 10 octaves 
};

void SPLayout::processTouch(SPTouch& t)  {
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
			lT = t;
			output(t);
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
			lT = t;
			
			output(t);
		}
	} else {
		if(lT.active_) {
			if(quantMode_!=QuantMode::NONE) {
				t.pitch_ = int(t.pitch_) * semiMult_;
			} else {
				t.pitch_ = (t.pitch_ - 0.5f)  * semiMult_ ;
			}
			lT=t;
 			output(t);
		}
	}
}



class ZoneLayout_1 : public SPLayout {
public:
	void touch(SPTouch& t) override{
		t.zone_=0;
		t.pitch_= t.x_ ;
		t.x_=t.x_;
	    t.y_=fullY(t.y_);
	    t.z_=pressure(t.z_);
		processTouch(t);
	}
	
	void output(const SPTouch& t) override {
    	// pitch, y, z, x 
        belaio_.digitalOut(0, t.active_);
        belaio_.analogOut(0, t.pitch_ + transpose(-3)); // C=3
        belaio_.analogOut(1, t.y_);
        belaio_.analogOut(2, t.z_);
        belaio_.analogOut(3, t.x_);
	}
};

class ZoneLayout_2 : public SPLayout {
public:
	void touch(SPTouch& t) override{
	    t.y_=fullY(t.y_);
	    t.z_=pressure(t.z_);
        if(t.x_ < zone1_) {
			t.zone_=0;
			t.pitch_= t.x_;
			t.x_=partialX(t.x_, 0.0f, zone1_ );
        } else {
			t.zone_=1;
			t.pitch_= t.x_ - zone1_;
			t.x_=partialX(t.x_, zone1_, 30.0f );
        }
		checkAndReleaseOldTouch(t);
		processTouch(t);
	}
	
	void output(const SPTouch& t) override {
		switch(t.zone_) {
	        case 0 : {
	        	// pitch, y, z, x
                belaio_.digitalOut(0, t.active_);
                belaio_.analogOut(0, t.pitch_ + transpose(-3));
                belaio_.analogOut(1, t.y_);
                belaio_.analogOut(2, t.z_);
                belaio_.analogOut(3, t.x_);
	      		break;
	        }; 
	        default : {
                belaio_.digitalOut(2, t.active_);
                belaio_.analogOut(4, t.pitch_ + transpose(-3));
                belaio_.analogOut(5, t.y_);
                belaio_.analogOut(6, t.z_);
                belaio_.analogOut(7, t.x_);
	        }
		}
	}
	static constexpr float zone1_ = 12.0f;
};



class ZoneLayout_4 : public SPLayout {
public:
	void touch(SPTouch& t) override {
	    t.y_=fullY(t.y_);
	    t.z_=pressure(t.z_);
        if(t.x_ < zone1_) {
			t.zone_=0;
			checkAndReleaseOldTouch(t);
			output(t);
        } else if(t.x_ < zone2_) {
			t.zone_=1;
			t.pitch_= t.x_-zone1_;
			t.x_=partialX(t.x_, zone1_, zone2_ );
			checkAndReleaseOldTouch(t);
			processTouch(t);
        } else if(t.x_ < zone3_) {
			t.zone_=2;
			t.pitch_= t.x_- zone2_;
			t.x_=partialX(t.x_, zone2_, zone3_ );
			checkAndReleaseOldTouch(t);
			processTouch(t);
        } else {
			t.zone_=3;
			checkAndReleaseOldTouch(t);
			output(t);
        }
	}

	void output(const SPTouch& t) override {
		switch(t.zone_) {
			case 0 : {
                belaio_.digitalOut(1, t.active_);
                belaio_.analogOut(3, t.y_);
				break;	
			}
			case 1 : {
                belaio_.digitalOut(0, t.active_);
                belaio_.analogOut(0, t.pitch_ + transpose(-1));
                belaio_.analogOut(1, t.y_);
                belaio_.analogOut(2, t.z_);
				break;	
			}
			case 2 : {
                belaio_.digitalOut(2, t.active_);
                belaio_.analogOut(4, t.pitch_ + transpose(-3));
                belaio_.analogOut(5, t.y_);
                belaio_.analogOut(6, t.z_);
				break;	
			}
			default:
			case 3 : {
                belaio_.digitalOut(3, t.active_);
                belaio_.analogOut(7, t.y_);
				break;	
			}
		}
	}
	
	static constexpr float zone1_ = 2.0f;
	static constexpr float zone2_ = 12.0f;
	static constexpr float zone3_ = 28.0f;
};

class ZoneLayout_1Fourths : public ZoneLayout_1 {
public:
	void touch(SPTouch& t) override{
		t.zone_=0;
		int row = t.y_;
		
		t.pitch_= t.x_ + (row * 5.0f) ;
		t.x_=t.x_;
	    t.y_=t.y_ - row;
	    t.z_=pressure(t.z_);
		processTouch(t);
	}
};

class ZoneLayout_2Fourths : public ZoneLayout_2 {
public:
	void touch(SPTouch& t) override{
		int row = t.y_;
	    t.y_=t.y_ - row;
	    t.z_=pressure(t.z_);
        if(t.x_ < zone1_) {
			t.zone_=0;
			t.pitch_= t.x_ + (row * 5.0f);
			t.x_=partialX(t.x_, 0.0f, zone1_ );
        } else {
			t.zone_=1;
			t.pitch_= t.x_ - zone1_ + + (row * 5.0f);
			t.x_=partialX(t.x_, zone1_, 30.0f );
        }
		checkAndReleaseOldTouch(t);
		processTouch(t);
	}
};


class BelaSPCallback : public SPLiteCallback {
public:
	BelaSPCallback() : layoutIdx_(0), quantMode_(QuantMode::GLIDE) {
		gStartTime = std::chrono::system_clock::now();
		layouts_.push_back(new ZoneLayout_1());
		layouts_.push_back(new ZoneLayout_2());
		layouts_.push_back(new ZoneLayout_4());
		layouts_.push_back(new ZoneLayout_1Fourths());
		layouts_.push_back(new ZoneLayout_2Fourths());
		layouts_[layoutIdx_]->quantMode(quantMode_);
	}
	
    void onError(unsigned, const char* err) override {
		gLastErrorTime = std::chrono::system_clock::now();
		std::chrono::duration<double> diff = gLastErrorTime-gStartTime;
		std::cerr << diff.count() << " secs " <<err << std::endl;
    }
    
   void touchOn(unsigned  tId, float x, float y, float z) override {
        updateOutput(tId,true,x,y,z);
    }

    void touchContinue(unsigned tId, float x, float y, float z) override {
        // std::cout << " touchContinue:" << tId << " x:" << x  << " y:" << y << " z:" << z << std::endl;
        updateOutput(tId,true, x,y,z);

    }

    void touchOff(unsigned tId, float x, float y, float z) override {
        // std::cout << " touchOff:" << tId << " x:" << x  << " y:" << y << " z:" << z << std::endl;
        updateOutput(tId,false, x,y,0.0f);
    }

    unsigned layout() { return layoutIdx_;}

    void switchLayout(unsigned idx) {
    	if(idx< layouts_.size()) layoutIdx_ = idx;
    	// TODO clear values?
    }
    
    void nextLayout() {
    	layoutIdx_ = ++layoutIdx_ % layouts_.size();
		layouts_[layoutIdx_]->quantMode(quantMode_);
    }
    
    unsigned quantMode() { return quantMode_;}
    void nextQuantMode() {
    	quantMode_ = ++quantMode_ % 3;
		layouts_[layoutIdx_]->quantMode(quantMode_);
    }

private:
	void updateOutput(unsigned  tId, bool active,float x, float y, float z) {
		SPTouch touch(tId,active,x,y,z * 0.6f);
		layouts_[layoutIdx_]->touch(touch);
	}
	unsigned  layoutIdx_;
	unsigned  quantMode_;
	std::vector<SPLayout*> layouts_;
};

SPLiteDevice *gpDevice = nullptr;
auto gCallback = std::make_shared<BelaSPCallback>();

void process_salt(void*) {
	gpDevice->process();
}

bool setup(BelaContext *context, void *userData)
{
	pinMode(context,0,trigIn1,INPUT);
	pinMode(context,0,trigIn2,INPUT);
	pinMode(context,0,trigIn3,INPUT);
	pinMode(context,0,trigIn4,INPUT);


	for(unsigned i = 0; i< belaio_.numDigitalOuts(); i++) {
	 	pinMode(context, 0, belaio_.digitalOutPin(i), OUTPUT);
	}

	gpDevice = new SPLiteDevice();
    gpDevice->addCallback(gCallback);
    gpDevice->maxTouches(MAX_TOUCH);
    gpDevice->start();
	
    // Initialise auxiliary tasks
	if((gSPLiteProcessTask = Bela_createAuxiliaryTask( process_salt, BELA_AUDIO_PRIORITY - 5, "SPLiteProcessTask", gpDevice)) == 0)
		return false;

	return true;
}




// render is called 2750 per second (44000/16)
// const int decimation = 5;  // = 550/seconds
long renderFrame = 0;
void render(BelaContext *context, void *userData)
{
#ifdef SALT
	drivePwm(context,pwmOut);

	static unsigned lsw,/*ltr1,*/ ltr2,ltr3,ltr4;
	static unsigned led_mode=0; // 0 = normal
	static unsigned led_counter=0;

	unsigned sw  = digitalRead(context, 0, switch1);  //next layout
	// unsigned tr1 = digitalRead(context, 0, trigIn1);  
	unsigned tr2 = digitalRead(context, 0, trigIn2);  //quantize 
	unsigned tr3 = digitalRead(context, 0, trigIn3);
	unsigned tr4 = digitalRead(context, 0, trigIn4);

	if(sw  && !lsw)  { 
		gCallback->nextLayout(); 
		led_counter=2000;
		led_mode=1;
	}
	
	if(led_mode==1) {
		led_counter--;
		if(led_counter>0) {
			unsigned layout = (gCallback->layout() + 1) % 16 ;
			setLed(context, ledOut1, (layout & 8) > 0 ) ;
			setLed(context, ledOut2, (layout & 4) > 0);
			setLed(context, ledOut3, (layout & 2) > 0);
			setLed(context, ledOut4, (layout & 1) > 0);
		} else {
			led_mode=0;
			led_counter=0;
			setLed(context, ledOut1, 0);
			setLed(context, ledOut2, 0);
			setLed(context, ledOut3, 0);
			setLed(context, ledOut4, 0);
		}
	}

	if(tr2  && !ltr2)  { gCallback->nextQuantMode(); }

	if(led_mode==0) {
		setLed(context, ledOut2, gCallback->quantMode() %3);
	}

	lsw =  sw;
	// ltr1 = tr1;
	ltr2 = tr2;
	ltr3=  tr3;
	ltr4=  tr4;
#endif

		
	
	renderFrame++;
	// silence audio buffer
	for(unsigned int n = 0; n < context->audioFrames; n++) {
		for(unsigned int channel = 0; channel < context->audioOutChannels; channel++) {
			audioWrite(context, n, channel, 0.0f);
		}
	}
	
	// if(renderFrame % 5 == 0) {
	    // distribute touches to cv
		Bela_scheduleAuxiliaryTask(gSPLiteProcessTask);
	// }

	for(unsigned int i = 0; i < belaio_.numDigitalOuts();i++) {
		unsigned pin= belaio_.digitalOutPin(i);
		if(pin < context->digitalChannels) { 
			bool value = belaio_.digitalOut(i);
			for(unsigned int n = 0; n < context->digitalFrames; n++) {
				digitalWriteOnce(context, n, pin ,value);	
			}
		}
	}

	for(unsigned int i = 0; i < belaio_.numAnalogOut();i++) {
		float value = belaio_.analogOut(i);
		for(unsigned int n = 0; n < context->analogFrames; n++) {
			analogWriteOnce(context, n, i,value );
		}
	}
}

void cleanup(BelaContext *context, void *userData)
{
// 	// gpDevice->removeCallback(gCallback);
	gpDevice->stop();
	delete gpDevice;
}
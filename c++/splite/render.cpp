#include <Bela.h>

#include <SPLiteDevice.h>

#include <chrono>

AuxiliaryTask gSPLiteProcessTask;


std::chrono::time_point<std::chrono::system_clock>  gStartTime;
std::chrono::time_point<std::chrono::system_clock>  gLastErrorTime;

#include <iostream>

struct SPZone {
	SPZone() : id_(0), active_(false), x_(0.0f),y_(0.0f),z_(0.0f), pitch_(0.0f) {;}
	void reset() { id_=0;active_=false;x_=0.0f;y_ = 0.0f;z_ = 0.0f; pitch_=0.0f;}
	
	unsigned id_;
	bool active_;
	float x_,y_,z_;
	float pitch_;
} zone_[4];

#ifdef SALT
	#define OUT_VOLT_RANGE 10.0f
	#define OUT_OFFSET 0.5f
#else 
	#define OUT_VOLT_RANGE 5.0f
	#define OUT_OFFSET 0.0f
#endif 

class ZoneMode {
	virtual void updateTouch(unsigned  tId, bool active,float x, float y, float z);
}

class 1ZoneMode : public ZoneMode {
	void updateTouch(unsigned  tId, bool active,float x, float y, float z) override {
        	zone[1]_.id_ = tId;
        	zone[1]_.active_ = active;
        	zone[1]_.x_ = x  / 10.0f ;
        	zone[1]_.y_ = y / 5.0f;
        	zone[1]_.z_ = ((z / 2.0) * 2.0f)  + OUT_OFFSET;
        	zone[1]_.pitch_ =  (x * semiMult) + OUT_OFFSET;
	}
}

class 4ZoneMode : public ZoneMode {
	void updateTouch(unsigned  tId, bool active,float x, float y, float z) override {
        if(x < 2.0f) {
        	zone[0]_.id_ = tId;
        	zone[0]_.active_ = active;
        	zone[0]_.y_ = y / 5.0f;
        } else if(x < 12.0f) {
        	zone[1]_.id_ = tId;
        	zone[1]_.active_ = active;
        	zone[1]_.x_ = (x - 2.0f)  / 10.0f ;
        	zone[1]_.y_ = y / 5.0f;
        	zone[1]_.z_ = ((z / 2.0) *2.0f)  + OUT_OFFSET;
        	zone[1]_.pitch_ =  ((x - 2.0f)  * semiMult) + OUT_OFFSET;
        } else if(x < 28.0f) {
        	zone[2]_.id_ = tId;
        	zone[2]_.active_ = active;
        	zone[2]_.x_ = (x - 12.0f)  / 16.0f;
        	zone[2]_.y_ = y / 5.0f;
        	zone[2]_.z_ = ((z / 2.0) * 2.0f) + OUT_OFFSET;
        	zone[2]_.pitch_ =  ((x - 12.0f)  * semiMult) + OUT_OFFSET;
        } else {
        	zone[3]_.id_ = tId;
        	zone[3]_.active_ = active;
        	zone[3]_.y_ = y / 5.0f;
        }
	}
}

ZoneMode* mode_[2] = {
	new 1ZoneMode(),
	new 4ZoneMode()
};

class BelaSPCallback : public SPLiteCallback {
public:
	BelaSPCallback() {
		gStartTime = std::chrono::system_clock::now();
	}
    void onError(unsigned, const char* err) override {
		gLastErrorTime = std::chrono::system_clock::now();
		std::chrono::duration<double> diff = gLastErrorTime-gStartTime;
		std::cerr << diff.count() << " secs " <<err << std::endl;
    }
    
   void touchOn(unsigned  tId, float x, float y, float z) override {
        // std::cout << " touchOn:" << tId << " x:" << x  << " y:" << y << " z:" << z << std::endl;
        updateTouch(tId,true,x,y,z);
    }

    void touchContinue(unsigned tId, float x, float y, float z) override {
        // std::cout << " touchContinue:" << tId << " x:" << x  << " y:" << y << " z:" << z << std::endl;
        updateTouch(tId,true, x,y,z);

    }

    void touchOff(unsigned tId, float x, float y, float z) override {
        // std::cout << " touchOff:" << tId << " x:" << x  << " y:" << y << " z:" << z << std::endl;
        updateTouch(tId,false, x,y,0.0f);
    }
private:
	static constexpr float semiMult = (1.0f / (OUT_VOLT_RANGE * 12.0f)); // 1.0 = 10v = 10 octaves 
	void updateTouch(unsigned  tId, bool active,float x, float y, float z) {
		mode_[modeIdx_]->updateTouch(tId,active,x,z,y);
	}
	unsigned modeIdx_;	

};

SPLiteDevice *gpDevice = nullptr;
auto gCallback = std::make_shared<BelaSPCallback>();

void process_salt(void*) {
	gpDevice->process();
}

bool setup(BelaContext *context, void *userData)
{
	pinMode(context, 0, 0, OUTPUT); 
	pinMode(context, 0, 5, OUTPUT); 
	pinMode(context, 0, 12, OUTPUT); 
	pinMode(context, 0, 13, OUTPUT); 
	
	gpDevice = new SPLiteDevice();
    gpDevice->addCallback(gCallback);
    gpDevice->maxTouches(4);
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
	
	for(unsigned int n = 0; n < context->digitalFrames; n++) {
		if(0  < context->digitalChannels) { digitalWriteOnce(context, n, 0,    zone[0].active_);}	// trig1
		if(5  < context->digitalChannels) { digitalWriteOnce(context, n, 5,    zone[1].active_);}	// trig2
		if(12  < context->digitalChannels) { digitalWriteOnce(context, n, 12,  zone[2].active_);}	// trig3
		if(13  < context->digitalChannels) { digitalWriteOnce(context, n, 13,  zone[3].active_);}	// trig4
	}

	for(unsigned int n = 0; n < context->analogFrames; n++) {
		unsigned ch = 0;
		if(ch < context->analogOutChannels) { analogWriteOnce(context, n, ch, zone[0].y_); ch++; }		// CV 1
		if(ch < context->analogOutChannels) { analogWriteOnce(context, n, ch, zone[1].pitch_ ); ch++; }	// CV 2
		if(ch < context->analogOutChannels) { analogWriteOnce(context, n, ch, zone[1].y_ ); ch++; } 	// CV 3
		if(ch < context->analogOutChannels) { analogWriteOnce(context, n, ch, zone[1].z_ ); ch++; } 	// CV 4
		if(ch < context->analogOutChannels) { analogWriteOnce(context, n, ch, zone[2].pitch_ ); ch++; } // CV 5
		if(ch < context->analogOutChannels) { analogWriteOnce(context, n, ch, zone[2].y_ ); ch++; }		// CV 6
		if(ch < context->analogOutChannels) { analogWriteOnce(context, n, ch, zone[2].z_ ); ch++; } 	// CV 7
		if(ch < context->analogOutChannels) { analogWriteOnce(context, n, ch, zone[3].y_); ch++; } 	// CV 8
	}

}

void cleanup(BelaContext *context, void *userData)
{
// 	// gpDevice->removeCallback(gCallback);
	gpDevice->stop();
	delete gpDevice;
}
#include <Bela.h>

#include <SPLiteDevice.h>

#include <chrono>

AuxiliaryTask gSPLiteProcessTask;


std::chrono::time_point<std::chrono::system_clock>  gStartTime;
std::chrono::time_point<std::chrono::system_clock>  gLastErrorTime;

#include <iostream>

struct SPTouch {
	unsigned id_;
	bool active_;
	float x_,y_,z_;
	float pitch_;
} ;

SPTouch lSlider_ 	{0,false,0.0f,0.0f,0.0f,0.0f};
SPTouch lPanel_  	{0,false,0.0f,0.0f,0.0f,0.0f};
SPTouch rPanel_ 	{0,false,0.0f,0.0f,0.0f,0.0f};
SPTouch rSlider_ 	{0,false,0.0f,0.0f,0.0f,0.0f};

#ifdef SALT
	#define OUT_VOLT_RANGE 10.0f
	#define OUT_OFFSET 0.5f
#else 
	#define OUT_VOLT_RANGE 5.0f
	#define OUT_OFFSET 0.0f
#endif 

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
        updateTouch(tId,x,y,z);
    }

    void touchContinue(unsigned tId, float x, float y, float z) override {
        // std::cout << " touchContinue:" << tId << " x:" << x  << " y:" << y << " z:" << z << std::endl;
        updateTouch(tId,x,y,z);

    }

    void touchOff(unsigned tId, float x, float y, float z) override {
        // std::cout << " touchOff:" << tId << " x:" << x  << " y:" << y << " z:" << z << std::endl;
        updateTouch(tId,x,y,0.0f);
    }
private:
	static constexpr float semiMult = (1.0f / (OUT_VOLT_RANGE * 12.0f)); // 1.0 = 10v = 10 octaves 
	void updateTouch(unsigned  tId, float x, float y, float z) {
        if(x < 2.0f) {
        	lSlider_.id_ = tId;
        	lSlider_.y_ = y / 5.0f;
        } else if(x < 12.0f) {
        	lPanel_.id_ = tId;
        	lPanel_.x_ = (x - 2.0f)  / 10.0f ;
        	lPanel_.y_ = y / 5.0f;
        	lPanel_.z_ = ((z / 2.0) *2.0f)  + OUT_OFFSET;
        	lPanel_.pitch_ =  ((x - 2.0f)  * semiMult) + OUT_OFFSET;
        } else if(x < 28.0f) {
        	rPanel_.id_ = tId;
        	rPanel_.x_ = (x - 12.0f)  / 16.0f;
        	rPanel_.y_ = y / 5.0f;
        	rPanel_.z_ = ((z / 2.0) * 2.0f) + OUT_OFFSET;
        	rPanel_.pitch_ =  ((x - 12.0f)  * semiMult) + OUT_OFFSET;
        } else {
        	rSlider_.id_ = tId;
        	rSlider_.y_ = y / 5.0f;
        }
	}

};

SPLiteDevice *gpDevice = nullptr;
auto gCallback = std::make_shared<BelaSPCallback>();

void process_salt(void*) {
	gpDevice->process();
}

bool setup(BelaContext *context, void *userData)
{
	gpDevice = new SPLiteDevice();
    gpDevice->addCallback(gCallback);
    gpDevice->maxTouches(4);
    gpDevice->start();
	
    // Initialise auxiliary tasks

	// if((gSPLiteProcessTask = Bela_createAuxiliaryTask( process_salt, 0, "SPLiteProcessTask", gpDevice)) == 0)
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

	for(unsigned int n = 0; n < context->analogFrames; n++) {
		unsigned ch = 0;
		if(ch < context->analogOutChannels) { analogWriteOnce(context, n, ch, lSlider_.y_); ch++; };// CV 1
		if(ch < context->analogOutChannels) { analogWriteOnce(context, n, ch, lPanel_.pitch_ ); ch++; };// CV 2
		if(ch < context->analogOutChannels) { analogWriteOnce(context, n, ch, lPanel_.y_ ); ch++; };// CV 3
		if(ch < context->analogOutChannels) { analogWriteOnce(context, n, ch, lPanel_.z_ ); ch++; };// CV 4
		if(ch < context->analogOutChannels) { analogWriteOnce(context, n, ch, rPanel_.pitch_ ); ch++; };// CV 5
		if(ch < context->analogOutChannels) { analogWriteOnce(context, n, ch, rPanel_.y_ ); ch++; };// CV 6
		if(ch < context->analogOutChannels) { analogWriteOnce(context, n, ch, rPanel_.z_ ); ch++; };// CV 7
		if(ch < context->analogOutChannels) { analogWriteOnce(context, n, ch, rSlider_.y_); ch++; };// CV 8
	}

}

void cleanup(BelaContext *context, void *userData)
{
// 	// gpDevice->removeCallback(gCallback);
	gpDevice->stop();
	delete gpDevice;
}
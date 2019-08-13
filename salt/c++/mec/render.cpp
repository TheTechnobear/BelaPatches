#include <Bela.h>


#include <mec_api.h>
#include <math.h>

#include "defs.h"

mec::MecApi* 	gMecApi=NULL;
class BelaMecCallback;
BelaMecCallback* 	gMecCallback=NULL;


static constexpr BREATH_CTRLID=0;
static constexpr RIBBON_1_CTRLID=0x10; 
static constexpr RIBBON_2_CTRLID=0x11;


AuxiliaryTask gMecProcessTask;


class BelaMecCallback : public mec::Callback {
public:
	BelaMecCallback()  {
		;
	}
	
    void touchOn(int touchId, float note, float x, float y, float z) override {
       //rt_printf("touchOn %i , %f, %f %f %f\n", touchId,note,x,y,z);
		if(!active_) {
			active_=true;
			touchId_=touchId;
			note_=note;
			x_=x;
			y_=y;
			z_=z;
		}
    }
    

    void touchContinue(int touchId, float note, float x, float y, float z) override {
		if(active_ && touchId==touchId_) {
			note_=note;
			x_=x;
			y_=y;
			z_=z;
        }
    }

    void touchOff(int touchId, float note, float x, float y, float z) override {
		if(active_ && touchId==touchId_) {
			active_=false;
			note_=note;
			x_=x;
			y_=y;
			z_=z;
		}
    }


    void control(int ctrlId, float v) override {
    	// rt_printf("control %i , %f\n", ctrlId,v);
    	switch (ctrlId) {
    		case BREATH_CTRLID : {
    			breath_=v;
    			break;
    		}
    		case RIBBON_1_CTRLID : {
    			ribbon_[0]=v;
    			break;
    		}
            case RIBBON_2_CTRLID : {
                ribbon_[1]=v;
                break;
            }
    	}
    }
    
    void render(BelaContext *context) {
        bool d[4];
        d[0] = active_;
        d[1] = false;
        d[2] = false;
        d[3] = false;


        for(unsigned int n = 0; n < context->digitalFrames; n++) {
            digitalWriteOnce(context, n, trigOut1 ,d[0]);
            digitalWriteOnce(context, n, trigOut2 ,d[1]);
            digitalWriteOnce(context, n, trigOut3 ,d[2]);
            digitalWriteOnce(context, n, trigOut4 ,d[3]);
        }


        float note      = transpose(note_, int((analogRead(context, 0, 0) - 0.5) * 6) ,-3);
        float y         = scaleY(y_,            analogRead(context, 0, 1) * 2);
        float z         = pressure(z_,          analogRead(context, 0, 2) * 2);
        float amp       = audioAmp(z_,          analogRead(context, 0, 2) * 2);
        float breath    = scaleBreath(breath_,  analogRead(context, 0, 3) * 2);
        float ribbon1   = scaleRibbon(ribbon[0]_,  analogRead(context, 0, 4) * 2);
        float ribbon2   = scaleRibbon(ribbon[0]_,  analogRead(context, 0, 5) * 2);

        float a[8];
        a[0] = note;
		a[1] = y;
		a[2] = z;
	    a[3] = breath;
		a[4] = ribbon1;
		a[5] = ribbon2;
		a[6] = 0.0f;
        a[7] = 0.0f;
	

		for(unsigned int n = 0; n < context->analogFrames; n++) {
            for(unsigned int i=0;i<8;i++) {
    			analogWriteOnce(context, n, i,a[i]);
            }
		}    	

        for(unsigned int n = 0; n < context->audioFrames; n++) {
            float v0 = audioRead(context, n, 0) * amp;
            audioWrite(context, n, 0, v0);
            float v1 = audioRead(context, n, 1) * amp;
            audioWrite(context, n, 1, v1);
        }
    }
    
    
private: 

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

    float scaleBreath(float x, float mult) {
        return ( (x * mult)  * ( 1.0f-ZERO_OFFSET) )  + ZERO_OFFSET ;   
    }

    float scaleRibbon(float x, float mult) {
        return ( (x * mult)  * ( 1.0f-ZERO_OFFSET) )  + ZERO_OFFSET ;   
    }
	
	float transpose (float pitch, int octave, int semi) {
        return (pitch + semi + (( START_OCTAVE + octave) * 12.0f )) *  semiMult_ ;
	}


#ifdef SALT
	static constexpr float 	OUT_VOLT_RANGE=10.0f;
	static constexpr float 	ZERO_OFFSET=0.5f;
	static constexpr int   	START_OCTAVE=0;
#else 
	static constexpr float 	OUT_VOLT_RANGE=5.0f;
	static constexpr float 	ZERO_OFFSET=0;
	static constexpr int 	START_OCTAVE=1.0f;
#endif 
	static constexpr float semiMult_ = (1.0f / (OUT_VOLT_RANGE * 12.0f)); // 1.0 = 10v = 10 octaves 

	float touchId_=-1.0f;   
    float note_=60.0f;
    float x_=0.0f,y_=0.0f,z_=0.0f;
    bool  active_ = false;

    float breath_=0.0f;
    float ribbon_[2]={0.0f,0.0f};
    

private:
};

void mecProcess(void* pvMec) {
	mec::MecApi *pMecApi = (mec::MecApi*) pvMec;
	pMecApi->process();
}


bool setup(BelaContext *context, void *userData) {
#ifdef SALT
    pinMode(context,0,trigIn1,INPUT);
    pinMode(context,0,trigIn2,INPUT);
    pinMode(context,0,trigIn3,INPUT);
    pinMode(context,0,trigIn4,INPUT);

    pinMode(context,0,trigOut1,OUTPUT);
    pinMode(context,0,trigOut2,OUTPUT);
    pinMode(context,0,trigOut3,OUTPUT);
    pinMode(context,0,trigOut4,OUTPUT);
#endif



	gMecApi=new mec::MecApi();
	gMecCallback=new BelaMecCallback();
	gMecApi->init();
	gMecApi->subscribe(gMecCallback);
	
    // Initialise auxiliary tasks

	if((gMecProcessTask = Bela_createAuxiliaryTask(&mecProcess, BELA_AUDIO_PRIORITY - 1, "mecProcess", gMecApi)) == 0)
		return false;

	return true;
}

// render is called 2750 per second (44000/16)
const int decimation = 5;  // = 550/seconds
long renderFrame = 0;
void render(BelaContext *context, void *userData)
{


#ifdef SALT
    drivePwm(context,pwmOut);

    setLed(context, ledOut1, 2);
    setLed(context, ledOut2, 1);
    setLed(context, ledOut3, 0);
    setLed(context, ledOut4, 1);

    // static unsigned lsw,/*ltr1,*/ ltr2,ltr3,ltr4;
    // static unsigned led_mode=0; // 0 = normal
    // static unsigned led_counter=0;

    // unsigned sw  = digitalRead(context, 0, switch1);  //next layout
    // // unsigned tr1 = digitalRead(context, 0, trigIn1);  
    // unsigned tr2 = digitalRead(context, 0, trigIn2);  //quantize 
    // unsigned tr3 = digitalRead(context, 0, trigIn3);
    // unsigned tr4 = digitalRead(context, 0, trigIn4);


    // if(sw  && !lsw)  { 
    //     // gCallback->nextLayout(); 
    //     led_counter=2100;
    //     led_mode=1;
    // }
    
    // lsw =  sw;
    // // ltr1 = tr1;
    // ltr2 = tr2;
    // ltr3=  tr3;
    // ltr4=  tr4;
#endif


	Bela_scheduleAuxiliaryTask(gMecProcessTask);
	
	renderFrame++;
	// silence audio buffer
	// for(unsigned int n = 0; n < context->audioFrames; n++) {
	// 	for(unsigned int channel = 0; channel < context->audioOutChannels; channel++) {
	// 		audioWrite(context, n, channel, 0.0f);
	// 	}
	// }
	
	gMecCallback->render(context);
	
	
	if(decimation <= 1 || ((renderFrame % decimation) ==0) ) {	
	}

}

void cleanup(BelaContext *context, void *userData)
{
	gMecApi->unsubscribe(gMecCallback);
	delete gMecCallback;
	delete gMecApi;
}
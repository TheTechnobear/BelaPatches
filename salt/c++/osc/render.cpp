#include <Bela.h>


#include <math.h>


#include <OSCServer.h>

#include "defs.h"

static constexpr unsigned BREATH_CTRLID=0;
static constexpr unsigned RIBBON_1_CTRLID=0x10; 
static constexpr unsigned RIBBON_2_CTRLID=0x11;


static constexpr float PRESSURE_CURVE=0.5f;
static constexpr float semiMult_ = (1.0f / (OUT_VOLT_RANGE * 12.0f)); // 1.0 = 10v = 10 octaves 


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




int touchId_=-1;   
float note_=60.0f;
float x_=0.0f,y_=0.0f,z_=0.0f;
bool  active_ = false;
float breath_=0.0f;
float ribbon_[2]={0.0f,0.0f};

OSCServer server_;


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

	server_.setup(3123);

	return true;
}

void processMsg(oscpkt::Message& msg){
	if(msg.partialMatch("/t3d/tch")) {
		char v=msg.addressPattern()[8];
		unsigned tid = v-'0';
		float x, y, z, note;
		msg.arg()
			.popFloat(x)
			.popFloat(y)
			.popFloat(z)
			.popFloat(note);
		
		
		if(active_ && tid==touchId_) {
			active_=z>0.001f;
			x_=x;
			y_=y;
			z_=z;
			note_=note;
			if(!active_) {
				z_=0.0f;
				// rt_printf("end note %d, %f, %f %f %f\n", touchId_, x_,y_,z_, note_);
			}
		} else {
			if(!active_) {
				// new touch
				active_=z>0.0f;
				x_=x;
				y_=y;
				z_=z;
				note_=note;
				touchId_=tid;
				// rt_printf("new note %d, %f, %f %f %f\n", touchId_, x_,y_,z_, note_);
			} else {
				// a new note, but another note is active
			}
		}
	} else if(msg.match("/t3d/control")) {
		int ctrlId;
		float value;
		msg.arg()
			.popInt32(ctrlId)
			.popFloat(value);
		
		switch(ctrlId) {
			case BREATH_CTRLID: {
				breath_=value;
				break;
			}
			case RIBBON_1_CTRLID: {
				ribbon_[0]=value;
				break;
			}
			case RIBBON_2_CTRLID: {
				ribbon_[1]=value;
				break;
			}
			default:
				break;
		}
	}
}
	

void render(BelaContext *context, void *userData)
{
#ifdef SALT
    drivePwm(context,pwmOut);

    setLed(context, ledOut1, 2);
    setLed(context, ledOut2, 1);
    setLed(context, ledOut3, 0);
    setLed(context, ledOut4, 1);
#endif

	if(server_.messageWaiting()) {
		oscpkt::Message msg=server_.popMessage();
		processMsg(msg);
	}
	
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
    float ribbon1   = scaleRibbon(ribbon_[0],  analogRead(context, 0, 4) * 2);
    float ribbon2   = scaleRibbon(ribbon_[1],  analogRead(context, 0, 5) * 2);

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

void cleanup(BelaContext *context, void *userData)
{
}
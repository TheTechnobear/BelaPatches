#include <Bela.h>

#include <string>
#include <algorithm>
#include <eigenapi.h>
#include <math.h>

#include "defs.h"
#include "seq.h"

EigenApi::Eigenharp*	gApi=NULL;
class BelaCallback;
BelaCallback* 	gCallback=NULL;

AuxiliaryTask gProcessTask;
AuxiliaryTask gLEDTask;

Sequencer gSeq;

// only good for pico!
static constexpr unsigned MAX_COLS=2;
static constexpr unsigned MAX_ROWS=18;

static unsigned gLeds[MAX_COLS][MAX_ROWS];
static unsigned gLedsSent[MAX_COLS][MAX_ROWS];

void setLED(unsigned col, unsigned row, unsigned value) {
	if(col<MAX_COLS && row < MAX_ROWS) {
		gLeds[col][row]=value;
	}
}

std::string gDev;


// really simple scale implmentation
static constexpr unsigned MAX_SCALE = 7;
static constexpr float SCALES [MAX_SCALE][13]= {
	{12.0f,0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f,  7.0f, 8.0f, 9.0f, 10.0f, 11.0f},// chromatic
	{7.0f, 0.0f, 2.0f, 4.0f, 5.0f, 7.0f, 9.0f, 11.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},  // major
	{7.0f, 0.0f, 2.0f, 3.0f, 5.0f, 7.0f, 8.0f, 11.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},  // harmonic minor
	{7.0f, 0.0f, 2.0f, 3.0f, 5.0f, 7.0f, 8.0f, 10.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},  // natural minor
	{5.0f, 0.0f, 2.0f, 4.0f, 7.0f, 9.0f, 0.0f, 0.0f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f},  // pentatonic major
	{5.0f, 0.0f, 3.0f, 5.0f, 7.0f, 10.0f,0.0f, 0.0f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f},  // pentatonic minor
	{7.0f, 0.0f, 2.0f, 3.0f, 6.0f, 7.0f, 8.0f, 11.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}   // hungarian minor
};
// scales converted from EigenD /plg_simple/scale_manager_plg.py

class BelaCallback : public EigenApi::Callback {
public:
	BelaCallback()   {
	}

	//========================== callback interface	======================
	void device(const char* dev, DeviceType dt, int rows, int cols, int ribbons, int pedals) override {
		rt_printf("device %s : %d - %d, %d : %d %d",dev, (int) dt, rows,cols,ribbons,pedals);
		gDev = dev; // for leds
		// dev_=dev;
		rows_=rows;
		cols_=cols;
		ribbons_=ribbons;
		pedals=pedals_;
		type_=dt;


		switch (dt) {
			case EigenApi::Callback::PICO:
				break;
			case EigenApi::Callback::TAU:
			case EigenApi::Callback::ALPHA:
				rt_printf("currently this is designed for the pico, some changes may be needed for TAU or ALPHA");
				break;
		}

		// put device into a default state
		mode_ = 0;
    	mainMode_ =0; // 0 == play
		displayScale();
		for(unsigned c = 0;c<MAX_COLS;c++) {
			for(unsigned r = 0;r<MAX_ROWS;r++) {
				gLedsSent[c][r] = 0; 
			}
		}
	}

	void key(const char* dev, unsigned long long t, unsigned course, unsigned key, bool a, unsigned p, int r, int y) override {
		//rt_printf("key %s , %d : %d,  %d  , %d %d %d \n", dev, course, key, a, p , r , y );
		// static unsigned long long lastT = 0;
		// if(t<lastT) {
		// 	rt_printf("old event %d %d\n", lastT,t );
		// 	lastT=t;
		// 	return;
		// }
		 

		if(course) {
			button(key,a);
		} else {
			switch(mode_) {
				case 0: {
					if(mainMode_==0) {
						// playing
						playNote(key,a,p,r,y);
					} else if (mainMode_ ==1) {
						
					}
					break;
				}
				case 1: {
					if(mainMode_==0) {
						if(a && key<MAX_OCTAVE && key!=octave_) {
							setLED(0,octave_,2);
							octave_=key;
							setLED(0,octave_,1);
						} 
					} else if(mainMode_ ==1 ) {
						note_ = scaleNote(key);
					}
					break;
				}
				case 2: {
					if(mainMode_==0) {
						// scale
						if(a && key<MAX_SCALE && key!=scaleIdx_) {
							setLED(0,scaleIdx_,2);
							scaleIdx_=key;
							setLED(0,scaleIdx_,1);
						}
					} else if(mainMode_ ==1 ) {

					}
					break;
				}
				case 3: {
					if(mainMode_==0) {
						// tonic
						if(a && key<12 && key!=tonic_) {
							setLED(0,tonic_,2);
							tonic_=key;
							setLED(0,tonic_,1);
						}
					} else if(mainMode_ ==1 ) {
					}
					break;
				}
				case 4: {
					if(mainMode_==0) {
						if(!a) {
							sequenceNote(key,a,p,r,y);
						}
					} else if(mainMode_ ==1 ) {
					}
					break;
				}
			}//switch
		}// main key
	}
	
	void breath(const char* dev, unsigned long long t, unsigned val) override {
		// rt_printf("breath %s , %d ", dev, val);
		breath_ = unipolar(val);
	}

	void strip(const char* dev, unsigned long long t, unsigned strip, unsigned val) override {
		// rt_printf("strip %s , %d %d ", dev, strip, val);
		ribbon_ = unipolar(val);
	}

	void pedal(const char* dev, unsigned long long t, unsigned pedal, unsigned val) override {
		// rt_printf("pedal %s , %d %d ", dev, pedal, val);
	};

	void dead(const char* dev, unsigned reason) override {
		rt_printf("dead %s , %d ", dev, reason);
	};

	//=====================end of callback interface	======================
	void playNote( unsigned key, bool a, unsigned p, int r, int y) {
		float note = scaleNote(key);
		float mx = bipolar(r);
		float my = bipolar(y);
		float mz = unipolar(p);

		if(active_) {
			// currently have an active key
			if(touchId_==key) {
				// its this key, so update
				active_=a;  // might be off!
				touchId_=key;
				note_=note;
				x_=mx;
				y_=my;
				z_=mz;
			}

		} else {
			// no active key, so take this one
			if(a) {
     			// rt_printf("playNote  %f: %f %f %f\n",note,mx,my,mz);
				active_ = true;
				touchId_ = key;
				note_ = note;
				x_ = mx;
				y_ = my;
				z_ = mz;
			}
		}
	}

	void sequenceNote( unsigned key, bool a, unsigned p, int r, int y) {
		unsigned st = key - (key>7);
		if(st>gSeq.endStep()) return;
		
		Step cs = gSeq.step(st);
		bool active = !cs.active();
		float note = cs.note();
		float sx = 0.0;
		float sy = 0.0;
		float sz = float(active);
		if(active) {
			note = note_;
			// for now, not using x, y
			// and z is constant, 1 or 0
		} else {
			// leave alone
			// note = cs.note();
			;
		}
		Step s(active, note, sx, sy, sz);
		gSeq.step(st ,s);
		displaySequence();
	}
	

	void button(unsigned key, bool a) {
		// rt_printf("button %d %d ", key, a);
		int mode = key + 1;
		if(a) {
			enterMode(mode);
			mode_=mode;
		} else {
			if(mode_==mode) {mode_=0; enterMode(0);} 
		}
	}
	
	void enterMode(unsigned mode) {
		switch(mode) {
			case 0: {
				displayScale();
				break;
			}
			case 1: {
				// select octave
				for(unsigned i=0;i<(rows_*cols_);i++) {
					setLED(0, i,(i<MAX_OCTAVE ? 2  : 0)  ); 
				}
				setLED(0,octave_,1);
				break;
			}
			case 2: {
				// select scale
				for(unsigned i=0;i<(rows_*cols_);i++) {
					setLED(0, i,(i<MAX_SCALE ? 2 : 0) ); 
				}
				setLED(0,scaleIdx_,1);
				break;
			}
			case 3: {
				// select tonic
				for(unsigned i=0;i<(rows_*cols_);i++) {
					setLED(0, i,(i<12 ? 2 : 0)); 
				}
				setLED(0,tonic_,1);
				break;
			}
			case 4: {
				displaySequence();
				break;
			}
		}
	}
	
	void displayScale() {
		unsigned scalelen=SCALES[scaleIdx_][0];
		for(unsigned i=0;i<(rows_*cols_);i++) {
			// turn off all lights
			setLED(0, i,(i%scalelen) == 0); 
		}
	}

	void displaySequence() {
		// green for unused keys 
		setLED(0, 8, 1);
		setLED(0, 17, 1);
		for(unsigned i=0;i<16;i++) {
			Step s = gSeq.step(i);
			bool a = i < gSeq.endStep() && s.active();
			setLED(0, i + (i>7), a);
		}
		unsigned cur = gSeq.curStep();
		setLED(0, cur + (cur>7), 2);
	}
	
	float scaleNote(unsigned key) {
		unsigned scalelen=SCALES[scaleIdx_][0];
		int octave = key / scalelen;
		int noteidx = (key % scalelen) + 1;
		float note = SCALES[scaleIdx_][ noteidx];
		return note + float(octave) * 12.0f;  
	}

    void render(BelaContext *context) {
    	float a[8];

		unsigned lStep = gSeq.curStep();
		for(unsigned int n = 0; n < context->analogFrames; n++) {
			gSeq.tick();
		}
		unsigned nStep = gSeq.curStep();
		Step s = gSeq.step(nStep);
		a[0] = transpose(note_ + pitchbend(x_),octave_,tonic_);
		a[1] = scaleY(y_,1.0f);
		a[2] = pressure(z_,1.0f);
		a[3] = ribbon_;
		a[4] = breath_;
		a[5] = transpose(s.note() + pitchbend(s.x()),octave_,tonic_);
		a[6] = scaleY(s.y(),1.0f);
		a[7] = pressure(s.z(),1.0f);
		
		if(mode_==4) {
	 		if(lStep!=nStep) {
				Step lS = gSeq.step(lStep);
				setLED( 0, lStep + (lStep>7), lS.active());
				setLED( 0 , nStep + (nStep>7), 2);
			}
		}

		for(unsigned int n = 0; n < context->analogFrames; n++) {
			for(unsigned i = 0;i<8;i++) {
				analogWriteOnce(context, n, i,a[i]);
			}
		}    	
    }
    
    
private: 

	static constexpr float PRESSURE_CURVE=0.5f;

	float audioAmp(float z, float mult ) {
		return powf(z, PRESSURE_CURVE) * mult;
	}

	float pressure(float z, float mult ) {
		// return z;
		return ( (powf(z, PRESSURE_CURVE) * mult * ( 1.0f-ZERO_OFFSET) ) ) + ZERO_OFFSET;	
	}

	static constexpr float PB_CURVE=2.0f;
	static constexpr float PB_RANGE=2.0f;
	float pitchbend(float x) {
		float sign = x < 0 ? -1.0f : 1.0f;
		float pbx = powf(x * sign, PB_CURVE);
		return pbx*sign*PB_RANGE;
		
	}
	
	float scaleY(float y, float mult) {
		return ( (y * mult * 0.5f)  * ( 1.0f-ZERO_OFFSET) )  + 0.5f ;	
	}

	float scaleX(float x, float mult) {
		return ( (x * mult * 0.5f)  * ( 1.0f-ZERO_OFFSET) )  + 0.5f ;	
	}
	
	float transpose (float pitch, int octave, int semi) {
		return (pitch + semi + (( START_OCTAVE + octave) * 12.0f )) *  semiMult_ ;
	}



#ifdef SALT
	static constexpr float 	OUT_VOLT_RANGE=10.0f;
	static constexpr float 	ZERO_OFFSET=0.5f;
	static constexpr int   	START_OCTAVE=5;
#else 
	static constexpr float 	OUT_VOLT_RANGE=5.0f;
	static constexpr float 	ZERO_OFFSET=0.0f;
	static constexpr int 	START_OCTAVE=0.0f;
#endif 

	static constexpr unsigned MAX_OCTAVE = OUT_VOLT_RANGE;
	static constexpr float semiMult_ = (1.0f / (OUT_VOLT_RANGE * 12.0f)); // 1.0 = 10v = 10 octaves 

	// std::string dev_;
	float rows_;
	float cols_;
	float ribbons_;
	float pedals_;
	DeviceType type_;


	float touchId_=0.0f;   
    float note_=0.0f;
    float x_=0.0f,y_=0.0f,z_=0.0f;
    bool active_ = false;

    float breath_=0.0f;
    float ribbon_=0.0f;
    
    unsigned octave_ = 1;
    unsigned tonic_ = 0; //C 
    unsigned scaleIdx_ = 0;
    unsigned mode_ = 0;
    unsigned mainMode_ =0; // 0 == play

	inline float clamp(float v, float mn, float mx) { return (std::max(std::min(v, mx), mn)); }

	float unipolar(int val) { return std::min(float(val) / 4096.0f, 1.0f); }

	float bipolar(int val) { return clamp(float(val) / 4096.0f, -1.0f, 1.0f); }

};

void eliteProcess(void* pvApi) {
	EigenApi::Eigenharp *pApi = (EigenApi::Eigenharp*) pvApi;
	pApi->process();
}


void ledProcess(void* pvApi) {
	EigenApi::Eigenharp *pApi = (EigenApi::Eigenharp*) pvApi;
	for(unsigned c = 0;c<MAX_COLS;c++) {
		for(unsigned r = 0;r<MAX_ROWS;r++)
		if(gLeds[c][r]!=gLedsSent[c][r]) {
			pApi->setLED(gDev.c_str(), c , r, gLeds[c][r]);
			gLedsSent[c][r] = gLeds[c][r];
		}
	}
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


	for(unsigned c = 0;c<MAX_COLS;c++) {
		for(unsigned r = 0;r<MAX_ROWS;r++) {
			gLedsSent[c][r] = 0; 
			gLeds[c][r] = 0;
		}
	}



	gApi= new EigenApi::Eigenharp("./");
	gApi->setPollTime(100);
	gCallback=new BelaCallback();
	gApi->addCallback(gCallback);

	if(!gApi->start()) return false;
	
    // Initialise auxiliary tasks

	if((gProcessTask = Bela_createAuxiliaryTask(&eliteProcess, BELA_AUDIO_PRIORITY - 1, "eliteProcess", gApi)) == 0)
		return false;

	if((gLEDTask = Bela_createAuxiliaryTask(&ledProcess, BELA_AUDIO_PRIORITY - 1, "ledProcess", gApi)) == 0)
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

	static unsigned lsw,/*ltr1,*/ ltr2,ltr3,ltr4;
	static unsigned led_mode=0; // 0 = normal
	static unsigned led_counter=0;

	unsigned sw  = digitalRead(context, 0, switch1);  //next layout
	// unsigned tr1 = digitalRead(context, 0, trigIn1);  
	unsigned tr2 = digitalRead(context, 0, trigIn2);  //quantize 
	unsigned tr3 = digitalRead(context, 0, trigIn3);
	unsigned tr4 = digitalRead(context, 0, trigIn4);

	setLed(context, ledOut1, 1);
	setLed(context, ledOut2, 2);
	setLed(context, ledOut3, 0);
	setLed(context, ledOut4, 1);



	if(sw  && !lsw)  { 
		// gCallback->nextLayout(); 
		led_counter=2100;
		led_mode=1;
	}
	
	// if(led_mode==1) {
	// 	led_counter--;
	// 	if(led_counter>100 
	// 		&& led_counter < 2000
	// 		&& ((led_counter / 500) % 2)
	// 	) {
	//		unsigned layout = (gCallback->layoutSignature());
	// 		setLed(context, ledOut4, layout & 0x3);
	// 		layout = layout >> 2;
	// 		setLed(context, ledOut3, layout & 0x3);
	// 		layout = layout >> 2;
	// 		setLed(context, ledOut2, layout & 0x3);
	// 		layout = layout >> 2;
	// 		setLed(context, ledOut1, layout & 0x3);
	// 	} else {
	// 		setLed(context, ledOut1, 0);
	// 		setLed(context, ledOut2, 0);
	// 		setLed(context, ledOut3, 0);
	// 		setLed(context, ledOut4, 0);
	// 		if(led_counter==0) led_mode=0;
	// 	}
	// }
	// if(tr2  && !ltr2)  { gCallback->nextCustomMode(); }
	// if(tr3  && !ltr3)  { gCallback->nextPitchMode(); }
	// if(tr4  && !ltr4)  { gCallback->nextQuantMode(); }
	// if(led_mode==0) {
	// 	setLed(context, ledOut2, gCallback->customMode() %3);
	// 	setLed(context, ledOut3, gCallback->pitchMode() %3);
	// 	setLed(context, ledOut4, gCallback->quantMode() %3);
	// }

	lsw =  sw;
	// ltr1 = tr1;
	ltr2 = tr2;
	ltr3=  tr3;
	ltr4=  tr4;
#endif

	Bela_scheduleAuxiliaryTask(gProcessTask);
	Bela_scheduleAuxiliaryTask(gLEDTask);
	
	renderFrame++;
	// silence audio buffer
	for(unsigned int n = 0; n < context->audioFrames; n++) {
		for(unsigned int channel = 0; channel < context->audioOutChannels; channel++) {
			audioWrite(context, n, channel, 0.0f);
		}
	}
	
	gCallback->render(context);
	
	
	if(decimation <= 1 || ((renderFrame % decimation) == 0) ) {	
	}

}

void cleanup(BelaContext *context, void *userData)
{
	gApi->clearCallbacks();
	delete gCallback;
	
	gApi->stop();
	delete gApi;
}

#include <Bela.h>

#include <memory>
#include <cmath>
#include <iostream>

#include <libraries/Scope/Scope.h>
#include <libraries/Gui/Gui.h>

Scope scope;
Gui gui;

enum dataIn {
	DI_BOARD,
	DI_I_TARGET_CH,
	DI_I_TARGET_VOLT,
	DI_I_CAL_TRIG,
	DI_O_TARGET_CH,
	DI_O_TARGET_VOLT,
	DI_O_MIN_VOLT,
	DI_O_MAX_VOLT,
	DI_O_T_FLOAT,
	DI_O_CAL_TRIG,
	DI_MAX
};


enum dataOut {
	DO_I_MIN_VOLT,
	DO_I_MAX_VOLT,
	DO_I_ACT_VOLT,
	DO_I_ACT_FLOAT,
	DO_I_CAL_FLOAT,
	DO_I_CAL_VOLT,
	DO_O_ACT_FLOAT,
	DO_O_CAL_FLOAT,
	DO_O_MIN_VOLT,
	DO_O_MAX_VOLT,
	DO_MAX
};

enum boardType {
	BT_SALT,
	BT_PEPPER,
	BT_BELA,
	BT_MAX
};


float dataOutBuf[DO_MAX];
float dataInBuf[DI_MAX];

static constexpr unsigned MAX_STATUS=120;
static char statusBuf[MAX_STATUS];


static constexpr unsigned MAX_RES=10+1;
static constexpr unsigned MAX_CH=8;
float inCalValues[MAX_CH][MAX_RES];
float outCalValues[MAX_CH][MAX_RES];


bool setup(BelaContext *context, void *userData) {
	scope.setup(2, context->audioSampleRate);
	gui.setup(context->projectName);

	for(unsigned i=0;i<DO_MAX;i++) {
		dataOutBuf[i]=0.0f;
	}
	for(unsigned i=0;i<DI_MAX;i++) {
		dataInBuf[i]=0.0f;
	}
	gui.setBuffer('f', DO_MAX); 
	gui.setBuffer('c', MAX_STATUS);

	dataInBuf[DI_BOARD]=BT_MAX;

	for(unsigned ch=0;ch<MAX_CH;ch++) {
		for(int i=0;i<MAX_RES;i++) {
			inCalValues[ch][i] = float(i) / 10.0f;
			outCalValues[ch][i] = float(i) / 10.0f;
		}
	}

	return true;
}

void sendStatus() {
	gui.sendBuffer(1,statusBuf);
}

unsigned findCalStep(float v, float min,float max) {
	float rangeV = fabs(min) + max;
	float absV = (fabs(min) + v) *  (float(10.0f)/ rangeV );
	unsigned step = absV + 0.2;
	return step;
}

void inCalibrated() {
	unsigned tCh= dataInBuf[DI_I_TARGET_CH];
	float tV=dataInBuf[DI_I_TARGET_VOLT];
	float minV=dataOutBuf[DO_I_MIN_VOLT];
	float maxV=dataOutBuf[DO_I_MAX_VOLT];

	unsigned step = findCalStep(tV,minV,maxV);
	step = step <MAX_RES ? step : MAX_RES-1;
	tCh = tCh <MAX_CH ? tCh : MAX_CH-1;

	inCalValues[tCh][step] = dataOutBuf[DO_I_ACT_FLOAT];
}

void outCalibrated() {
	unsigned tCh= dataInBuf[DI_O_TARGET_CH];
	float tV=dataInBuf[DI_O_TARGET_VOLT];
	float minV=dataOutBuf[DO_O_MIN_VOLT];
	float maxV=dataOutBuf[DO_O_MAX_VOLT];
	unsigned step = findCalStep(tV,minV,maxV);
	step = step <MAX_RES ? step : MAX_RES-1;
	tCh = tCh <MAX_CH ? tCh : MAX_CH-1;
	outCalValues[tCh][step] = dataInBuf[DI_O_T_FLOAT];
	// rt_printf("out calibrated %d %f\n", step, outCalValues[tCh][step]);
	sprintf(statusBuf,"out calibrated s %d tV %f mV %f xV %f outV %f", step, tV, minV, maxV,outCalValues[tCh][step]);
	sendStatus();
}


void calTargetVolt() {
	unsigned tCh= dataInBuf[DI_O_TARGET_CH];
	float tV=dataInBuf[DI_O_TARGET_VOLT];
	float minV=dataOutBuf[DO_O_MIN_VOLT];
	float maxV=dataOutBuf[DO_O_MAX_VOLT];
	unsigned step = findCalStep(tV,minV,maxV);
	float rangeV = fabs(minV) + maxV;
	float absV = (fabs(minV) + tV);

	dataOutBuf[DO_O_ACT_FLOAT] = absV / rangeV;
	step = step <MAX_RES ? step : MAX_RES-1;
	tCh = tCh <MAX_CH ? tCh : MAX_CH-1;
	dataOutBuf[DO_O_CAL_FLOAT] = outCalValues[tCh][step];
	dataInBuf[DI_O_T_FLOAT]= outCalValues[tCh][step];
}

void dataChanged(unsigned i, float oldV, float newV) {
	switch(i) {
		case DI_BOARD : {
			switch ((unsigned) newV) {
				case BT_SALT : {
					dataOutBuf[DO_I_MIN_VOLT]=-5.0;
					dataOutBuf[DO_I_MAX_VOLT]=5.0;
					dataOutBuf[DO_O_MIN_VOLT]=-5.0;
					dataOutBuf[DO_O_MAX_VOLT]=5.0;
					calTargetVolt();

					break;
				} 
				case BT_PEPPER : {
					dataOutBuf[DO_I_MIN_VOLT]=0;
					dataOutBuf[DO_I_MAX_VOLT]=10.0;
					dataOutBuf[DO_O_MIN_VOLT]=0;
					dataOutBuf[DO_O_MAX_VOLT]=5.0;
					calTargetVolt();
					break;
				} 
				case BT_BELA: {
					dataOutBuf[DO_I_MIN_VOLT]=0;
					dataOutBuf[DO_I_MAX_VOLT]=5.0;
					dataOutBuf[DO_O_MIN_VOLT]=0;
					dataOutBuf[DO_O_MAX_VOLT]=5.0;
					calTargetVolt();
					break;
				} 
				default: {
					dataOutBuf[DO_I_MIN_VOLT]=0;
					dataOutBuf[DO_I_MAX_VOLT]=5.0;
					dataOutBuf[DO_O_MIN_VOLT]=0;
					dataOutBuf[DO_O_MAX_VOLT]=5.0;
					calTargetVolt();
					break;
				} 
			} // board type
			break;
		} // case DI BOARD
		case DI_I_CAL_TRIG : {
			if(newV>0.5) inCalibrated();
			break;
		}
		case DI_O_CAL_TRIG : {
			if(newV>0.5) outCalibrated();
			break;
		}
		case DI_I_TARGET_VOLT : {
			break;
		}
		case DI_O_TARGET_VOLT : {
			calTargetVolt();
			sprintf(statusBuf,"new TargetVoltage %f %f %f %f ", newV, dataOutBuf[DO_O_ACT_FLOAT], dataInBuf[DI_O_T_FLOAT],dataOutBuf[DO_O_CAL_FLOAT]);
			sendStatus();
			break;
		}
		case DI_O_T_FLOAT : {
			dataOutBuf[DO_O_CAL_FLOAT] = dataInBuf[DI_O_T_FLOAT];
			break;
		}
	}
}



void render(BelaContext *context, void *userData) {
	if(!gui.isConnected()) return;

	static unsigned sendCount =0;
	sendCount++;
	if(! (sendCount % 1000)) {	
		// Retrieve contents of the buffer as floats
		DataBuffer buffer = gui.getDataBuffer(0);
		unsigned sz =buffer.getNumElements();
		float* data = buffer.getAsFloat();
		if(data) {
			// rt_printf("data  buffer sz=%d  [ ",sz);
			// for(unsigned i=0;i<sz;i++) 	rt_printf("%f ,  ", data[i]);
			// rt_printf("]\n",sz);

			for(unsigned i=0;i<DI_MAX && i<sz;i++) {
				float old = dataInBuf[i];

				dataInBuf[i]=data[i];
				if(old != dataInBuf[i]) {
					dataChanged(i,old, dataInBuf[i]);
				}
			}
		}

		if(unsigned(dataInBuf[DI_I_TARGET_CH]) < context->analogInChannels) {
			unsigned ch = dataInBuf[DI_I_TARGET_CH];
			float v = analogRead(context, 0,ch);

			// act = current reading.
			// cal = calibrated reading
			dataOutBuf[DO_I_ACT_FLOAT]= v;
			float minV= dataOutBuf[DO_I_MIN_VOLT];
			float maxV= dataOutBuf[DO_I_MAX_VOLT];
			float vMult = fabs(minV) + maxV;
			dataOutBuf[DO_I_ACT_VOLT]=(dataOutBuf[DO_I_ACT_FLOAT] * vMult) + minV;

			unsigned actStep = v * (MAX_RES - 1);
			float pctStep= (v * (MAX_RES - 1)) - actStep;
			float calLow = actStep == 0 ? inCalValues[ch][0] : inCalValues[ch][actStep];
			float calHigh= actStep == (MAX_RES-1) ? inCalValues[ch][MAX_RES-1] : inCalValues[ch][actStep+1];
			float calF   = calLow + ((calHigh-calLow) * pctStep);

			dataOutBuf[DO_I_CAL_FLOAT]= calF;

			//convert to voltage
			dataOutBuf[DO_I_CAL_VOLT]=(dataOutBuf[DO_I_CAL_FLOAT] * vMult) + minV;
		}

		gui.sendBuffer(0,dataOutBuf);
	}
	
	if(unsigned(dataInBuf[DI_O_TARGET_CH]) < context->analogOutChannels) {
		for(unsigned int n = 0; n < context->analogFrames; n++) {
			analogWriteOnce(context, n, unsigned(dataInBuf[DI_O_TARGET_CH]) , dataInBuf[DI_O_T_FLOAT]);
		}
	}
	
}

void cleanup(BelaContext *context, void *userData) {
	;
}
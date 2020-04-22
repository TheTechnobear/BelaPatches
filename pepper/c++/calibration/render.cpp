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
	DO_I_THEORY_VOLT,
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



static constexpr unsigned MAX_DIG=16;
// std::shared_ptr<Calibrator> calibrator=nullptr;
bool digInState[MAX_DIG];
bool digInTrans[MAX_DIG];




bool setup(BelaContext *context, void *userData)
{
	scope.setup(2, context->audioSampleRate);
	gui.setup(context->projectName);

	for(unsigned i=0;i<DO_MAX;i++) {
		dataOutBuf[i]=0.0f;
	}
	for(unsigned i=0;i<DI_MAX;i++) {
		dataInBuf[i]=0.0f;
	}
	gui.setBuffer('f', DO_MAX); 

	dataInBuf[DI_BOARD]=BT_MAX;

	return true;
}

void inCalibrated() {
	dataOutBuf[DO_I_MAX_VOLT]+=1.0;
}

void outCalibrated() {
	dataOutBuf[DO_O_MAX_VOLT]+=1.0;
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
					break;
				} 
				case BT_PEPPER : {
					dataOutBuf[DO_I_MIN_VOLT]=0;
					dataOutBuf[DO_I_MAX_VOLT]=10.0;
					dataOutBuf[DO_O_MIN_VOLT]=0;
					dataOutBuf[DO_O_MAX_VOLT]=5.0;
					break;
				} 
				case BT_BELA: {
					dataOutBuf[DO_I_MIN_VOLT]=0;
					dataOutBuf[DO_I_MAX_VOLT]=5.0;
					dataOutBuf[DO_O_MIN_VOLT]=0;
					dataOutBuf[DO_O_MAX_VOLT]=5.0;
					break;
				} 
				default: {
					dataOutBuf[DO_I_MIN_VOLT]=0;
					dataOutBuf[DO_I_MAX_VOLT]=5.0;
					dataOutBuf[DO_O_MIN_VOLT]=0;
					dataOutBuf[DO_O_MAX_VOLT]=5.0;
					break;
				} 
			} // board type
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
			break;
		}
		case DI_O_T_FLOAT : {
			break;
		}
	}
}

void render(BelaContext *context, void *userData)
{
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
				if(data[i] != dataInBuf[i]) {
					dataChanged(i,dataInBuf[i],data[i]);
				}
				dataInBuf[i]=data[i];
			}
		}

		if(unsigned(dataInBuf[DI_I_TARGET_CH]) < context->analogInChannels) {
			float v = analogRead(context, 0,unsigned(dataInBuf[DI_I_TARGET_CH]));
			dataOutBuf[DO_I_ACT_FLOAT]=v;
			dataOutBuf[DO_I_CAL_FLOAT]=v;
			dataOutBuf[DO_I_THEORY_VOLT]=v * dataOutBuf[DO_I_MAX_VOLT];
			dataOutBuf[DO_I_CAL_VOLT]=v * dataOutBuf[DO_I_MAX_VOLT];
		}

		dataOutBuf[DO_O_ACT_FLOAT]=dataInBuf[DI_O_T_FLOAT];
		dataOutBuf[DO_O_CAL_FLOAT]=dataInBuf[DI_O_T_FLOAT];

		gui.sendBuffer(0,dataOutBuf);
	}
	
	if(unsigned(dataInBuf[DI_O_TARGET_CH]) < context->analogOutChannels) {
		for(unsigned int n = 0; n < context->analogFrames; n++) {
			analogWriteOnce(context, n, unsigned(dataInBuf[DI_O_TARGET_CH]) , dataOutBuf[DI_O_T_FLOAT]);
		}
	}
	
}

void cleanup(BelaContext *context, void *userData)
{
}
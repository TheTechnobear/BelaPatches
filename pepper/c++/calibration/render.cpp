#include <Bela.h>

#include <memory>
#include <cmath>
#include <iostream>

#include <libraries/Scope/Scope.h>
#include <libraries/Gui/Gui.h>

Scope scope;
Gui gui;

float guiBuf[] = {0.0f,0,0f,0.0f,0.0f,0,0f};


class Calibrator {
public: 
	Calibrator() {
		offsets_ = new float[maxOutputs_ * voltRange_];
		for(unsigned o = 0;o<maxOutputs_;o++) {
			out_[o] = 0.0f;
			for(unsigned i = 0;i<voltRange_;i++) {
				offset(o,i) = 0.0f;
			}
		}
	}
	~Calibrator() {
		delete [] offsets_;
	}
	
	float& offset(unsigned o, unsigned v) {
		return offsets_[voltRange_*o + v];
	}
	
	float out(unsigned o) { return out_[o] + calibOffset_;}
	void  out(unsigned o, float v) { out_[o]=v;}
	
	unsigned calibOutput() { return calibOutput_; }
	float nextCalibrationStep() {
		offset(calibOutput_,calibN_) = calibOffset_;
		if(calibN_< voltRange_ ) {
			calibN_ ++;
		} else {
			for(unsigned i=0;i<voltRange_;i++) {
				rt_printf("%d:%d %f \n", calibOutput_, i, offset(calibOutput_,i));
			}
			calibN_=0;
			calibOutput_++;
			if(calibOutput_>7) calibOutput_ = 0;
		}
		return float(calibN_) * calibStep_;
	}
	float prevCalibrationStep() {
		rt_printf("%d:%d %f \n", calibOutput_, calibN_, calibOffset_);
		offset(calibOutput_,calibN_) = calibOffset_;
		if(calibN_>0) {
			calibN_--;
		}
		else {
			calibN_=voltRange_;
			if(calibOutput_==0) {
				calibOutput_ = 7;
			} else {
				calibOutput_--;
			}
		}
		return float(calibN_) * calibStep_;
	}
	
	void applyOffset(float off) {
		calibOffset_ = off / 100.0f;
	}

private:
    static constexpr unsigned maxOutputs_ = 8;
    static constexpr int minVolt_ = 0;
    static constexpr int maxVolt_ = 5;
    unsigned voltRange_ = maxVolt_ + std::abs(minVolt_);
	float calibStep_ = 1.0f / (float) voltRange_;  
    
	unsigned calibN_ = 0;
	float calibOffset_ = 0.0f;
	unsigned calibOutput_ = 0;
	
    float out_[maxOutputs_];
	float *offsets_;
};


static constexpr unsigned MAX_DIG=16;
std::shared_ptr<Calibrator> calibrator=nullptr;
bool digInState[MAX_DIG];
bool digInTrans[MAX_DIG];


bool setup(BelaContext *context, void *userData)
{
	scope.setup(2, context->audioSampleRate);
	gui.setup(context->projectName);
	gui.setBuffer('f', 1); // Index = 0
	gui.setBuffer('f', 1); // Index = 1
	gui.setBuffer('f', 1); // Index = 1
	gui.setBuffer('f', 1); // Index = 1
	gui.setBuffer('f', 1); // Index = 1

	calibrator=std::make_shared<Calibrator>();
	for(unsigned n=0;n<MAX_DIG;n++) {
		digInState[n] = digInState[n] = false;
	}
	return true;
}


void render(BelaContext *context, void *userData)
{
	static unsigned sendCount =0;
	sendCount++;
	
	static constexpr unsigned but1 = 15;
	// static constexpr unsigned but2 = 14;
	
	for(unsigned int n=0; n<context->digitalFrames; n++) {
		for(unsigned int c=0; c<MAX_DIG; c++){
			int status=digitalRead(context, 0, c);
			if(n==0) {
				digInTrans[c] = !status && digInState[c];
				digInState[c] = status;
			}
		}
	}
	
	if(digInTrans[but1]) {
		float v = calibrator->nextCalibrationStep();
		calibrator->out(calibrator->calibOutput(),v);
	}

	float pot1 = analogRead(context, 0,0);
	float offset = ( (pot1 - 0.5) * 2.0f ); // -1 to 1
	calibrator->applyOffset(offset);

	for(unsigned int n = 0; n < context->analogFrames; n++) {
		for(unsigned int channel = 0; channel < context->analogOutChannels; channel++) {
			float out = calibrator->out(channel);
			analogWriteOnce(context, n, channel, out);
		}
	}

	if(! (sendCount % 1000)) {	
		guiBuf[0] = offset;
		gui.sendBuffer(0,guiBuf);
		
		DataBuffer buffer = gui.getDataBuffer(1);
		// Retrieve contents of the buffer as floats
		float* data = buffer.getAsFloat();
		
		static unsigned idx = 0;
		if(idx!=data[0]) {
			idx = data[0];
			rt_printf("index change %d \n", idx);
		}
	}
	
	
}
void cleanup(BelaContext *context, void *userData)
{
	calibrator=nullptr;
}
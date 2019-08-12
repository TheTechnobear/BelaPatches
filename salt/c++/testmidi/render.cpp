#include <Bela.h>
#include <Midi.h>
#include <stdlib.h>
#include <cmath>

Midi midi;

const char* gMidiPort0 = "hw:1,0,0";

bool setup(BelaContext *context, void *userData)
{
	midi.readFrom(gMidiPort0);
	midi.writeTo(gMidiPort0);
	midi.enableParser(true);
	return true;
}



float gVOct = 0.0;
bool gGate = false;
int gCVPin = 0;
int gGatePin = 0;

// float gVOctScale =  0.911484f;
float gVOctScale =  1.0f;

enum {kVelocity, kNoteOn, kNoteNumber};
void render(BelaContext *context, void *userData)
{
	int num = 0;
	while((num = midi.getParser()->numAvailableMessages()) > 0){
		static MidiChannelMessage message;
		message = midi.getParser()->getNextChannelMessage();
		if(message.getType() == kmmNoteOn){
			int note= message.getDataByte(0);
			int velocity = message.getDataByte(1);
			gVOct = float(note) / 120.0f ; // 10 octaves = 10v
			gGate = velocity > 0;
			if(gGate) {
				rt_printf("note on %d, %f\n",  note, gVOct);
			} else {
				rt_printf("note off %d, %f\n",  note, gVOct);
			}
		} else if(message.getType() == kmmNoteOff){
			int note= message.getDataByte(0);
			// int velocity = message.getDataByte(1);
			gVOct = float(note) / 120.0f ; // 10 octaves = 10v
			gGate = false;
			rt_printf("note off %d, %f\n",  note, gVOct);
		}
	}
	for(unsigned int n = 0; n < context->analogFrames; ++n) {
		analogWriteOnce(context, n, gCVPin, gVOct *gVOctScale);
	}
	for(unsigned int n = 0; n < context->digitalFrames; ++n) {
		pinModeOnce(context, n, gGatePin, OUTPUT);
		digitalWriteOnce(context, n, gGatePin, gGate); 
	}
}

void cleanup(BelaContext *context, void *userData)
{

}


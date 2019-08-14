/***** defs.h *****/
#pragma once

#define MAX_TOUCH 4


#ifdef SALT
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
#else
enum
{
	switch1 = 1,
	trigIn1 = 2,
	trigIn2 = 3,
	trigIn3 = 4,
	trigIn4 = 5,
	
	trigOut1 = 1,
	trigOut2 = 2,
	trigOut3 = 3,
	trigOut4 = 4,
	ledOut1 = 5,
	ledOut2 = 6,
	ledOut3 = 7,
	ledOut4 = 8
	// pwmOut = 7,
};
#endif

#ifdef SALT
	static constexpr float 	OUT_VOLT_RANGE=10.0f;
	static constexpr float 	ZERO_OFFSET=0.5f;
	static constexpr int   	START_OCTAVE=0;
#else 
	static constexpr float 	OUT_VOLT_RANGE=5.0f;
	static constexpr float 	ZERO_OFFSET=0;
	static constexpr int 	START_OCTAVE=1.0f;
#endif 





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



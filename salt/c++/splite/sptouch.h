/***** sptouch.h *****/
#pragma once

struct SPTouch {
	SPTouch() : 
		SPTouch(0,false,
				0.0f,0.0f,0.0f) {
		;
	}
	
	SPTouch(unsigned  tId, bool active,float x, float y, float z) 
	: 	tId_(tId), active_(active),
		x_(x), y_(y), z_(z) , 
		pitch_(0.0f), 
		scalePitch_(0.0f), vibPitch_(0.0f), 
		zone_(0) {
		;		
	}
	
	SPTouch(const SPTouch& t) 
	: 	tId_(t.tId_), active_(t.active_),
		x_(t.x_), y_(t.y_), z_(t.z_) , 
		pitch_(t.pitch_),  
		scalePitch_(t.scalePitch_), vibPitch_(t.vibPitch_),
		zone_(t.zone_) {
		;		
	}

	SPTouch& operator=(const SPTouch& t) {
		tId_ = t.tId_;
		active_=t.active_;
		x_=t.x_;
		y_=t.y_;
		z_=t.z_;
		pitch_=t.pitch_;
		scalePitch_=t.scalePitch_;
		vibPitch_=t.vibPitch_;
		zone_= t.zone_;
		return *this;
	}
	
	unsigned tId_;
	bool active_;
	float x_;
	float y_;
	float z_;
	
	float pitch_;
	float scalePitch_;
	float vibPitch_;
	unsigned  zone_;
	// float vib_;
};

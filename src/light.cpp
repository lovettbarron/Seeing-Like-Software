//
//  light.cpp
//  seeing-like-software
//
//  Created by Andrew Lovett Barron on 2013-06-02.
//
//

#include "light.h"


Light::Light(ofVec3f _position, int _id) {
    loc = _position;
    lightId = _id;
    active = false;
    lightDebug = false;
}

Light::~Light() {
    
}

void Light::draw() {
    ofPushMatrix();
    ofTranslate(loc);
    if(active)
        ofSetColor(255);
    else
        ofSetColor(0);
    ofDrawBitmapString("Light" + ofToString(lightId),0,12);
    ofDrawBitmapString("x" + ofToString(loc.x) + " y" + ofToString(loc.z),0,0);
    ofScale(10,10,10);
    ofSphere(1);
    ofPopMatrix();
}

void Light::update() {
    
}

// Previous installations have included a debug mode for more
// complex lighting, basically a display on the side that communicates
// the light state. See http://github.com/readywater/firesite for examples
void Light::debug() {
    
}

void Light::setActive(bool _active) {
    active = _active;
}

void Light::setLocation(ofVec3f _position) {
    loc = _position;
}

ofVec3f Light::getLocation() {
    return loc;
}

// Simply turns on and off, but can be set up to return
// a greater range, hence returning an integer.
int Light::getStrength() {
    if(active) return 1;
    else return 0;
}
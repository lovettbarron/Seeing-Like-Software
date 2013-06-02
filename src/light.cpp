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
    ofSphere(1);
    ofPopMatrix();
}

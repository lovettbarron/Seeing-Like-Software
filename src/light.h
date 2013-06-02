//
//  light.h
//  seeing-like-software
//
//  Created by Andrew Lovett Barron on 2013-06-02.
//
//

#ifndef __seeing_like_software__light__
#define __seeing_like_software__light__

#include "ofMain.h"


class Light {
    
public:
    
    Light(ofVec3f _position, int _id);
    ~Light();
    void update();
    void draw();
    void setLocation(ofVec3f _position);
    ofVec3f getLocation();
    
    void setTotalDist(float _dist);
    float getTotalDist();
    void isActive(bool _active);
    void isDebug(bool _debug);
private:
    int lightId;
    bool active, lightDebug;
    ofVec3f loc;
};

#endif /* defined(__seeing_like_software__light__) */

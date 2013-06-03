//
//  camera.h
//  seeing-like-software
//
//  Created by Andrew Lovett Barron on 2013-06-02.
//
//

#ifndef __seeing_like_software__camera__
#define __seeing_like_software__camera__

#include "ofMain.h"
#include "light.h"
#include "ofxCv.h"
#include "ofxKinect.h"
#include "ofxAutoControlPanel.h"

class Camera {
public:
    Camera(ofxAutoControlPanel * _panel, vector<Light*> _lights);
    ~Camera();

    void setup();
    void update();
    void draw();
    void setBackground();
    
    vector<ofPoint> getPeople();
    void drawPerson(ofPoint pos);
    void drawPeople();
    
    ofxAutoControlPanel * panel;
    vector<Light*> lights;
    
    // cv
    ofxKinect kinect;
    ofVideoGrabber cam;
    void fillHoles(cv::Mat src, cv::Mat dst);
    void fillHoles(cv::Mat _mat);

    ofImage thresh;
    ofImage bgThresh;
    ofImage kDepth;
    cv::Mat kDepthMat;
    cv::Mat threshMat;
    cv::Mat avgMat;
    int avgCounter;
    ofxCv::RunningBackground background;
    ofxCv::ContourFinder contourFinder;
};

#endif /* defined(__seeing_like_software__camera__) */

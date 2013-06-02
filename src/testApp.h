#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "ofxKinect.h"
#include "ofxAutoControlPanel.h"

class testApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
		void keyPressed  (int key);
		
        void setupCamera();
        void updateCamera();
        void drawCamera();

        vector <Light*> lights;

        // Globals
        bool debug, rotate, sliderControl;
        float depthMulti;
        int numberOfLights;
        ofVec3f room;

        // Arduino
        ofSerial serial;
        string buffer;
        long limitBuffer;


        // control
        ofxAutoControlPanel panel;
        int panelWidth; // Debugging away from img
        float threshold;

        // cv

        ofEasyCam camera;
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

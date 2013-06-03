#pragma once

#include "ofMain.h"
#include "ofxAutoControlPanel.h"
#include "light.h"
#include "camera.h"

class testApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
        void exit();
		void keyPressed  (int key);
        void updateSettings();
//        void drawPerson(ofPoint pos, ofVec3f dir);

        vector <Light*> lights;

        // Globals
        bool debug, rotate, sliderControl;
        float depthMulti;
        int numberOfLights;
        ofVec3f room;
        ofEasyCam worldCam;
        Camera * cam;

        // Arduino
        void setupArduino();
        void writeArduino();
        ofSerial serial;
        string buffer;
        long limitBuffer;


        // control
        void setupGUI();
        ofxAutoControlPanel panel;
        int panelWidth; // Debugging away from img
        float threshold;

};

#include "testApp.h"


//--------------------------------------------------------------
void testApp::setup(){
    ofSetVerticalSync(true);
    //    ofEnableLighting();
    
    worldCam.disableMouseInput();
    
    numberOfLights =8;
    room = ofVec3f(1000,400,500);
    
    for(int i=0;i<numberOfLights;i++) {
        lights.push_back( new Light(
                                    ofVec3f(
                                            room.x/2,
                                            room.y/2,
                                            i*(room.z/numberOfLights)
                                        ), i ) );
    }
    
    setupGUI();
    cam = new Camera(&panel, lights);
    
    
    setupArduino();
}

//--------------------------------------------------------------
void testApp::update(){
    cam->update();
    worldCam.setTarget(ofVec3f(room.x/2,room.y/2,room.z/2));
    worldCam.setPosition(room.x, room.y, room.z);
    worldCam.setDistance(panel.getValueF("cameraDistance"));
    writeArduino();
    updateSettings();
}

//--------------------------------------------------------------
void testApp::draw(){
    ofBackgroundGradient(ofColor(200),ofColor(170) );
    
    glEnable(GL_DEPTH_TEST);
    
	worldCam.begin();
    
    if(rotate)
        ofRotateY(ofRadToDeg(  ofGetElapsedTimeMillis()*.0001 ));
    
    ofSetColor(200);
    
    for(int i=0;i<lights.size();i++) {
        lights[i]->draw();
    }

    cam->drawPeople();
    
    ofPushMatrix();
    ofSetColor(100,100,100);
    ofTranslate(room.x/2,0,room.z/2);
    ofScale(room.x,0,room.z);
    ofBox(1);
    ofPopMatrix();
    
    worldCam.end();
    
    
    
    if(debug) {
        ofSetHexColor(0x123456);
        
        for(int i=0;i<lights.size();i++) {
            lights[i]->debug();
        }
    }
    
    cam->draw();
    
    glDisable(GL_ALPHA_TEST);
    glDisable(GL_DEPTH_TEST);
}

void testApp::updateSettings(){
    for( int i=0;i<numberOfLights;i++) {
        float pwr = panel.getValueF("l" + ofToString(i) + "pwr");
        float x = panel.getValueF("l" + ofToString(i) + "x");
        float y = panel.getValueF("l" + ofToString(i) + "y");
        float z = panel.getValueF("l" + ofToString(i) + "z");
        lights[i]->setLocation(ofVec3f(x*room.x,y*room.y,z*room.z));
//        if(sliderControl) lights[i]->setStrength(pwr);
    }
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
    switch(key) {
        case ' ':
            break;
        case 'r':
            rotate =!rotate;
        case 'd':
            debug = !debug;
            break;
            
        case 'm':
			if(worldCam.getMouseInputEnabled()) worldCam.disableMouseInput();
			else worldCam.enableMouseInput();
			break;
        case 'b':
            cam->setBackground();
            break;
    }
}



///////////////////
///////////////////
///////////////////
void testApp::setupGUI() {
    panelWidth = 300;
    panel.setup(panelWidth, 1024);
    
    panel.addPanel("PointCloud");
    panel.addSlider("cameraDistance",700,0,1000,false);
    panel.addSlider("TimeBetweenUpdate", 1000, 0,5000,true);
    panel.addSlider("lightThresh",100,0,1000,true);
    
    for( int i=0;i<numberOfLights;i++) {
        panel.addPanel("Light" + ofToString(i));    void setLocation(ofVec3f _position);
        panel.addLabel("Light" + ofToString(i) );
        panel.addSlider("l" + ofToString(i) + "pwr", 1., 0., 1., false);
        panel.addSlider("l" + ofToString(i) + "x", lights[i]->getLocation().x / room.x, 0., 1., false);
        panel.addSlider("l" + ofToString(i) + "y", lights[i]->getLocation().y / room.y, 0., 1., false);
        panel.addSlider("l" + ofToString(i) + "z", lights[i]->getLocation().z / room.z, 0., 1., false);
    }
    
    panel.addPanel("Tracking Bits");
    panel.addLabel("Image Processing");
    panel.addSlider("cvBlur",10,0,100,true);
    panel.addSlider("maxThreshold", 15, 0, 255, true);
    panel.addSlider("minAreaRadius", 7, 0, 640, true);
    panel.addSlider("maxAreaRadius", 100, 0, 640, true);
    panel.addSlider("DepthMultiplier", .01,0,5.,false);
    panel.addLabel("Background Subtraction");
    panel.addSlider("learningTime",900,0,2000,true);
    panel.addSlider("backgroundThresh",10,0,50,true);
    panel.addToggle("resetBg", false);
    panel.addSlider("OverlapDistance", 500, 0,1000,true);
    
    panel.addSlider("idScale",.3,0,1.,false);
    panel.addSlider("idPosx",-500,-700,700,true);
    panel.addSlider("idPosy",-500,-700,700,true);
    
    panel.addPanel("Kinect");
    panel.addSlider("angle", 0, -40, 40, true);
}

///////////////////
///////////////////
///////////////////
void testApp::setupArduino() {
    
    
    //    New proto
    // Lights in ID order, CSV new line delin
    serial.listDevices();
	serial.setup(7,115200);

    
}

void testApp::writeArduino() {
    if(limitBuffer <= ofGetElapsedTimeMillis()) {
        limitBuffer = ofGetElapsedTimeMillis() + panel.getValueI("TimeBetweenUpdate");
        buffer = "";
        for(int l = 0; l<lights.size();l++) {
            lights[l]->update();
            serial.writeByte((unsigned char)lights[l]->getStrength());
            buffer += ofToString(lights[l]->getStrength());
            
           // if(l!=lights.size()-1) buffer += ",";
            
         //   serial.writeByte(',');
        }
        buffer += "\n";
        serial.writeByte('a');
        ofLog() << buffer;
    }
}

void testApp::exit() {
    serial.writeByte('e');
}
#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){

}

//--------------------------------------------------------------
void testApp::update(){
    contourFinder.setMinAreaRadius(panel.getValueI("minAreaRadius"));
    contourFinder.setMaxAreaRadius(panel.getValueI("maxAreaRadius"));
    contourFinder.setThreshold(panel.getValueI("maxThreshold"));
    contourFinder.setAutoThreshold(true);
    //contourFinder.setInvert(true);
    
    background.setLearningTime(panel.getValueI("learningTime"));
    background.setThresholdValue(panel.getValueI("backgroundThresh"));
    
    if(panel.hasValueChanged("angle")) {
        angle = panel.getValueI("angle");
        kinect.setCameraTiltAngle(angle);
    }
    
    
    if(panel.getValueB("resetBg")) {
        //        background.reset();
        threshMat = kDepthMat.clone();
        panel.setValueB("resetBg",false);
    }
    
    if(!kinect.isConnected()) {
    } else {
        kinect.update();
        if(kinect.isFrameNew()) {
            if(avgCounter == 0) {
                kDepthMat = toCv(kinect.getDepthPixelsRef());
                blur(kDepthMat, panel.getValueI("cvBlur"));
                avgMat = kDepthMat;
                avgCounter++;
            } else
                if(avgCounter < 2) { // If averaging the frames
                    kDepthMat = toCv(kinect.getDepthPixelsRef());
                    blur(kDepthMat, panel.getValueI("cvBlur"));
                    avgMat += kDepthMat/3;
                    avgCounter++;
                    
                } else { // Act on the average
                    cameras[0]->isNewFrame(true);
                    kDepthMat = toCv(kinect.getDepthPixelsRef());
                    blur(kDepthMat, panel.getValueI("cvBlur"));
                    avgMat += kDepthMat/3;
                    // threshMat = ( (kDepthMat * .3) + (threshMat))/2 ; // Attempt at an adapting threshold...
                    cv::absdiff(avgMat, threshMat, avgMat);
                    fillHoles(avgMat);
                    contourFinder.findContours(avgMat);
                    toOf(avgMat,kDepth);
                    kDepth.update();
                    // brush = getContour(&contourFinder);
                    //               float distance;
                    for(int j=0;j<lights.size();j++) {
                        float distance = 0;
                        for( int i=0;i<contourFinder.size();i++) {
                            ofPoint center = toOf(contourFinder.getCenter(i));
                            float depth, multi;
                            if(kinect.isConnected()) {
                                depth = kinect.getDistanceAt(center)*depthMulti;
                                multi = 1 * pow((float)depth,1.1f);
                            } else {
                                multi = 1 * pow((float)center.y,1.1f);
                            }
                            
                            distance += lights[j]->getLocation().squareDistance(ofVec3f(multi, 30, center.x)) * .001;
                        }
                        lights[j]->setTotalDist(distance);
                    }
                    
                    // This is the weird bit that chooses which light is active
                    int pwrLight = 0;
                    for(int i=0;i<lights.size();i++) {
                        lights[i]->isActive(false);
                        if(lights[i]->getTotalDist() <= lights[pwrLight]->getTotalDist()) {
                            pwrLight = i;
                        }
                    }
                    lights[pwrLight]->isActive(true); 
                    avgCounter = 0; // This resets the frame averaging.
                }
        } else {
            cameras[0]->isNewFrame(false);
        }
        
        
    }
}

//--------------------------------------------------------------
void testApp::draw(){
    ofBackgroundGradient(ofColor(200),ofColor(170) );
    
    glEnable(GL_DEPTH_TEST);
    
	camera.begin();
    //        ofTranslate(0,-400);
    if(rotate)
        ofRotateY(ofRadToDeg(  ofGetElapsedTimeMillis()*.0001 ));
    
    ofSetColor(200);
    
    //        testLight->draw();
    for(int i=0;i<lights.size();i++) {
        lights[i]->draw();
    }
    //        for(int i=0;i<people.size();i++) {
    //            people[i]->draw();
    //        }
    
    for(int i=0;i<cameras.size();i++) {
        cameras[i]->draw();
    }
    
    for(int i = 0; i < contourFinder.size(); i++) {
        ofPoint center = toOf(contourFinder.getCenter(i));
        //ofSphere(center.x, 10, center.y, 20);
        float depth, multi;
        if(kinect.isConnected()) {
            depth = kinect.getDistanceAt(center)*.1;
            multi = 1 * pow((float)depth,1.1f);
        } else {
            multi = 1 * pow((float)center.y,1.1f);
        }
        drawPerson(ofPoint(multi,  center.x), toOf(contourFinder.getVelocity(i)));
    }
    
    ofPushMatrix();
    ofSetColor(100,100,100);
    ofTranslate(room.x/2,0,room.z/2);
    ofScale(room.x,0,room.z);
    ofBox(1);
    ofPopMatrix();
    
    camera.end();
    
    
    
    if(debug) {
        ofSetHexColor(0x123456);
        
        for(int i=0;i<lights.size();i++) {
            lights[i]->debug();
        }
        
        for(int i = 0; i < contourFinder.size(); i++) {
            ofPoint center = toOf(contourFinder.getCenter(i));
            float depth;
            if(kinect.isConnected())
                depth = kinect.getDistanceAt(center)*.1;
            else depth = 1.0;
        }
        drawCamDebug();
    }
    
    
    
    glDisable(GL_ALPHA_TEST);
    glDisable(GL_DEPTH_TEST);
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
			if(camera.getMouseInputEnabled()) camera.disableMouseInput();
			else camera.enableMouseInput();
			break;
        case 'b':
            ofxCv::copy(kDepthMat, threshMat);
            break;
    }
}

void testApp::setupCamera() {
    kinect.setRegistration(true);
    ofLog() << "Starting first kinect";
    kinect.init(false, false, true); // infrared=false, video=true, texture=true
    kinect.open(0);
    kinect.setCameraTiltAngle(angle);
    
    if(!kinect.isConnected()) {
        cam.initGrabber(640, 480);
    }
    
    thresh.allocate(640, 480, OF_IMAGE_GRAYSCALE);
    kDepth.allocate(640, 480, OF_IMAGE_GRAYSCALE);
    kDepthMat.create(480, 640, CV_8UC1);
    //threshMat.create(480, 640, CV_32F);
    imitate(threshMat, kDepthMat);
    imitate(avgMat, kDepthMat);
    //    imitate(kDepth, kDepthMat);
    //    imitate(thresh, threshMat);
    
    background.setThresholdValue(panel.getValueI("backgroundThresh"));
    background.setLearningTime(panel.getValueI("learningTime"));
    
    contourFinder.setMinAreaRadius(panel.getValueI("minAreaRadius"));
    contourFinder.setMaxAreaRadius(panel.getValueI("maxAreaRadius"));
    contourFinder.setThreshold(panel.getValueI("maxThreshold"));
    // wait for half a frame before forgetting something
    contourFinder.getTracker().setPersistence(15);
    // an object can move up to 32 pixels per frame
    contourFinder.getTracker().setMaximumDistance(32);
    avgCounter = 0;
}

void testApp::drawCamera() {
    
    ofPushMatrix();
    glDisable(GL_DEPTH_TEST);
    ofTranslate(ofGetWidth()-(kinect.getWidth()/2), ofGetHeight()-(kinect.getHeight()/2));
    ofScale(.5,.5);
    ofSetColor(255);
    ofSetLineWidth(1);
    if(!kinect.isConnected()) cam.draw(0, 0);
    else kDepth.draw(0,0);
    
    //        ofImage debug = toOf(kDepthMat);
    contourFinder.draw();
    
    for(int i = 0; i < contourFinder.size(); i++) {
        ofPoint center = toOf(contourFinder.getCenter(i));
        
        float depth;
        if(kinect.isConnected())
            depth = kinect.getDistanceAt(center);
        else depth = 0;;
        ofSetColor(255);
        contourFinder.getPolyline(i).draw();
        ofPushMatrix();
        ofTranslate(center.x, center.y);
        int label = contourFinder.getLabel(i);
        ofDrawBitmapString(ofToString(label) + ":" + ofToString(depth*depthMulti), 0, 12);
        ofVec2f velocity = toOf(contourFinder.getVelocity(i));
        ofScale(5, 5);
        ofLine(0, 0, velocity.x, velocity.y);
        ofPopMatrix();
    }
    glDisable(GL_DEPTH_TEST);
    //    thresh.draw(thresh.width, 0, 2, 256,192);
    ofPopMatrix(); // For panel
}

// This was taken from
// http://www.morethantechnical.com/2011/
// 03/05/neat-opencv-smoothing-trick-when-kineacking-kinect-hacking-w-code/
void testApp::fillHoles(cv::Mat src, cv::Mat dst) {
    cv::Mat depthf(cv::Size(640,480),CV_8UC1);
    src.convertTo(depthf, CV_8UC1, 255.0/2048.0);
    cv::Mat _tmp,_tmp1; //minimum observed value is ~440. so shift a bit
    cv::Mat(depthf - 400.0).convertTo(_tmp1,CV_64FC1);
    
    ofPoint minLoc; double minval,maxval;
    minMaxLoc(_tmp1, &minval, &maxval, NULL, NULL);
    _tmp1.convertTo(depthf, CV_8UC1, 255.0/maxval);  //linear interpolation
    
    //use a smaller version of the image
    cv::Mat small_depthf; cv::resize(depthf,small_depthf,cv::Size(),0.2,0.2);
    //inpaint only the "unknown" pixels
    cv::inpaint(small_depthf,(small_depthf == 255),_tmp1,5.0,INPAINT_TELEA);
    
    resize(_tmp1, _tmp, depthf.size());
    imitate(dst,depthf);
    _tmp.copyTo(dst, (dst == 255));
    //   ofxCv::copy(depthf,dst);
}

void testApp::fillHoles(cv::Mat _mat) {
    fillHoles(_mat, _mat);
}
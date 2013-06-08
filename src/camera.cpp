//
//  camera.cpp
//  seeing-like-software
//
//  Created by Andrew Lovett Barron on 2013-06-02.
//
//

#include "camera.h"


using namespace cv;
using namespace ofxCv;

Camera::Camera(ofxAutoControlPanel * _panel, vector<Light*> _lights) {
    isNew = false;
    panel = _panel;
    lights = _lights;
    setup();
}

Camera::~Camera() {
    // Cleanup is good.
    kinect.close();
    kinect.clear();
}


void Camera::setup() {
    kinect.setRegistration(true);
    kinect.init(false, false, true); // infrared=false, video=true, texture=true
    kinect.open(0);
    
    // If there's no kinect, use webcam. Better for my mode of
    // coding, i.e. parks, coffee shops, airplanes.
    if(!kinect.isConnected()) {
        cam.initGrabber(640, 480);
    }
    
   // thresh.allocate(640, 480, OF_IMAGE_GRAYSCALE); // Threshold image
    kDepth.allocate(640, 480, OF_IMAGE_GRAYSCALE); // ofImage to render
    kDepthMat.create(480, 640, CV_8UC1);           // cv::Mat for kDepth
    //threshMat.create(480, 640, CV_32F);
//    imitate(threshMat, kDepthMat);
    imitate(avgMat, kDepthMat);
    imitate(kDepthMat,avgMat);
    imitate(prevFrame,avgMat);
    imitate(threshMat,avgMat);
    //    imitate(kDepth, kDepthMat);
    //    imitate(thresh, threshMat);
    
    
    background.setLearningTime(900);
	background.setThresholdValue(10);
    
    background.setThresholdValue(panel->getValueI("backgroundThresh"));
    background.setLearningTime(panel->getValueI("learningTime"));
    
    // Setup the contour finder
    contourFinder.setMinAreaRadius(panel->getValueI("minAreaRadius"));
    contourFinder.setMaxAreaRadius(panel->getValueI("maxAreaRadius"));
    contourFinder.setThreshold(panel->getValueI("maxThreshold"));
    
    // wait for half a frame before forgetting something
    contourFinder.getTracker().setPersistence(15);
    
    // an object can move up to 32 pixels per frame
    contourFinder.getTracker().setMaximumDistance(32);
    avgCounter = 0;
}

void Camera::draw() {
    ofPushMatrix();
    
    // Draw the depth buffer
    glDisable(GL_DEPTH_TEST);

    // This just sets up the image debug in the right place and scales it
    ofTranslate(ofGetWidth()-(kinect.getWidth()/2), ofGetHeight()-(kinect.getHeight()/2));
    ofScale(.5,.5);
    ofSetColor(255);
    ofSetLineWidth(1);

    // Drawing this will applay to both cam and kinect
    kDepth.draw(0,0);
    
    // use ofxCountour draw
    contourFinder.draw();
    
    // Draw the individual polylines
    for(int i = 0; i < contourFinder.size(); i++) {
        ofPoint center = toOf(contourFinder.getCenter(i));
        
        // Draw depth
        float depth;
        if(kinect.isConnected())
            depth = kinect.getDistanceAt(center);
        else depth = 0;;
        ofSetColor(255);
        contourFinder.getPolyline(i).draw();
        ofPushMatrix();
        ofTranslate(center.x, center.y);
        
        // Draw depth IDed in center
        ofDrawBitmapString(ofToString(depth), 0, 12);
        
        // Draws direction of poly movement
        ofVec2f velocity = toOf(contourFinder.getVelocity(i));
        ofScale(5, 5);
        ofLine(0, 0, velocity.x, velocity.y);
        ofPopMatrix();
    }
    glDisable(GL_DEPTH_TEST);
    ofPopMatrix();
}

void Camera::update() {
    contourFinder.setMinAreaRadius(panel->getValueI("minAreaRadius"));
    contourFinder.setMaxAreaRadius(panel->getValueI("maxAreaRadius"));
    contourFinder.setThreshold(panel->getValueI("maxThreshold"));
    contourFinder.setAutoThreshold(true);
    //contourFinder.setInvert(true);
    
    background.setLearningTime(panel->getValueI("learningTime"));
    background.setThresholdValue(panel->getValueI("backgroundThresh"));
    
    
    if(panel->getValueB("resetBg")) {
        //        background.reset();
        threshMat = kDepthMat.clone();
        panel->setValueB("resetBg",false);
    }
    
    // Update cam or kinect, flag as a new frame
    if(!kinect.isConnected()) {
        cam.update();
        if(cam.isFrameNew()) isNew = true;
    } else {
        kinect.update();
        if(kinect.isFrameNew()) isNew = true;
    }
    
    // If we have a new frame with either the Kinect or Camera
    if(isNew) {
        if(!kinect.isConnected()) {
//            kDepthMat = ;
            Mat temp = toCv(cam.getPixelsRef());
            convertColor(temp, kDepthMat, CV_RGB2GRAY); // Convert in place
        }
        else {
         kDepthMat = toCv(kinect.getDepthPixelsRef());   
        }
        blur(kDepthMat, panel->getValueI("cvBlur"));
        avgMat = kDepthMat;
        
        // Averaging across three frames
        if(avgCounter == 0) {
            avgCounter++;
        }
        else if(avgCounter < 2) { // If averaging the frames
                avgMat += kDepthMat/3;
                avgCounter++;
                
        }
        else { // Act on the average
            avgMat += kDepthMat/3;
            // threshMat = ( (kDepthMat * .3) + (threshMat))/2 ; // Attempt at an adapting threshold...
            
            fillHoles(avgMat); // Reduce how often "fill hole" is used for speed
            copy(avgMat, threshMat);
//            cv::absdiff(avgMat, prevFrame, threshMat);
            
//            copy(avgMat, prevFrame);
            
            contourFinder.findContours(threshMat);
            toOf(threshMat,kDepth); // Online convert mat to ofImage when necessary
            kDepth.update(); // Update the glTexture w/ cv::mat updated info
            

            // This is where we calculate whether a light is turned on or not.
            for(int j=0;j<lights.size();j++) {
                float distance = 0;
                // Check for contours, which we define is people.
                if(contourFinder.size() > 0) {
                for( int i=0;i<contourFinder.size();i++) {
                    ofPoint center = toOf(contourFinder.getCenter(i));
                    float depth;
                    if(kinect.isConnected())
                        depth = kinect.getDistanceAt(center)*.1;
                    else depth = 100;
                    float multi = 1 * pow((float)depth,1.1f);
                    
                    ofVec3f personCoord = ofVec3f(center.x, multi, center.y);
                    
                    ofVec3f cur = lights[j]->getLocation();
                    if(cur.squareDistance(personCoord) * .001 < panel->getValueI("lightThresh") )
                        lights[j]->setActive(true);
                    else
                        lights[j]->setActive(false);

                }
                }
                else {
                        lights[j]->setActive(false);
                    
                    
                }
                // lights[j]->setTotalDist(distance);
            }
            avgCounter = 0; // This resets the frame averaging.
        }
        isNew = false;
    }
        
        
    
}


// This was taken from
// http://www.morethantechnical.com/2011/
// 03/05/neat-opencv-smoothing-trick-when-kineacking-kinect-hacking-w-code/
void Camera::fillHoles(cv::Mat src, cv::Mat dst) {
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

void Camera::smoothDetected() {
    
}


void Camera::setBackground() {
    ofxCv::copy(kDepthMat, threshMat);
}

vector<ofPoint> Camera::getPeople() {
    vector<ofPoint> ppl;
//    for( int i=0;i<contourFinder.size();i++) {
//        ofPoint center = toOf(contourFinder.getCenter(i));
//        ppl.push_back(ofPoint(multi,  center.x));
//    }
    return ppl;
}

void Camera::drawPeople() {
    for( int i=0;i<contourFinder.size();i++) {
        ofPoint center = toOf(contourFinder.getCenter(i));
        drawPerson(center);
    }
}

void Camera::drawPerson(ofPoint _pos) {
    
    
    float depth, multi;
    depth = kinect.getDistanceAt(_pos)*.1;
    multi = 1 * pow((float)depth,1.1f);
    
    ofPoint pos = ofPoint(multi, _pos.x);
    
    // Personal sanity check...
    ofPushMatrix();
    ofTranslate(ofVec3f(_pos.x, 10, _pos.y));
    ofSetColor(255,120,120);
    ofSphere(10);
    ofPopMatrix();
    
    // Actual people rendering
    ofPushMatrix();
    ofTranslate(pos.x,30,pos.y);
    ofSetColor(255);
    ofDrawBitmapString("x" + ofToString(pos.x) + " y" + ofToString(pos.y),0,0);
    ofSetColor(127);
    ofSphere(0,60,0,20);
    ofScale(20,60,10);
    ofBox(1);
    ofPopMatrix();
}

void Camera::fillHoles(cv::Mat _mat) {
    fillHoles(_mat, _mat);
}
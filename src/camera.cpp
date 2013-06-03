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
    panel = _panel;
    lights = _lights;
    setup();
}

Camera::~Camera() {
    kinect.close();
    kinect.clear();
}


void Camera::setup() {
    kinect.setRegistration(true);
    ofLog() << "Starting first kinect";
    kinect.init(false, false, true); // infrared=false, video=true, texture=true
    kinect.open(0);
    
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
    
    
    background.setLearningTime(900);
	background.setThresholdValue(10);
    
    background.setThresholdValue(panel->getValueI("backgroundThresh"));
    background.setLearningTime(panel->getValueI("learningTime"));
    
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
        ofDrawBitmapString(ofToString(label) + ":" + ofToString(depth), 0, 12);
        ofVec2f velocity = toOf(contourFinder.getVelocity(i));
        ofScale(5, 5);
        ofLine(0, 0, velocity.x, velocity.y);
        ofPopMatrix();
    }
    glDisable(GL_DEPTH_TEST);
    //    thresh.draw(thresh.width, 0, 2, 256,192);
    ofPopMatrix(); // For panel
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
    
    if(!kinect.isConnected()) {
        cam.update(); 
    } else {
        kinect.update();
        if(kinect.isFrameNew()) {
            if(avgCounter == 0) {
                kDepthMat = toCv(kinect.getDepthPixelsRef());
                blur(kDepthMat, panel->getValueI("cvBlur"));
                avgMat = kDepthMat;
                avgCounter++;
            } else
                if(avgCounter < 2) { // If averaging the frames
                    kDepthMat = toCv(kinect.getDepthPixelsRef());
                    blur(kDepthMat, panel->getValueI("cvBlur"));
                    avgMat += kDepthMat/3;
                    avgCounter++;
                    
                } else { // Act on the average
                    kDepthMat = toCv(kinect.getDepthPixelsRef());
                    blur(kDepthMat, panel->getValueI("cvBlur"));
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
                                depth = kinect.getDistanceAt(center);
                                multi = 1 * pow((float)depth,1.1f);
                            } else {
                                multi = 1 * pow((float)center.y,1.1f);
                            }
                            
                           // distance += lights[j]->getLocation().squareDistance(ofVec3f(multi, 30, center.x)) * .001;
                            ofVec3f cur = lights[j]->getLocation();
                            if(cur.squareDistance(ofVec3f(multi, 30, center.x) *.00001) < panel->getValueI("lightThresh") )
                                lights[j]->setActive(true);
                            else
                                lights[j]->setActive(false);

                        }
                        // lights[j]->setTotalDist(distance);
                    }
                    
                    avgCounter = 0; // This resets the frame averaging.
                }
        }
        
        
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


void Camera::setBackground() {
    ofxCv::copy(kDepthMat, threshMat);
}

vector<ofPoint> Camera::getPeople() {
    vector<ofPoint> ppl;
    for( int i=0;i<contourFinder.size();i++) {
        ofPoint center = toOf(contourFinder.getCenter(i));
       float depth, multi;
       if(kinect.isConnected()) {
           depth = kinect.getDistanceAt(center)*.1;
           multi = 1 * pow((float)depth,1.1f);
       } else {
           multi = 1 * pow((float)center.y,1.1f);
       }
        ppl.push_back(ofPoint(multi,  center.x));
    }
    return ppl;
}

void Camera::fillHoles(cv::Mat _mat) {
    fillHoles(_mat, _mat);
}
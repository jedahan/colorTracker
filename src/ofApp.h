#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxLivingRoom.h"

class ofApp : public ofBaseApp {
    public:
        void setup();
        void update();
        void draw();

        void mousePressed(int x, int y, int button);
        void sendContours();

        int width = 320;
        int height = 240;
    
        unsigned int hueRange = 7;
        const static unsigned int numberOfHuesToTrack = 4;
        unsigned int huesToTrack[numberOfHuesToTrack] = {13, 63, 102, 167};
        unsigned int huesToTrackIndex = 0;
    
        vector<ofxCvGrayscaleImage> hues;
    
        ofxLivingRoom room;
    
        ofVideoGrabber movie;
        ofxCvColorImage rgb;
        ofxCvColorImage hsb;
    
        ofxCvGrayscaleImage hue;
        ofxCvGrayscaleImage sat;
        ofxCvGrayscaleImage bri;
    
        vector<ofxCvContourFinder> contours;
};

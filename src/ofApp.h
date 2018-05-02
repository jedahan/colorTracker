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
    
        uint8_t hueRange = 10;
        const uint8_t numberOfHuesToTrack = 4;
        uint8_t huesToTrack[4] = {0, 64, 128, 192};
        uint8_t huesToTrackIndex = 0;
    
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

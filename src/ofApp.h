#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxOsc.h"

class ofApp : public ofBaseApp {
    public:
        void setup();
        void update();
        void draw();
        void sendContours();

        void mousePressed(int x, int y, int button);

        int width, height;
        int findHue;
        ofVideoGrabber movie;
        ofxCvColorImage rgb,hsb;
        ofxCvGrayscaleImage hue,sat,bri,filtered;
        ofxCvContourFinder contours;

        std::string roomdb_host;
        int roomdb_port;
        ofxOscSender sender;
};

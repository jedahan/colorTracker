#include "ofApp.h"

void ofApp::setup() {
    ofSetFrameRate(30);

    ofBackground(100, 100, 100);

    room.setup();
    
    hues.resize(numberOfHuesToTrack);
    contours.resize(numberOfHuesToTrack);
    
    for (int i = 0; i < numberOfHuesToTrack; i++) {
        hues[i].allocate(width, height);
    }
    
    rgb.allocate(width, height);
    hsb.allocate(width, height);
    
    hue.allocate(width, height);
    sat.allocate(width, height);
    bri.allocate(width, height);

    movie.setup(width, height, true);
}

void ofApp::update(){

    movie.update();

    if (movie.isFrameNew()) {

        //copy webcam pixels to rgb image
        rgb.setFromPixels(movie.getPixels());

        hsb = rgb; // copy rgb

        hsb.convertRgbToHsv();

        //store the first channel as hue
        hsb.convertToGrayscalePlanarImage(hue, 0);

        //filter image based on the hue value were looking for
        for (int x = 0; x < numberOfHuesToTrack; x++) {
            for (int i = 0; i < width * height; i++) {
                if (huesToTrack[x] < hueRange) {
                    if (hue.getPixels()[i] > 255 + huesToTrack[x] - hueRange ||
                        hue.getPixels()[i] < huesToTrack[x] + hueRange) {
                        hues[x].getPixels()[i] = 255;
                    } else {
                        hues[x].getPixels()[i] = 0;
                    }
                } else {
                    hues[x].getPixels()[i] = ofInRange(
                                        hue.getPixels()[i],
                                        (255 + huesToTrack[x] - hueRange) % 255,
                                        huesToTrack[x] + hueRange) ? 255 : 0;
                }
            }
            hues[x].erode();
            hues[x].erode();
            hues[x].dilate();
            hues[x].dilate();
            hues[x].flagImageChanged();
            contours[x].findContours(hues[x], 25, width * height / 10, 50, false);
        }
        sendContours();
    }
}

void ofApp::sendContours() {
    for (int x = 0; x < numberOfHuesToTrack; x++) {
        for (int i = 0; i < contours[x].nBlobs; i++) {
            auto blob = contours[x].blobs[i];
            auto cx = blob.centroid.x / width;
            auto cy = blob.centroid.y / height;
            
            std::string fact = "color" + ofToString(x) + ofToString(i) + " is hue " + ofToString(huesToTrack[x]) + " at (" + ofToString(cx) + ", " + ofToString(cy) + ")";
            //ofLog() << fact << std::endl;
            room.assertFact(fact);
        }
    }
}

void ofApp::draw() {
    
    ofSetColor(255,255,255);

    //draw all cv images
    rgb.draw(0 * width, 0);
    hsb.draw(1 * width, 0);
    rgb.draw(2 * width, 0);
    for (int x = 0; x < numberOfHuesToTrack; x++) {
        ofColor thisColor = ofColor::fromHsb(huesToTrack[x], 200, 200);
        ofSetColor(thisColor);
        hues[x].draw(x * width, height);
        contours[x].draw(x * width, 2*height);
        
        ofFill();
        
        //draw red circles for found blobs
        std::string colorString = ofToString(contours[x].nBlobs) + " " + ofToString(huesToTrack[x]);
        ofDrawBitmapStringHighlight(colorString,
                                    x * width, ofGetHeight() - 16,
                                    thisColor);
        
        for (int i = 0; i < contours[x].nBlobs; i++) {
            auto blob = contours[x].blobs[i];
            auto ratio = blob.boundingRect.getAspectRatio();
            // if (ratio > 2 || ratio < 0.5) return; // non-square
            auto radius = (blob.boundingRect.width + blob.boundingRect.height) / 3;
            ofDrawCircle((2 * width) + blob.centroid.x, blob.centroid.y, radius);
        }
    }
}

void ofApp::mousePressed (int x, int y, int button) {
    //calculate local mouse x, y in image
    int mx = x % width;
    int my = y % height;
    huesToTrackIndex = (huesToTrackIndex + 1) % numberOfHuesToTrack;
    //get hue value on mouse position
    huesToTrack[huesToTrackIndex] = hue.getPixels()[my*width+mx];
}

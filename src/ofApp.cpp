#include "ofApp.h"

void ofApp::setup() {
    ofSetFrameRate(30);
    isConnected = false;
    status = "not connected";

    ofBackground(100,100,100);

    roomdb_uri = ofGetEnv("ROOMDB_URI");

    if (roomdb_uri == "") {
      roomdb_uri = "http://localhost:3000";
    }

    ofLog() << "roomdb_uri is " << roomdb_uri << std::endl;

    socketIO.setup(roomdb_uri);
    ofAddListener(socketIO.notifyEvent, this, &ofApp::gotEvent);
    ofAddListener(socketIO.connectionEvent, this, &ofApp::onConnection);

    width = 320;
    height = 240;

    movie.setup(width, height, true);

    //reserve memory for cv images
    rgb.allocate(width, height);
    hsb.allocate(width, height);
    hue.allocate(width, height);
    sat.allocate(width, height);
    bri.allocate(width, height);
    filtered.allocate(width, height);
}

template<typename ... Args>
string string_format( const std::string& format, Args ... args )
{
    size_t size = snprintf( nullptr, 0, format.c_str(), args ... ) + 1; // Extra space for '\0'
    unique_ptr<char[]> buf( new char[ size ] );
    snprintf( buf.get(), size, format.c_str(), args ... );
    return string( buf.get(), buf.get() + size - 1 ); // We don't want the '\0' inside
}

void ofApp::onConnection () {
  isConnected = true;
  socketIO.bindEvent(assertEvent, "assert");
  ofAddListener(assertEvent, this, &ofApp::onAssertEvent);
}

void ofApp::update(){

    movie.update();

    if (movie.isFrameNew()) {

        //copy webcam pixels to rgb image
        rgb.setFromPixels(movie.getPixels());

        //mirror horizontal
        rgb.mirror(false, true);

        /*
        for (int i = 0; i < width * height; i++) {
            rgb.getPixels()[i] = rgb.getPixels()[i] > 0x01000000 ? 0 : 255;
        }
        */

        hsb = rgb; // copy rgb

        hsb.convertRgbToHsv();

        //store the three channels as grayscale images
        hsb.convertToGrayscalePlanarImages(hue, sat, bri);

        //filter image based on the hue value were looking for
        for (int i=0; i<width*height; i++) {
            filtered.getPixels()[i] = ofInRange(hue.getPixels()[i],findHue-5,findHue+5) ? 255 : 0;
        }

        filtered.flagImageChanged();
        //run the contour finder on the filtered image to find blobs with a certain hue
        contours.findContours(filtered, 25, width*height/2, 5, false);

        sendContours();
    }
}

void ofApp::sendContours() {
    for (int i = 0; i < contours.nBlobs; i++) {
        auto blob = contours.blobs[i];
        auto x = blob.centroid.x / width;
        auto y = blob.centroid.y / height;
        std::string facts = string_format("#label%d is a label at (%03.2f, %03.2f)", i, x, y);
        std::string event = "assert";
        socketIO.emit(event, facts);
    }
}

void ofApp::draw(){
    ofSetColor(255,255,255);

    //draw all cv images
    rgb.draw(0,0);
    hsb.draw(640,0);
    hue.draw(0,240);
    sat.draw(320,240);
    bri.draw(640,240);
    filtered.draw(0,480);
    contours.draw(0,480);

    ofSetColor(255, 0, 0);
    ofFill();

    //draw red circles for found blobs
    for (int i=0; i<contours.nBlobs; i++) {
        ofDrawCircle(contours.blobs[i].centroid.x, contours.blobs[i].centroid.y, 20);
    }

    ofDrawBitmapStringHighlight(ofApp::status, 20, 20);
}

void ofApp::mousePressed (int x, int y, int button) {
    //calculate local mouse x,y in image
    int mx = x % width;
    int my = y % height;

    //get hue value on mouse position
    findHue = hue.getPixels()[my*width+mx];
}

void ofApp::gotEvent (string& name) {
    status = name;
}

void ofApp::onAssertEvent (ofxSocketIOData& data) {
    auto result = data.getVector();
    for (uint8_t i = 0; i < result.size(); i++) {
      ofLogNotice("ofxSocketIO", ofToString(result[i]));
    }
}

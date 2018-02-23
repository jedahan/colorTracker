#include "ofApp.h"

void ofApp::setup() {

    ofSetFrameRate(30);

    ofBackground(100,100,100);

    std::string env_host = ofGetEnv("ROOMDB_HOST");
    std::string env_port = ofGetEnv("ROOMDB_PORT");

    if (env_host == "") env_host = DEFAULT_HOST;
    if (env_port == "") env_port = DEFAULT_PORT;

    roomdb_host = env_host;
    roomdb_port = ofToInt(env_port);

    sender.setup(roomdb_host, roomdb_port);

    ofLog() << "roomdb_host is " << roomdb_host << std::endl;
    ofLog() << "roomdb_port is " << roomdb_port << std::endl;

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

void ofApp::update(){

    movie.update();

    if (movie.isFrameNew()) {

        //copy webcam pixels to rgb image
        rgb.setFromPixels(movie.getPixels());

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

        ofxOscMessage m;
        m.setAddress("/assert");
        std::string facts = string_format("#label%d is a label at (%03.2f, %03.2f)", i, x, y);
        m.addStringArg(facts);
        sender.sendMessage(m, false);

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
}

void ofApp::mousePressed (int x, int y, int button) {
    //calculate local mouse x,y in image
    int mx = x % width;
    int my = y % height;

    //get hue value on mouse position
    findHue = hue.getPixels()[my*width+mx];
}

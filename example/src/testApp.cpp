#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup() {

    ofBackgroundHex(0x65868D);
    bPressedInside = false;
    int w = 320; int h = 240;
    grabber.initGrabber(w, h);
    colorImage.allocate(w, h);
    grayImage.allocate(w, h);
    
    confidenceMin = 70;
    cropRect.set(10, 10, w/2, h/2);
}

//--------------------------------------------------------------
void testApp::update() {
    
    grabber.update();
    
    if(grabber.isFrameNew()) {
        
        colorImage.setFromPixels(grabber.getPixelsRef());
        grayImage = colorImage;
    
        // clear out all the old matches
        matches.clear();
        
        // look for any of the objects in the source image
        for (int i=0; i<objects.size(); i++) {
            
            int nFound = matcher.findMatches(grayImage, objects[i], confidenceMin);
            
            // copy over the matches and store the object id
            for (int j=0; j<matcher.matches.size(); j++) {
                matches.push_back(matcher.matches[j]);
                matches.back().objectID = i;
            }
            
            
            printf("Found %i in object %i\n", nFound, i);
        }
    }
    
}

//--------------------------------------------------------------
void testApp::draw() {
    
    ofSetColor(255);
    grayImage.draw(10, 10);

    ofNoFill();
    ofSetHexColor(0xF5ECCF);
    ofRect(cropRect);
    
    float ypos = 10;
    for(int i=0; i<objects.size(); i++) {
        
        ofPushMatrix();
        ofTranslate(ofGetWidth()-(objects[i].getWidth()+10), ypos);
        ofSetColor(255);
        objects[i].draw(0, 0);
        ofSetColor(255, 255, 0);
        ofDrawBitmapString(ofToString(i), 0, 15);
        for(int j=0; j<objects[i].pts.size(); j++) {
            ofSetHexColor(0xF5ECCF);
            ofCircle(objects[i].pts[j], 2);
        }
        ofPopMatrix();
        
        ypos += objects[i].getHeight()+10;
        
    }
    
    ofPushMatrix();
    ofTranslate(20 + grabber.getWidth(), 10);
    ofSetColor(255);
    grayImage.draw(0, 0);
    for (int i=0; i<matches.size(); i++) {
        matches[i].draw();
    }
    ofPopMatrix();
 
    ofSetHexColor(0xF5ECCF);
    ofDrawBitmapString("Draw a bounding box around object\nPress spacebar to capture object\nUp/Down to change confidence threshold\nFPS: "+ofToString(ofGetFrameRate(),0)+"\nConfidence Min:"+ofToString(confidenceMin)+"%", 10, grabber.getHeight()+30);
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){

    if(key == ' ') {
        objects.push_back(MatchObject());
        objects.back().allocate(cropRect.width, cropRect.height);
        grayImage.setROI(cropRect.x-10, cropRect.y-10, cropRect.width, cropRect.height);
        objects.back().setFromPixels(grayImage.getRoiPixels(), cropRect.width, cropRect.height);
        grayImage.resetROI();
    }
    
    if(key == 'c') {
        objects.clear();
    }
    
    if(key == OF_KEY_UP) {
        if(confidenceMin<100) confidenceMin ++;
    }
    if(key == OF_KEY_DOWN) {
        if(confidenceMin>0) confidenceMin --;
    }
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
    if(bPressedInside) {
        if(x > downPos.x+5 && y > downPos.y+5) {
            cropRect.width  = x - downPos.x;
            cropRect.height = y - downPos.y;
            if(cropRect.height+cropRect.y>grabber.getHeight()) cropRect.height = (grabber.getHeight()-cropRect.y)+10;
            if(cropRect.width+cropRect.x>grabber.getWidth()) cropRect.width = (grabber.getWidth()-cropRect.x)+10;
        }
    }
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
    if(ofRectangle(10, 10, grabber.getWidth(), grabber.getHeight()).inside(x, y)) {
        cropRect.setPosition(x, y);
        if(cropRect.width+cropRect.x>grabber.getWidth()) cropRect.width = (grabber.getWidth()-x)+10;
        if(cropRect.height+cropRect.y>grabber.getHeight()) cropRect.height = (grabber.getHeight()-y)+10;
        downPos.set(x, y);
        bPressedInside = true;
    }
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){
    bPressedInside = false;
}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}
//
//  ofxTemplateMatching.h
//  FastTemplateMatching
//
//  Created by Todd Vanderlin on 6/17/13.
//
//

#pragma once
#include "ofMain.h"
#include "ofxOpenCv.h"
#include "FastMatchTemplate.h"

class TemplateMatch : public ofRectangle {
public:
    float   confidence;
    int     objectID;
    TemplateMatch() {
        objectID = 0;
        confidence = 0.0f;
    }
    void draw() {
        ofNoFill();
        ofSetColor(255, 255, 0);
        ofRect(*this);
        ofDrawBitmapString(ofToString(objectID), getCenter());
        
    }
    TemplateMatch operator=( const TemplateMatch& match ) const {
        TemplateMatch tm;
        tm.set(match);
        tm.confidence = match.confidence;
        tm.objectID   = match.objectID;
        return tm;
    }
};

class MatchObject : public ofxCvGrayscaleImage {
public:
    
    vector <ofPoint> pts;

};

class ofxTemplateMatching {

public:
    
    vector <TemplateMatch> matches;
    
    int findMatches(ofxCvGrayscaleImage &source,
                    MatchObject &objectToFind,
                    int matchPercentage=70,
                    bool findMultipleTarget=true,
                    int maxToFind=5,
                    int numDownPyrs=2,
                    int searchExpansion=15) {
        
        matches.clear();
        vector <cv::Point> foundPoints;
        vector <double>    confidencesList;
        
        if(FastMatchTemplate(source.getCvImage(),
                             objectToFind.getCvImage(),
                             &foundPoints,
                             &confidencesList,
                             matchPercentage,
                             findMultipleTarget,
                             maxToFind,
                             numDownPyrs,
                             searchExpansion)) {
            
            objectToFind.pts.clear();
            
            for(int i=0; i<foundPoints.size(); i++) {
            
                
                const cv::Point& point = foundPoints[i];
                objectToFind.pts.push_back(ofPoint(point.x, point.y));
                
                TemplateMatch tm;
                tm.set(point.x - objectToFind.getWidth() / 2,
                       point.y - objectToFind.getHeight() / 2,
                       objectToFind.getWidth(),
                       objectToFind.getHeight());
                
                tm.confidence = confidencesList[i];
                matches.push_back(tm);
                
            }
            
            return foundPoints.size();
        }
        else {
            return 0;
        }
        
    }
    
    int findMatches(ofxCvGrayscaleImage &source,
                    ofxCvGrayscaleImage &objectToFind,
                    int matchPercentage=70,
                    bool findMultipleTarget=true,
                    int maxToFind=5,
                    int numDownPyrs=2,
                    int searchExpansion=15) {
       
        matches.clear();
        vector <cv::Point> foundPoints;
        vector <double>    confidencesList;
        
        if(FastMatchTemplate(source.getCvImage(),
                             objectToFind.getCvImage(),
                             &foundPoints,
                             &confidencesList,
                             matchPercentage,
                             findMultipleTarget,
                             maxToFind,
                             numDownPyrs,
                             searchExpansion)) {
            
            for(int i=0; i<foundPoints.size(); i++) {
                
                const cv::Point& point = foundPoints[i];
                
                TemplateMatch tm;
                tm.set(point.x - objectToFind.getWidth() / 2,
                       point.y - objectToFind.getHeight() / 2,
                       objectToFind.getWidth(),
                       objectToFind.getHeight());
                
                tm.confidence = confidencesList[i];
                matches.push_back(tm);
              
            }

            return foundPoints.size();
        }
        else {
            return 0;
        }

    }
};
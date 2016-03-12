#pragma once

#include "ofxCustomThreadedImageLoader.h"

using namespace std;

class ofxActionThreadedImageLoader : public ofxCustomThreadedImageLoader {
public:
    
    ofxActionThreadedImageLoader();
    
    int imageSize;
    int thumbSize;
    
    vector<ofImage> items;
    vector<ofImage> thumbs;
    
    deque<ofImage> thumbs_to_update;
    bool needClean = true;
    
    void clean();
    
protected:
    
    // Check the update queue and update the texture
    //--------------------------------------------------------------
    void update(ofEventArgs & a);
    
    // Reads from the queue and loads new images.
    //--------------------------------------------------------------
    void threadedFunction();
};
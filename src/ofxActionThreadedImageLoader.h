#pragma once

#include "ofxCustomThreadedImageLoader.h"

using namespace std;

class ofxActionThreadedImageLoader : public ofxCustomThreadedImageLoader {
public:
    
    ofxActionThreadedImageLoader();
    
    vector<ofImage> items;
    vector<ofImage> thumbs;

    int imageSize;
    int thumbSize;

    void clean();
    
    void setSize(int size, int thumbSize, bool crop);
    
protected:
    
    bool crop;

    deque<ofImage> thumbs_to_update;
    bool needClean = true;

    // Check the update queue and update the texture
    //--------------------------------------------------------------
    void update(ofEventArgs & a);
    
    // Reads from the queue and loads new images.
    //--------------------------------------------------------------
    void threadedFunction();
};
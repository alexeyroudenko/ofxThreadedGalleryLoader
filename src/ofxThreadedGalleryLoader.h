#pragma once

#include "ofMain.h"
#include "ofxActionThreadedImageLoader.h"

class ofxThreadGalleryLoader {
    
public:
    ofxActionThreadedImageLoader *loader;
    int countToLoad = 0;

    void setup();
    
    void setSize(int size, int thumbSize, bool crop);
    
    void onPhotoLoaded(string &message);
    
    void load(string path);
    
    void stopLoad();
    
    void clean();
};
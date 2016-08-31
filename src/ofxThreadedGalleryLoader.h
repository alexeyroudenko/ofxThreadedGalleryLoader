#pragma once

#include "ofMain.h"
#include "ofxActionThreadedImageLoader.h"

class ofxThreadGalleryLoader {
    
public:
    vector<string> srcPaths;
    ofxActionThreadedImageLoader *loader;
    int countToLoad = 0;

    void setup();
    
    void setSize(int size, int thumbSize, bool crop);
    
    void onPhotoLoaded(string &message);
    
    int load(string path);
    
    void add(string filePath);
    
    void stopLoad();
    
    void clean();
};
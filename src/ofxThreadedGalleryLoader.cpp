#include "ofxThreadedGalleryLoader.h"

void ofxThreadGalleryLoader::setup() {
    loader = new ofxActionThreadedImageLoader();
    ofAddListener(loader->IMAGE_LOADED, this, &ofxThreadGalleryLoader::onPhotoLoaded);
    
    ofImage *image = new ofImage();
    loader->loadFromDisk(*image, "");
}

void ofxThreadGalleryLoader::setSize(int size, int thumbSize, bool crop) {
    loader->setSize(size, thumbSize, crop);
}

void ofxThreadGalleryLoader::add(string filePath) {
    ofLogVerbose("ofxThreadGalleryLoader", "add " + filePath);
    srcPaths.push_back(filePath);

//    if (loader->items.size() == 0) {
//        ofLogVerbose("ofxThreadGalleryLoader", "add empty");
//        ofImage *image = new ofImage();
//        loader->loadFromDisk(*image, "");
//    }
    
    ofImage *image = new ofImage();
    loader->loadFromDisk(*image, filePath);
}

void ofxThreadGalleryLoader::load(string path) {
    ofLogVerbose("ofxThreadGalleryLoader", "load:" + path);
    clean();
    
    // need to clean
    ofImage *image = new ofImage();
    loader->loadFromDisk(*image, "");
    
    
    ofDirectory dir;
    dir.listDir(path);
    countToLoad = dir.size();
    for(int i = 0; i < (int)dir.size(); i++) {
        ofImage *image = new ofImage();
        string fileName = dir.getPath(i);
        srcPaths.push_back(fileName);
        loader->loadFromDisk(*image, fileName);
    }
}

void ofxThreadGalleryLoader::stopLoad() {
    ofLogVerbose("ofxThreadGalleryLoader", "stopLoad");
}

void ofxThreadGalleryLoader::clean() {
    ofLogVerbose("ofxThreadGalleryLoader", "clean");
    countToLoad = 0;
    loader->clean();
    srcPaths.clear();
//    // need to clean
//    ofImage *image = new ofImage();
//    loader->loadFromDisk(*image, "");
}

void ofxThreadGalleryLoader::onPhotoLoaded(string &message) {
    ofLogVerbose("ofxThreadGalleryLoader", "loaded " + message + " " + ofToString(loader->items.size()) +"/" + ofToString(countToLoad));
}
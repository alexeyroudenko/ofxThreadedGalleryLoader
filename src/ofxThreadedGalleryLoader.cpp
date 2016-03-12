#include "ofxThreadedGalleryLoader.h"

void ofxThreadGalleryLoader::setup() {
    loader = new ofxActionThreadedImageLoader();
    ofAddListener(loader->IMAGE_LOADED, this, &ofxThreadGalleryLoader::onPhotoLoaded);
}

void ofxThreadGalleryLoader::onPhotoLoaded(string &message) {
    ofLogVerbose("ofxThreadGalleryLoader", "[ThreadGalleryLoader:onPhotoLoaded] loaded " + message + " " + ofToString(loader->items.size()) +"/" + ofToString(countToLoad));
}

void ofxThreadGalleryLoader::load(string path) {
    ofLogVerbose("[ThreadGalleryLoader:load]", path);
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
        loader->loadFromDisk(*image, fileName);
    }
}

void ofxThreadGalleryLoader::stopLoad() {
    ofLogVerbose("[ThreadGalleryLoader]", "stopLoad");
}

void ofxThreadGalleryLoader::clean() {
    ofLogVerbose("[ThreadGalleryLoader]", "clean");
    countToLoad = 0;
    loader->clean();
}
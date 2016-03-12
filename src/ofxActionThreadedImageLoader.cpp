
#include "ofxActionThreadedImageLoader.h"

#define IMAGE_SIZE 835 // mask radius
#define THUMB_SIZE  150

ofxActionThreadedImageLoader::ofxActionThreadedImageLoader()
{
    nextID = 0;
    setUseSuperClass();
    ofAddListener(ofEvents().update, this, &ofxActionThreadedImageLoader::update);
    lastUpdate = 0;
    
    imageSize = IMAGE_SIZE;
    thumbSize = THUMB_SIZE;
}

void ofxActionThreadedImageLoader::clean() {
    lock();
    
    for (int i = 0; i < items.size(); i++)
        items.at(i).clear();
    items.clear();
    
    for (int i = 0; i < thumbs.size(); i++)
        thumbs.at(i).clear();
    thumbs.clear();
    
    needClean = true;
    
    condition.signal();
    unlock();
}

// Check the update queue and update the texture
//--------------------------------------------------------------
void ofxActionThreadedImageLoader::update(ofEventArgs & a){
    
    // Load 1 image per update so we don't block the gl thread for too long
    //        ofLogVerbose("ofxActionThreadedImageLoader::update");
    lock();
    if (!images_to_update.empty()) {
        
        ofImageLoaderEntry entry = images_to_update.front();
        
        const ofPixels& pix = entry.image->getPixels();
        entry.image->getTexture().allocate(
                                                    pix.getWidth()
                                                    ,pix.getHeight()
                                                    ,ofGetGlInternalFormat(pix)
                                                    );
        
        entry.image->setUseTexture(true);
        entry.image->update();
        
        
        images_to_update.pop_front();
        
        items.push_back(*entry.image);
        
        entry.image->clear();
        //            ofLogVerbose("update image " + entry.filename);
        //            ofNotifyEvent(IMAGE_LOADED, entry.filename, this);
    }
    
    if (!thumbs_to_update.empty()) {
        
        ofImage thumb = thumbs_to_update.front();
        
        const ofPixels& pix = thumb.getPixels();
        thumb.getTexture().allocate(
                                             pix.getWidth()
                                             ,pix.getHeight()
                                             ,ofGetGlInternalFormat(pix)
                                             );
        
        thumb.setUseTexture(true);
        thumb.update();
        
        
        thumbs_to_update.pop_front();
        
        thumbs.push_back(thumb);
        //            ofLogVerbose("update thumb " + ofToString(thumb.width));
    }
    unlock();
}

// Reads from the queue and loads new images.
//--------------------------------------------------------------
void ofxActionThreadedImageLoader::threadedFunction() {
    deque<ofImageLoaderEntry> images_to_load;
    
    while(isThreadRunning()) {
        lock();
        ofLogVerbose("[ofxActionThreadedImageLoader] isThreadRunning start waitng images_to_load_buffer size " + ofToString(images_to_load_buffer.size()));
        if(images_to_load_buffer.empty()) condition.wait(mutex);
        ofLogVerbose("[ofxActionThreadedImageLoader] isThreadRunning end waiting images_to_load_buffer size " + ofToString(images_to_load_buffer.size()));
        images_to_load.insert( images_to_load.end(),
                              images_to_load_buffer.begin(),
                              images_to_load_buffer.end() );
        
        images_to_load_buffer.clear();
        unlock();
        
        
        while(!images_to_load.empty()) {
            if (needClean == true) {
                needClean = false;
                for (int i = 0; i < images_to_load.size(); i++)
                    images_to_load.at(i).image->clear();
                images_to_load.clear();
                break;
            }
            
            ofImageLoaderEntry  &entry = images_to_load.front();
            
            if(!entry.image->load(entry.filename)) {
                ofLogError() << "[ofxActionThreadedImageLoader] error loading image " << entry.filename;
            } else {
                
                entry.image->resize(entry.image->getWidth() * (1.0 * imageSize / entry.image->getHeight()), imageSize);
                entry.image->crop((entry.image->getWidth() - imageSize) / 2, 0, imageSize, imageSize);
                
                ofImage thumb;
                thumb.clone(*entry.image);
                float scale = 0.5 * thumbSize / thumb.getHeight();
                thumb.resize(thumbSize, thumbSize);
                lock();
                thumbs_to_update.push_back(thumb);
                unlock();
            }
            
            lock();
            images_to_update.push_back(entry);
            unlock();
            
            images_to_load.pop_front();
        }
    }
}
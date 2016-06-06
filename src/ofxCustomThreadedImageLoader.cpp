#include "ofxCustomThreadedImageLoader.h"
#include <sstream>
ofxCustomThreadedImageLoader::ofxCustomThreadedImageLoader() 
:ofThread()
{
	nextID = 0;
    ofAddListener(ofEvents().update, this, &ofxCustomThreadedImageLoader::update);
	ofAddListener(ofURLResponseEvent(),this,&ofxCustomThreadedImageLoader::urlResponse);
    
    startThread();
    lastUpdate = 0;
}

ofxCustomThreadedImageLoader::~ofxCustomThreadedImageLoader(){
	condition.signal();
    ofRemoveListener(ofEvents().update, this, &ofxCustomThreadedImageLoader::update);
	ofRemoveListener(ofURLResponseEvent(),this,&ofxCustomThreadedImageLoader::urlResponse);
}

void ofxCustomThreadedImageLoader::setUseSuperClass() {
    ofRemoveListener(ofEvents().update, this, &ofxCustomThreadedImageLoader::update);
	ofRemoveListener(ofURLResponseEvent(),this,&ofxCustomThreadedImageLoader::urlResponse);
}

// Load an image from disk.
//--------------------------------------------------------------
void ofxCustomThreadedImageLoader::loadFromDisk(ofImage& image, string filename, float scale) {
	nextID++;
	ofImageLoaderEntry entry(image, OF_LOAD_FROM_DISK);
	entry.filename = filename;
	entry.id = nextID;
	entry.image->setUseTexture(false);
	entry.name = filename;
    entry.scale = scale;
    
    lock();
    images_to_load_buffer.push_back(entry);
    condition.signal();
    unlock();
}


// Load an url asynchronously from an url.
//--------------------------------------------------------------
void ofxCustomThreadedImageLoader::loadFromURL(ofImage& image, string url) {
	nextID++;
	ofImageLoaderEntry entry(image, OF_LOAD_FROM_URL);
	entry.fileURL = url;
	entry.id = nextID;
	entry.image->setUseTexture(false);	
	entry.name = "image" + ofToString(entry.id);

    lock();
	images_to_load_buffer.push_back(entry);
    condition.signal();
    unlock();
}


// Reads from the queue and loads new images.
//--------------------------------------------------------------
void ofxCustomThreadedImageLoader::threadedFunction() {
    
    deque<ofImageLoaderEntry> images_to_load;

	while( isThreadRunning() ) {
		lock();
		if(images_to_load_buffer.empty()) condition.wait(mutex);
		images_to_load.insert( images_to_load.end(),
							images_to_load_buffer.begin(),
							images_to_load_buffer.end() );

		images_to_load_buffer.clear();
		unlock();
        
        
        while( !images_to_load.empty() ) {
            ofImageLoaderEntry  & entry = images_to_load.front();
            
            if(entry.type == OF_LOAD_FROM_DISK) {
                if(! entry.image->load(entry.filename) )  {
                    ofLogError() << "ofxCustomThreadedImageLoader error loading image " << entry.filename;
                } else {
//                    ofLogVerbose() << "ofxCustomThreadedImageLoader loaded image " << entry.filename << " " << ofToString(entry.image->width);
//                    if (entry.scale != 1.0) {
//                        entry.image->resize(entry.image->width * entry.scale, entry.image->height * entry.scale);
//                    }
                }
                
                lock();
                images_to_update.push_back(entry);
                unlock();
            } else if (entry.type == OF_LOAD_FROM_URL) {
                lock();
                images_async_loading.push_back(entry);
                unlock();
                
                ofLoadURLAsync(entry.fileURL, entry.name);
            }

    		images_to_load.pop_front();
        }
	}
}


// When we receive an url response this method is called; 
// The loaded image is removed from the async_queue and added to the
// update queue. The update queue is used to update the texture.
//--------------------------------------------------------------
void ofxCustomThreadedImageLoader::urlResponse(ofHttpResponse & response) {
	if(response.status == 200) {
		lock();
		
		// Get the loaded url from the async queue and move it into the update queue.
		entry_iterator it = getEntryFromAsyncQueue(response.request.name);
		if(it != images_async_loading.end()) {
			(*it).image->load(response.data);
			images_to_update.push_back(*it);
			images_async_loading.erase(it);
		}
		
		unlock();
	}else{
		// log error.
		ofLogError()<< "Could not get image from url, response status: " << response.status;
		ofRemoveURLRequest(response.request.getId());
		// remove the entry from the queue
		lock();
		entry_iterator it = getEntryFromAsyncQueue(response.request.name);
		if(it != images_async_loading.end()) {
			images_async_loading.erase(it);
		}
		unlock();
	}
}


// Check the update queue and update the texture
//--------------------------------------------------------------
void ofxCustomThreadedImageLoader::update(ofEventArgs & a){
    
    // Load 1 image per update so we don't block the gl thread for too long
    ofLogVerbose("ofxCustomThreadedImageLoader::update");
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
        
        ofNotifyEvent(IMAGE_LOADED, entry.filename, this);
	}
    unlock();
}


// Find an entry in the aysnc queue.
//   * private, no lock protection, is private function
//--------------------------------------------------------------
ofxCustomThreadedImageLoader::entry_iterator ofxCustomThreadedImageLoader::getEntryFromAsyncQueue(string name) {
	entry_iterator it = images_async_loading.begin();
	for(;it != images_async_loading.end();it++) {
		if((*it).name == name) {
			return it;
		}
	}
	return images_async_loading.end();
}

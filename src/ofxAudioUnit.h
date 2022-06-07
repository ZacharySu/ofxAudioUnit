#pragma once

#include "ofxAudioUnitBase.h"    // for base Audio Unit class ofxAudioUnit
#include "ofxAudioUnitDSPNode.h" // for base DSP class ofxAudioUnitDSPNode

// ofxAudioUnit subclasses for specific audio units
#include "ofxAudioUnitFilePlayer.h"
#include "ofxAudioUnitCapture.h"
#include "ofxAudioUnitRawMixer.h"
#include "ofxAudioUnitRender.h"
#include "ofxAudioUnitSampler.h"
#include "lpAudioUnitSamplesOutput.hpp"
#include "ofxAudioUnitOutput.h"
#include "ofxAudioUnitRawInput.h"
#include "TargetConditionals.h"
#if !TARGET_OS_IPHONE
	#include "ofxAudioUnitNetReceive.h"
	#include "ofxAudioUnitNetSend.h"
	#include "ofxAudioUnitSpeechSynth.h"
	#include "ofxAudioUnitRecorder.h"

	// ofxAudioUnitDSPNode subclasses for specific DSP tasks
//	#include "ofxAudioUnitTap.h"
	#include "ofxAudioUnitFftNode.h"
#endif

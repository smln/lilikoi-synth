/**
 * Synth class header
 */

#pragma once

#include <iostream>
#include <stdlib.h>
#include <unistd.h>

#include "PdBase.hpp"
#include "RtAudio.h"

#include "PdObject.h"
#include "externals/Externals.h"

using namespace pd;
using namespace std;

/**
 * Not sure if this is the best way to structure this, but the Synth class will basically handle all the puredata stuff
 * Messages will be passed into Synth about CV data analysis. Synth will then use this information to create sounds in pd.
 */
class Synth
{
    public:

    /**
     * default constructor, just sets the synth up normally
     */
    Synth();
    
    /**
     * Start makin those sounds.
     */
    void playAudio();

    /**
     * our pd engine. Maybe not the best way to do this?
     */
	static PdBase pd;


    private:

    /**
     * The .pd patch for our synthesizer
     */
    Patch patch;

    /**
     * @@@for testing, to print messages from pd
     */
    PdObject pd_receiver;

    // two output channels
	// block size 64, one tick per buffer
    unsigned int numbufferframes = 128;

    /**
     * sample rate
     */
    int srate;

    RtAudio audio;

    /**
     * Just used internally by RtAudio, not for external use
     */
    // int Synth::audioCallback(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void *userData);

};
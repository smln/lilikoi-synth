/**
 * implementation of Synth class
 */

#include "Synth.h"

/**
 * Just used internally by RtAudio, not for external use
 */
int audioCallback(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void *userData){

   // pass audio samples to/from libpd
   int ticks = nBufferFrames / 64;
   Synth::pd.processFloat(ticks, (float *)inputBuffer, (float*)outputBuffer);

   return 0;
}

Synth::Synth() :
                srate(44100),
                numbufferframes(128),
                audio()
{

	// init pd
	//
	// set 4th arg to true for queued message passing using an internal ringbuffer
	//
	// in this test, messages should return immediately when not queued otherwise
	// they should all return at once when pd is processing at the end of this
	// function
	//
	if(!pd.init(0, 2, srate, true)) {
		cerr << "ERROR: Could not init pd" << endl;
		exit(1);
	}

   pd_tilde_setup();

	// add the pd folder to the search path
	pd.addToSearchPath("pd");

   //@@@testing message stuff
   pd.setReceiver(&pd_receiver);

	patch = pd.openPatch("pd/lilikoi.pd", ".");

	if(!patch.isValid()) //if the .pd patch couldn't open...
	{
		cerr << "ERROR: Could not open .pd patch" << endl;
	}
	

   // use the RtAudio API to connect to the default audio device
   if(audio.getDeviceCount()==0){
      std::cout << "There are no available sound devices." << std::endl;
      exit(1);
   }

   RtAudio::StreamParameters parameters;
   parameters.deviceId = audio.getDefaultOutputDevice();;
   parameters.nChannels = 2;

   RtAudio::StreamOptions options;
   options.streamName = "Lilikoi";
   options.flags = RTAUDIO_SCHEDULE_REALTIME;


   if(audio.getCurrentApi() != RtAudio::MACOSX_CORE) {
      options.flags |= RTAUDIO_MINIMIZE_LATENCY; // CoreAudio doesn't seem to like this
   }
   try {
      audio.openStream( &parameters, NULL, RTAUDIO_FLOAT32, srate, &numbufferframes, &audioCallback, NULL, &options );
   }
   catch(RtAudioError& e) {
      std::cerr << e.getMessage() << std::endl;
      exit(1);
   }

   
}

void Synth::playAudio()
{
   // audio processing on
   pd.computeAudio(true);

   try {
   audio.startStream();
   }
   catch(RtAudioError& e) {
      std::cerr << e.getMessage() << std::endl;
      exit(1);
   }

}
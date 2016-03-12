# OHMComm
This project started as an IT-Project at the [faculty for Computer Science of the technical university Georg Simon Ohm Nürnberg](http://www.th-nuernberg.de/seitenbaum/fakultaeten/informatik/page.html) in summer semester 2015 and winter semester 2015/2016 to create a platform-independent voice-over-IP peer-to-peer communication program in C++.

[![Build Status](https://travis-ci.org/doe300/OHMComm.svg)](https://travis-ci.org/doe300/OHMComm)
[![Build Status](https://ci.appveyor.com/api/projects/status/58fv0pln0jv270am?svg=true)](https://ci.appveyor.com/project/doe300/ohmcomm)
[![GitHub license](https://img.shields.io/github/license/doe300/OHMComm.svg)](https://github.com/doe300/OHMComm/blob/master/LICENSE)
[![Release](https://img.shields.io/github/tag/doe300/OHMComm.svg)](https://github.com/doe300/OHMComm/releases/latest)

This program is currently developed under linux (Fedora 22) and Windows (7/8) and tested under Mac OS X.

OHMComm is based upon [RtAudio](http://www.music.mcgill.ca/~gary/rtaudio/) and therefore supports all audio-libraries supported by RtAudio (see [RtAudio API-Notes](http://www.music.mcgill.ca/~gary/rtaudio/apinotes.html)). 
The audio-data are (optionally but highly recommended) encoded with the included [opus-codec](http://www.opus-codec.org/).
As of version 0.8, RtAudio as well as Opus are optional and [PortAudio](http://www.portaudio.com/) is also supported as audio-library.
By additionally supporting [SIP](https://tools.ietf.org/html/rfc3261) as well as the audio-codecs Opus, G.711 A-law and mu-law, 
OHMComm can be used to call or get called by any other SIP-based VoIP application.

The OHMComm framework is highly extensible. Any audio-library or codec can be added by writing a wrapper-class 
and registering it with the correct factory-class.

## Build it

#### Under Linux/Mac OS
You will need a compiler with full C++11 support and cmake in version 2.6 or higher.
Additionally you will require the development-header for the supported audio-library of your choice,
for *ALSA* this would be `libasound-dev` on Debian-based systems and `alsa-lib-devel` on Fedora.

	$ cd <project-directory>
	$ cmake -G "Unix Makefiles" ./CMakeLists.txt
	$ make OHMCommLib	# To build the library
	$ make OHMComm		# To build the executable

This will build the library/executable into `<project-directory>/build/` by default.
## Run it

#### Under Linux/Mac OS/Windows

	$ cd <executable-directory>	
	$ ./OHMComm		# To run in interactive mode
	$ ./OHMComm	-h	# Prints all command-line arguments
	$ ./OHMComm path/to/config-file	# Runs in file-configuration mode

## Supported libraries
#### Audio libraries
- [RtAudio](http://www.music.mcgill.ca/~gary/rtaudio/): A platform-independent API for accessing audio-drivers written in C++
- [PortAudio](http://www.portaudio.com/): Another platform-independent audio API for accessing native libraries, written in C

#### Codecs
- [Opus](http://www.opus-codec.org/): An highly effective audio-codec, primarily used for real-time applications (e.g. VoIP)
- [G.711](https://www.itu.int/rec/T-REC-G.711): The two audio-codecs (A-law and mu-law) used for digital telephony
- [iLBC](https://tools.ietf.org/html/rfc3951): Another low-bandwidth VoIP codec, defined in RFC 3951, now a part of [WebRTC](https://webrtc.org/)
- [GSM]: GSM 06.10 Mobile communication standard

## Features
- Fully standard-conform communication based on [RTP](https://tools.ietf.org/html/rfc3550) including full [RTCP](https://tools.ietf.org/html/rfc3550#section-6) support
- Support for direct calls to and from any VoIP application featuring [SIP](https://tools.ietf.org/html/rfc3261)
- Support for DTX to further decrease required bandwidth

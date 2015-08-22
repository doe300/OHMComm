# OHMComm
IT-Project at the [faculty for Computer Science of the technical university Georg Simon Ohm Nürnberg](http://www.th-nuernberg.de/seitenbaum/fakultaeten/informatik/page.html) in summer semester 2015 and winter semester 2015/2016 to create a platform-independent voice-over-IP peer-to-peer communication program in C++.

[![Build Status](https://travis-ci.org/doe300/OHMComm.svg)](https://travis-ci.org/doe300/OHMComm)
[![GitHub license](https://img.shields.io/github/license/doe300/OHMComm.svg)](https://github.com/doe300/OHMComm/blob/master/LICENSE)
[![Release](https://img.shields.io/github/tag/doe300/OHMComm.svg)](https://github.com/doe300/OHMComm/releases/latest)

This program is currently developed under linux (Fedora 22) and Windows (7/8) and tested under Mac OS X.

OHMComm is based upon [RtAudio](http://www.music.mcgill.ca/~gary/rtaudio/) and therefore supports all audio-libraries supported by RtAudio (see [RtAudio API-Notes](http://www.music.mcgill.ca/~gary/rtaudio/apinotes.html)). The audio-data are (optionally but highly recommended) encoded with the included [opus-codec](http://www.opus-codec.org/).

/* 
 * File:   PayloadType.h
 * Author: daniel
 *
 * Created on March 3, 2016, 11:00 AM
 */

#ifndef PAYLOADTYPE_H
#define	PAYLOADTYPE_H

namespace ohmcomm
{

    /*!
     * List of default mappings for payload-type, as specified in https://www.ietf.org/rfc/rfc3551.txt
     *
     * Also see: https://en.wikipedia.org/wiki/RTP_audio_video_profile
     *
     * Currently only containing audio mappings.
     */
    enum PayloadType : signed char
    {
        //ITU-T G.711 PCMU - https://en.wikipedia.org/wiki/PCMU
        PCMU = 0,
        //GSM Full Rate - https://en.wikipedia.org/wiki/Full_Rate
        GSM = 3,
        //ITU-T G.723.1 - https://en.wikipedia.org/wiki/G.723.1
        G723 = 4,
        //IMA ADPCM 32 kbit/s - https://en.wikipedia.org/wiki/Adaptive_differential_pulse-code_modulation
        DVI4_32 = 5,
        //IMA ADPCM 64 kbit/s
        DVI4_64 = 6,
        //LPC - https://en.wikipedia.org/wiki/Linear_predictive_coding
        LPC = 7,
        //ITU-T G.711 PCMA - https://en.wikipedia.org/wiki/PCMA
        PCMA = 8,
        //ITU-T G.722 - https://en.wikipedia.org/wiki/G.722
        G722 = 9,
        //Linear PCM, 2 channels - https://en.wikipedia.org/wiki/Linear_PCM
        L16_2 = 10,
        //Linear PCM, 1 channel - https://en.wikipedia.org/wiki/Linear_PCM
        L16_1 = 11,
        //Qualcomm Code Excited Linear Prediction
        QCELP = 12,
        //Comfort noise
        CN = 13,
        //MPEG-1 or MPEG-2 audio - https://en.wikipedia.org/wiki/MPEG-1 / https://en.wikipedia.org/wiki/MPEG-2
        MPA = 14,
        //ITU-T G.728
        G728 = 15,
        //IMA ADPCM 44.1 kbit/s
        DVI4_44 = 16,
        //IMA ADPCM 88.2 kbit/s
        DVI4_88 = 17,
        //ITU-T G.729(a) - https://en.wikipedia.org/wiki/G.729
        G729 = 18,
        //OPUS variable bandwidth - https://en.wikipedia.org/wiki/Opus_%28audio_format%29
        //RFC 7587 (RTP Payload Format for Opus, see: https://ietf.org/rfc/rfc7587.txt) defines the opus payload-type as dynamic
        OPUS = 112,
        //AMR-NB - https://en.wikipedia.org/wiki/Adaptive_Multi-Rate_audio_codec
        //RFC 4867 (RTP Payload format for AMR, see: https://tools.ietf.org/html/rfc4867) defines the AMR payload-type as dynamic
        AMR_NB = 113,
        //iLBC - https://tools.ietf.org/html/rfc3951
        //https://en.wikipedia.org/wiki/RTP_audio_video_profile suggests a dynamic payload type
        ILBC = 114,
        //dummy payload-type to accept all types
        ALL = -1

    };
}

#endif	/* PAYLOADTYPE_H */


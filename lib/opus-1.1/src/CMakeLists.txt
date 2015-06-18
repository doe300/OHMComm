# CMakeLists.txt in src

SET(OPUS_SRCS
analysis.c
mlp.c
mlp_data.c
opus.c
opus_compare.c
opus_decoder.c
opus_encoder.c
opus_multistream.c
opus_multistream_encoder.c
opus_multistream_decoder.c
repacketizer.c
)

SET(OPUS_HEADERS
../include/opus.h
../include/opus_multistream.h
analysis.h
mlp.h
opus_private.h
tansig_table.h
)

INCLUDE_DIRECTORIES(
${opusIncludes}
./
../celt
../silk
../silk/fixed
../silk/float
)

ADD_LIBRARY(opus STATIC ${OPUS_HEADERS} ${OPUS_SRCS})

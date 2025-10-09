/*
This implements a data source that decodes Vorbis streams via libvorbis + libvorbisfile

This object can be plugged into any `soloud_ma_data_source_*()` API and can also be used as a custom
decoding backend. See the custom_decoder example.
*/
#ifndef miniaudio_libvorbis_h
#define miniaudio_libvorbis_h

#ifdef __cplusplus
extern "C" {
#endif

#include "../soloud/src/backend/miniaudio/miniaudio.h"

typedef struct
{
    soloud_ma_data_source_base ds;     /* The libvorbis decoder can be used independently as a data source. */
    soloud_ma_read_proc onRead;
    soloud_ma_seek_proc onSeek;
    soloud_ma_tell_proc onTell;
    void* pReadSeekTellUserData;
    soloud_ma_format format;           /* Will be either f32 or s16. */
    /*OggVorbis_File**/ void* vf;   /* Typed as void* so we can avoid a dependency on opusfile in the header section. */
} soloud_ma_libvorbis;

SOLOUD_MA_API soloud_ma_result soloud_ma_libvorbis_init(soloud_ma_read_proc onRead, soloud_ma_seek_proc onSeek, soloud_ma_tell_proc onTell, void* pReadSeekTellUserData, const soloud_ma_decoding_backend_config* pConfig, const soloud_ma_allocation_callbacks* pAllocationCallbacks, soloud_ma_libvorbis* pVorbis);
SOLOUD_MA_API soloud_ma_result soloud_ma_libvorbis_init_file(const char* pFilePath, const soloud_ma_decoding_backend_config* pConfig, const soloud_ma_allocation_callbacks* pAllocationCallbacks, soloud_ma_libvorbis* pVorbis);
SOLOUD_MA_API soloud_ma_result soloud_ma_libvorbis_init_memory(const void* pData, size_t dataSize, const soloud_ma_decoding_backend_config* pConfig, const soloud_ma_allocation_callbacks* pAllocationCallbacks, soloud_ma_libvorbis* pVorbis);
SOLOUD_MA_API void soloud_ma_libvorbis_uninit(soloud_ma_libvorbis* pVorbis, const soloud_ma_allocation_callbacks* pAllocationCallbacks);
SOLOUD_MA_API soloud_ma_result soloud_ma_libvorbis_read_pcm_frames(soloud_ma_libvorbis* pVorbis, void* pFramesOut, soloud_ma_uint64 frameCount, soloud_ma_uint64* pFramesRead);
SOLOUD_MA_API soloud_ma_result soloud_ma_libvorbis_seek_to_pcm_frame(soloud_ma_libvorbis* pVorbis, soloud_ma_uint64 frameIndex);
SOLOUD_MA_API soloud_ma_result soloud_ma_libvorbis_get_data_format(soloud_ma_libvorbis* pVorbis, soloud_ma_format* pFormat, soloud_ma_uint32* pChannels, soloud_ma_uint32* pSampleRate, soloud_ma_channel* pChannelMap, size_t channelMapCap);
SOLOUD_MA_API soloud_ma_result soloud_ma_libvorbis_get_cursor_in_pcm_frames(soloud_ma_libvorbis* pVorbis, soloud_ma_uint64* pCursor);
SOLOUD_MA_API soloud_ma_result soloud_ma_libvorbis_get_length_in_pcm_frames(soloud_ma_libvorbis* pVorbis, soloud_ma_uint64* pLength);

/* Decoding backend vtable. This is what you'll plug into soloud_ma_decoder_config.pBackendVTables. No user data required. */
extern soloud_ma_decoding_backend_vtable* soloud_ma_decoding_backend_libvorbis;

#ifdef __cplusplus
}
#endif
#endif  /* miniaudio_libvorbis_h */
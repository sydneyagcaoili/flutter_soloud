#if !defined(NO_OPUS_OGG_LIBS)

#ifndef miniaudio_libvorbis_c
#define miniaudio_libvorbis_c

#include "miniaudio_libvorbis.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
// For Web include dirs downloaded from git for build
#include "../../xiph/vorbis/include/vorbis/codec.h"
#include "../../xiph/vorbis/include/vorbis/vorbisfile.h"
#else
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>
#endif


#if !defined(SOLOUD_MA_NO_LIBVORBIS)
#ifndef OV_EXCLUDE_STATIC_CALLBACKS
#define OV_EXCLUDE_STATIC_CALLBACKS
#endif
#include <vorbis/vorbisfile.h>
#endif

#include <string.h> /* For memset(). */
#include <assert.h>

static soloud_ma_result soloud_ma_libvorbis_ds_read(soloud_ma_data_source* pDataSource, void* pFramesOut, soloud_ma_uint64 frameCount, soloud_ma_uint64* pFramesRead)
{
    return soloud_ma_libvorbis_read_pcm_frames((soloud_ma_libvorbis*)pDataSource, pFramesOut, frameCount, pFramesRead);
}

static soloud_ma_result soloud_ma_libvorbis_ds_seek(soloud_ma_data_source* pDataSource, soloud_ma_uint64 frameIndex)
{
    return soloud_ma_libvorbis_seek_to_pcm_frame((soloud_ma_libvorbis*)pDataSource, frameIndex);
}

static soloud_ma_result soloud_ma_libvorbis_ds_get_data_format(soloud_ma_data_source* pDataSource, soloud_ma_format* pFormat, soloud_ma_uint32* pChannels, soloud_ma_uint32* pSampleRate, soloud_ma_channel* pChannelMap, size_t channelMapCap)
{
    return soloud_ma_libvorbis_get_data_format((soloud_ma_libvorbis*)pDataSource, pFormat, pChannels, pSampleRate, pChannelMap, channelMapCap);
}

static soloud_ma_result soloud_ma_libvorbis_ds_get_cursor(soloud_ma_data_source* pDataSource, soloud_ma_uint64* pCursor)
{
    return soloud_ma_libvorbis_get_cursor_in_pcm_frames((soloud_ma_libvorbis*)pDataSource, pCursor);
}

static soloud_ma_result soloud_ma_libvorbis_ds_get_length(soloud_ma_data_source* pDataSource, soloud_ma_uint64* pLength)
{
    return soloud_ma_libvorbis_get_length_in_pcm_frames((soloud_ma_libvorbis*)pDataSource, pLength);
}

static soloud_ma_data_source_vtable g_ma_libvorbis_ds_vtable =
{
    soloud_ma_libvorbis_ds_read,
    soloud_ma_libvorbis_ds_seek,
    soloud_ma_libvorbis_ds_get_data_format,
    soloud_ma_libvorbis_ds_get_cursor,
    soloud_ma_libvorbis_ds_get_length,
    NULL,   /* onSetLooping */
    0       /* flags */
};


#if !defined(SOLOUD_MA_NO_LIBVORBIS)
static size_t soloud_ma_libvorbis_vf_callback__read(void* pBufferOut, size_t size, size_t count, void* pUserData)
{
    soloud_ma_libvorbis* pVorbis = (soloud_ma_libvorbis*)pUserData;
    soloud_ma_result result;
    size_t bytesToRead;
    size_t bytesRead;

    /* For consistency with fread(). If `size` of `count` is 0, return 0 immediately without changing anything. */
    if (size == 0 || count == 0) {
        return 0;
    }

    bytesToRead = size * count;
    result = pVorbis->onRead(pVorbis->pReadSeekTellUserData, pBufferOut, bytesToRead, &bytesRead);
    if (result != SOLOUD_MA_SUCCESS) {
        /* Not entirely sure what to return here. What if an error occurs, but some data was read and bytesRead is > 0? */
        return 0;
    }

    return bytesRead / size;
}

static int soloud_ma_libvorbis_vf_callback__seek(void* pUserData, ogg_int64_t offset, int whence)
{
    soloud_ma_libvorbis* pVorbis = (soloud_ma_libvorbis*)pUserData;
    soloud_ma_result result;
    soloud_ma_seek_origin origin;

    if (whence == SEEK_SET) {
        origin = soloud_ma_seek_origin_start;
    } else if (whence == SEEK_END) {
        origin = soloud_ma_seek_origin_end;
    } else {
        origin = soloud_ma_seek_origin_current;
    }

    result = pVorbis->onSeek(pVorbis->pReadSeekTellUserData, offset, origin);
    if (result != SOLOUD_MA_SUCCESS) {
        return -1;
    }

    return 0;
}

static long soloud_ma_libvorbis_vf_callback__tell(void* pUserData)
{
    soloud_ma_libvorbis* pVorbis = (soloud_ma_libvorbis*)pUserData;
    soloud_ma_result result;
    soloud_ma_int64 cursor;
    
    result = pVorbis->onTell(pVorbis->pReadSeekTellUserData, &cursor);
    if (result != SOLOUD_MA_SUCCESS) {
        return -1;
    }

    return (long)cursor;
}
#endif

static soloud_ma_result soloud_ma_libvorbis_init_internal(const soloud_ma_decoding_backend_config* pConfig, const soloud_ma_allocation_callbacks* pAllocationCallbacks, soloud_ma_libvorbis* pVorbis)
{
    soloud_ma_result result;
    soloud_ma_data_source_config dataSourceConfig;

    if (pVorbis == NULL) {
        return SOLOUD_MA_INVALID_ARGS;
    }

    memset(pVorbis, 0, sizeof(*pVorbis));
    pVorbis->format = soloud_ma_format_f32;    /* f32 by default. */

    if (pConfig != NULL && (pConfig->preferredFormat == soloud_ma_format_f32 || pConfig->preferredFormat == soloud_ma_format_s16)) {
        pVorbis->format = pConfig->preferredFormat;
    } else {
        /* Getting here means something other than f32 and s16 was specified. Just leave this unset to use the default format. */
    }

    dataSourceConfig = soloud_ma_data_source_config_init();
    dataSourceConfig.vtable = &g_ma_libvorbis_ds_vtable;

    result = soloud_ma_data_source_init(&dataSourceConfig, &pVorbis->ds);
    if (result != SOLOUD_MA_SUCCESS) {
        return result;  /* Failed to initialize the base data source. */
    }

    #if !defined(SOLOUD_MA_NO_LIBVORBIS)
    {
        pVorbis->vf = (OggVorbis_File*)soloud_ma_malloc(sizeof(OggVorbis_File), pAllocationCallbacks);
        if (pVorbis->vf == NULL) {
            soloud_ma_data_source_uninit(&pVorbis->ds);
            return SOLOUD_MA_OUT_OF_MEMORY;
        }

        return SOLOUD_MA_SUCCESS;
    }
    #else
    {
        /* libvorbis is disabled. */
        (void)pAllocationCallbacks;
        return SOLOUD_MA_NOT_IMPLEMENTED;
    }
    #endif
}

SOLOUD_MA_API soloud_ma_result soloud_ma_libvorbis_init(soloud_ma_read_proc onRead, soloud_ma_seek_proc onSeek, soloud_ma_tell_proc onTell, void* pReadSeekTellUserData, const soloud_ma_decoding_backend_config* pConfig, const soloud_ma_allocation_callbacks* pAllocationCallbacks, soloud_ma_libvorbis* pVorbis)
{
    soloud_ma_result result;

    (void)pAllocationCallbacks; /* Can't seem to find a way to configure memory allocations in libvorbis. */
    
    result = soloud_ma_libvorbis_init_internal(pConfig, pAllocationCallbacks, pVorbis);
    if (result != SOLOUD_MA_SUCCESS) {
        return result;
    }

    if (onRead == NULL || onSeek == NULL) {
        return SOLOUD_MA_INVALID_ARGS; /* onRead and onSeek are mandatory. */
    }

    pVorbis->onRead = onRead;
    pVorbis->onSeek = onSeek;
    pVorbis->onTell = onTell;
    pVorbis->pReadSeekTellUserData = pReadSeekTellUserData;

    #if !defined(SOLOUD_MA_NO_LIBVORBIS)
    {
        int libvorbisResult;
        ov_callbacks libvorbisCallbacks;

        /* We can now initialize the vorbis decoder. This must be done after we've set up the callbacks. */
        libvorbisCallbacks.read_func  = soloud_ma_libvorbis_vf_callback__read;
        libvorbisCallbacks.seek_func  = soloud_ma_libvorbis_vf_callback__seek;
        libvorbisCallbacks.close_func = NULL;
        libvorbisCallbacks.tell_func  = soloud_ma_libvorbis_vf_callback__tell;

        libvorbisResult = ov_open_callbacks(pVorbis, (OggVorbis_File*)pVorbis->vf, NULL, 0, libvorbisCallbacks);
        if (libvorbisResult < 0) {
            return SOLOUD_MA_INVALID_FILE;
        }

        return SOLOUD_MA_SUCCESS;
    }
    #else
    {
        /* libvorbis is disabled. */
        return SOLOUD_MA_NOT_IMPLEMENTED;
    }
    #endif
}

SOLOUD_MA_API soloud_ma_result soloud_ma_libvorbis_init_file(const char* pFilePath, const soloud_ma_decoding_backend_config* pConfig, const soloud_ma_allocation_callbacks* pAllocationCallbacks, soloud_ma_libvorbis* pVorbis)
{
    soloud_ma_result result;

    (void)pAllocationCallbacks; /* Can't seem to find a way to configure memory allocations in libvorbis. */

    result = soloud_ma_libvorbis_init_internal(pConfig, pAllocationCallbacks, pVorbis);
    if (result != SOLOUD_MA_SUCCESS) {
        return result;
    }

    #if !defined(SOLOUD_MA_NO_LIBVORBIS)
    {
        int libvorbisResult;

        libvorbisResult = ov_fopen(pFilePath, (OggVorbis_File*)pVorbis->vf);
        if (libvorbisResult < 0) {
            return SOLOUD_MA_INVALID_FILE;
        }

        return SOLOUD_MA_SUCCESS;
    }
    #else
    {
        /* libvorbis is disabled. */
        (void)pFilePath;
        return SOLOUD_MA_NOT_IMPLEMENTED;
    }
    #endif
}

/* Add these structures and callbacks before soloud_ma_libvorbis_init */
typedef struct {
    const unsigned char* data;
    size_t dataSize;
    size_t currentPos;
} soloud_ma_memory_stream;

static size_t soloud_ma_memory_stream_read(void* pUserData, void* pBufferOut, size_t bytesToRead, size_t* pBytesRead)
{
    soloud_ma_memory_stream* stream = (soloud_ma_memory_stream*)pUserData;
    size_t remainingSize = stream->dataSize - stream->currentPos;
    size_t bytesToReadAdjusted = (bytesToRead < remainingSize) ? bytesToRead : remainingSize;

    if (bytesToReadAdjusted > 0) {
        memcpy(pBufferOut, stream->data + stream->currentPos, bytesToReadAdjusted);
        stream->currentPos += bytesToReadAdjusted;
    }

    if (pBytesRead != NULL) {
        *pBytesRead = bytesToReadAdjusted;
    }

    return bytesToReadAdjusted;
}

static soloud_ma_result soloud_ma_memory_stream_seek(void* pUserData, soloud_ma_int64 offset, soloud_ma_seek_origin origin)
{
    soloud_ma_memory_stream* stream = (soloud_ma_memory_stream*)pUserData;
    soloud_ma_int64 newPos = stream->currentPos;

    switch (origin) {
        case soloud_ma_seek_origin_start:
            newPos = offset;
            break;
        case soloud_ma_seek_origin_current:
            newPos = stream->currentPos + offset;
            break;
        case soloud_ma_seek_origin_end:
            newPos = stream->dataSize + offset;
            break;
        default:
            return SOLOUD_MA_INVALID_ARGS;
    }

    if (newPos < 0 || (size_t)newPos > stream->dataSize) {
        return SOLOUD_MA_INVALID_ARGS;
    }

    stream->currentPos = (size_t)newPos;
    return SOLOUD_MA_SUCCESS;
}

static soloud_ma_result soloud_ma_memory_stream_tell(void* pUserData, soloud_ma_int64* pCursor)
{
    soloud_ma_memory_stream* stream = (soloud_ma_memory_stream*)pUserData;
    if (pCursor == NULL) {
        return SOLOUD_MA_INVALID_ARGS;
    }

    *pCursor = stream->currentPos;
    return SOLOUD_MA_SUCCESS;
}

SOLOUD_MA_API soloud_ma_result soloud_ma_libvorbis_init_memory(const void* pData, size_t dataSize, const soloud_ma_decoding_backend_config* pConfig, const soloud_ma_allocation_callbacks* pAllocationCallbacks, soloud_ma_libvorbis* pVorbis)
{
    soloud_ma_memory_stream* pStream;
    soloud_ma_result result;

    if (pVorbis == NULL || pData == NULL || dataSize == 0) {
        return SOLOUD_MA_INVALID_ARGS;
    }

    /* Initialize the base object */
    result = soloud_ma_libvorbis_init_internal(pConfig, pAllocationCallbacks, pVorbis);
    if (result != SOLOUD_MA_SUCCESS) {
        return result;
    }

    /* Create and initialize the memory stream */
    pStream = (soloud_ma_memory_stream*)soloud_ma_malloc(sizeof(soloud_ma_memory_stream), pAllocationCallbacks);
    if (pStream == NULL) {
        soloud_ma_libvorbis_uninit(pVorbis, pAllocationCallbacks);
        return SOLOUD_MA_OUT_OF_MEMORY;
    }

    pStream->data = (const unsigned char*)pData;
    pStream->dataSize = dataSize;
    pStream->currentPos = 0;

    /* Set up the callbacks */
    pVorbis->onRead = (soloud_ma_read_proc)soloud_ma_memory_stream_read;
    pVorbis->onSeek = (soloud_ma_seek_proc)soloud_ma_memory_stream_seek;
    pVorbis->onTell = (soloud_ma_tell_proc)soloud_ma_memory_stream_tell;
    pVorbis->pReadSeekTellUserData = pStream;

    #if !defined(SOLOUD_MA_NO_LIBVORBIS)
    {
        int libvorbisResult;
        ov_callbacks libvorbisCallbacks;

        libvorbisCallbacks.read_func  = soloud_ma_libvorbis_vf_callback__read;
        libvorbisCallbacks.seek_func  = soloud_ma_libvorbis_vf_callback__seek;
        libvorbisCallbacks.close_func = NULL;
        libvorbisCallbacks.tell_func  = soloud_ma_libvorbis_vf_callback__tell;

        libvorbisResult = ov_open_callbacks(pVorbis, (OggVorbis_File*)pVorbis->vf, NULL, 0, libvorbisCallbacks);
        if (libvorbisResult < 0) {
            soloud_ma_free(pStream, pAllocationCallbacks);
            soloud_ma_libvorbis_uninit(pVorbis, pAllocationCallbacks);
            return SOLOUD_MA_INVALID_FILE;
        }

        return SOLOUD_MA_SUCCESS;
    }
    #else
    {
        soloud_ma_free(pStream, pAllocationCallbacks);
        soloud_ma_libvorbis_uninit(pVorbis, pAllocationCallbacks);
        return SOLOUD_MA_NOT_IMPLEMENTED;
    }
    #endif
}

SOLOUD_MA_API void soloud_ma_libvorbis_uninit(soloud_ma_libvorbis* pVorbis, const soloud_ma_allocation_callbacks* pAllocationCallbacks)
{
    if (pVorbis == NULL) {
        return;
    }

    (void)pAllocationCallbacks;

    #if !defined(SOLOUD_MA_NO_LIBVORBIS)
    {
        ov_clear((OggVorbis_File*)pVorbis->vf);
    }
    #else
    {
        /* libvorbis is disabled. Should never hit this since initialization would have failed. */
        assert(SOLOUD_MA_FALSE);
    }
    #endif

    soloud_ma_data_source_uninit(&pVorbis->ds);
    soloud_ma_free(pVorbis->vf, pAllocationCallbacks);

    /* Don't free pReadSeekTellUserData here - it will be freed by the decoding backend */
    pVorbis->pReadSeekTellUserData = NULL;
}

SOLOUD_MA_API soloud_ma_result soloud_ma_libvorbis_read_pcm_frames(soloud_ma_libvorbis* pVorbis, void* pFramesOut, soloud_ma_uint64 frameCount, soloud_ma_uint64* pFramesRead)
{
    if (pFramesRead != NULL) {
        *pFramesRead = 0;
    }

    if (frameCount == 0) {
        return SOLOUD_MA_INVALID_ARGS;
    }

    if (pVorbis == NULL) {
        return SOLOUD_MA_INVALID_ARGS;
    }

    #if !defined(SOLOUD_MA_NO_LIBVORBIS)
    {
        /* We always use floating point format. */
        soloud_ma_result result = SOLOUD_MA_SUCCESS;  /* Must be initialized to SOLOUD_MA_SUCCESS. */
        soloud_ma_uint64 totalFramesRead;
        soloud_ma_format format;
        soloud_ma_uint32 channels;

        soloud_ma_libvorbis_get_data_format(pVorbis, &format, &channels, NULL, NULL, 0);

        totalFramesRead = 0;
        while (totalFramesRead < frameCount) {
            long libvorbisResult;
            soloud_ma_uint64 framesToRead;
            soloud_ma_uint64 framesRemaining;

            framesRemaining = (frameCount - totalFramesRead);
            framesToRead = 1024;
            if (framesToRead > framesRemaining) {
                framesToRead = framesRemaining;
            }

            if (format == soloud_ma_format_f32) {
                float** ppFramesF32;

                libvorbisResult = ov_read_float((OggVorbis_File*)pVorbis->vf, &ppFramesF32, (int)framesToRead, NULL);
                if (libvorbisResult < 0) {
                    result = SOLOUD_MA_ERROR;  /* Error while decoding. */
                    break;
                } else {
                    /* Frames need to be interleaved. */
                    soloud_ma_interleave_pcm_frames(format, channels, libvorbisResult, (const void**)ppFramesF32, soloud_ma_offset_pcm_frames_ptr(pFramesOut, totalFramesRead, format, channels));
                    totalFramesRead += libvorbisResult;

                    if (libvorbisResult == 0) {
                        result = SOLOUD_MA_AT_END;
                        break;
                    }
                }
            } else {
                libvorbisResult = ov_read((OggVorbis_File*)pVorbis->vf, (char*)soloud_ma_offset_pcm_frames_ptr(pFramesOut, totalFramesRead, format, channels), (int)(framesToRead * soloud_ma_get_bytes_per_frame(format, channels)), 0, 2, 1, NULL);
                if (libvorbisResult < 0) {
                    result = SOLOUD_MA_ERROR;  /* Error while decoding. */
                    break;
                } else {
                    /* Conveniently, there's no need to interleaving when using ov_read(). I'm not sure why ov_read_float() is different in that regard... */
                    totalFramesRead += libvorbisResult / soloud_ma_get_bytes_per_frame(format, channels);

                    if (libvorbisResult == 0) {
                        result = SOLOUD_MA_AT_END;
                        break;
                    }
                }
            }
        }

        if (pFramesRead != NULL) {
            *pFramesRead = totalFramesRead;
        }

        if (result == SOLOUD_MA_SUCCESS && totalFramesRead == 0) {
            result = SOLOUD_MA_AT_END;
        }

        return result;
    }
    #else
    {
        /* libvorbis is disabled. Should never hit this since initialization would have failed. */
        assert(SOLOUD_MA_FALSE);

        (void)pFramesOut;
        (void)frameCount;
        (void)pFramesRead;

        return SOLOUD_MA_NOT_IMPLEMENTED;
    }
    #endif
}

SOLOUD_MA_API soloud_ma_result soloud_ma_libvorbis_seek_to_pcm_frame(soloud_ma_libvorbis* pVorbis, soloud_ma_uint64 frameIndex)
{
    if (pVorbis == NULL) {
        return SOLOUD_MA_INVALID_ARGS;
    }

    #if !defined(SOLOUD_MA_NO_LIBVORBIS)
    {
        int libvorbisResult = ov_pcm_seek((OggVorbis_File*)pVorbis->vf, (ogg_int64_t)frameIndex);
        if (libvorbisResult != 0) {
            if (libvorbisResult == OV_ENOSEEK) {
                return SOLOUD_MA_INVALID_OPERATION;    /* Not seekable. */
            } else if (libvorbisResult == OV_EINVAL) {
                return SOLOUD_MA_INVALID_ARGS;
            } else {
                return SOLOUD_MA_ERROR;
            }
        }

        return SOLOUD_MA_SUCCESS;
    }
    #else
    {
        /* libvorbis is disabled. Should never hit this since initialization would have failed. */
        assert(SOLOUD_MA_FALSE);

        (void)frameIndex;

        return SOLOUD_MA_NOT_IMPLEMENTED;
    }
    #endif
}

SOLOUD_MA_API soloud_ma_result soloud_ma_libvorbis_get_data_format(soloud_ma_libvorbis* pVorbis, soloud_ma_format* pFormat, soloud_ma_uint32* pChannels, soloud_ma_uint32* pSampleRate, soloud_ma_channel* pChannelMap, size_t channelMapCap)
{
    /* Defaults for safety. */
    if (pFormat != NULL) {
        *pFormat = soloud_ma_format_unknown;
    }
    if (pChannels != NULL) {
        *pChannels = 0;
    }
    if (pSampleRate != NULL) {
        *pSampleRate = 0;
    }
    if (pChannelMap != NULL) {
        memset(pChannelMap, 0, sizeof(*pChannelMap) * channelMapCap);
    }

    if (pVorbis == NULL) {
        return SOLOUD_MA_INVALID_OPERATION;
    }

    if (pFormat != NULL) {
        *pFormat = pVorbis->format;
    }

    #if !defined(SOLOUD_MA_NO_LIBVORBIS)
    {
        vorbis_info* pInfo = ov_info((OggVorbis_File*)pVorbis->vf, 0);
        if (pInfo == NULL) {
            return SOLOUD_MA_INVALID_OPERATION;
        }

        if (pChannels != NULL) {
            *pChannels = pInfo->channels;
        }

        if (pSampleRate != NULL) {
            *pSampleRate = pInfo->rate;
        }

        if (pChannelMap != NULL) {
            soloud_ma_channel_map_init_standard(soloud_ma_standard_channel_map_vorbis, pChannelMap, channelMapCap, pInfo->channels);
        }

        return SOLOUD_MA_SUCCESS;
    }
    #else
    {
        /* libvorbis is disabled. Should never hit this since initialization would have failed. */
        assert(SOLOUD_MA_FALSE);
        return SOLOUD_MA_NOT_IMPLEMENTED;
    }
    #endif
}

SOLOUD_MA_API soloud_ma_result soloud_ma_libvorbis_get_cursor_in_pcm_frames(soloud_ma_libvorbis* pVorbis, soloud_ma_uint64* pCursor)
{
    if (pCursor == NULL) {
        return SOLOUD_MA_INVALID_ARGS;
    }

    *pCursor = 0;   /* Safety. */

    if (pVorbis == NULL) {
        return SOLOUD_MA_INVALID_ARGS;
    }

    #if !defined(SOLOUD_MA_NO_LIBVORBIS)
    {
        ogg_int64_t offset = ov_pcm_tell((OggVorbis_File*)pVorbis->vf);
        if (offset < 0) {
            return SOLOUD_MA_INVALID_FILE;
        }

        *pCursor = (soloud_ma_uint64)offset;

        return SOLOUD_MA_SUCCESS;
    }
    #else
    {
        /* libvorbis is disabled. Should never hit this since initialization would have failed. */
        assert(SOLOUD_MA_FALSE);
        return SOLOUD_MA_NOT_IMPLEMENTED;
    }
    #endif
}

SOLOUD_MA_API soloud_ma_result soloud_ma_libvorbis_get_length_in_pcm_frames(soloud_ma_libvorbis* pVorbis, soloud_ma_uint64* pLength)
{
    if (pLength == NULL) {
        return SOLOUD_MA_INVALID_ARGS;
    }

    *pLength = 0;   /* Safety. */

    if (pVorbis == NULL) {
        return SOLOUD_MA_INVALID_ARGS;
    }

    #if !defined(SOLOUD_MA_NO_LIBVORBIS)
    {
        ogg_int64_t totalSamples = ov_pcm_total((OggVorbis_File*)pVorbis->vf, -1);
        if (totalSamples < 0) {
            return SOLOUD_MA_INVALID_FILE;
        }

        *pLength = (soloud_ma_uint64)totalSamples;
        return SOLOUD_MA_SUCCESS;
    }
    #else
    {
        /* libvorbis is disabled. Should never hit this since initialization would have failed. */
        assert(SOLOUD_MA_FALSE);
        return SOLOUD_MA_NOT_IMPLEMENTED;
    }
    #endif
}


/*
The code below defines the vtable that you'll plug into your `soloud_ma_decoder_config` object.
*/
#if !defined(SOLOUD_MA_NO_LIBVORBIS)
static soloud_ma_result soloud_ma_decoding_backend_init__libvorbis(void* pUserData, soloud_ma_read_proc onRead, soloud_ma_seek_proc onSeek, soloud_ma_tell_proc onTell, void* pReadSeekTellUserData, const soloud_ma_decoding_backend_config* pConfig, const soloud_ma_allocation_callbacks* pAllocationCallbacks, soloud_ma_data_source** ppBackend)
{
    soloud_ma_result result;
    soloud_ma_libvorbis* pVorbis;

    (void)pUserData;

    pVorbis = (soloud_ma_libvorbis*)soloud_ma_malloc(sizeof(*pVorbis), pAllocationCallbacks);
    if (pVorbis == NULL) {
        return SOLOUD_MA_OUT_OF_MEMORY;
    }

    result = soloud_ma_libvorbis_init(onRead, onSeek, onTell, pReadSeekTellUserData, pConfig, pAllocationCallbacks, pVorbis);
    if (result != SOLOUD_MA_SUCCESS) {
        soloud_ma_free(pVorbis, pAllocationCallbacks);
        return result;
    }

    *ppBackend = pVorbis;

    return SOLOUD_MA_SUCCESS;
}

static soloud_ma_result soloud_ma_decoding_backend_init_file__libvorbis(void* pUserData, const char* pFilePath, const soloud_ma_decoding_backend_config* pConfig, const soloud_ma_allocation_callbacks* pAllocationCallbacks, soloud_ma_data_source** ppBackend)
{
    soloud_ma_result result;
    soloud_ma_libvorbis* pVorbis;

    (void)pUserData;

    pVorbis = (soloud_ma_libvorbis*)soloud_ma_malloc(sizeof(*pVorbis), pAllocationCallbacks);
    if (pVorbis == NULL) {
        return SOLOUD_MA_OUT_OF_MEMORY;
    }

    result = soloud_ma_libvorbis_init_file(pFilePath, pConfig, pAllocationCallbacks, pVorbis);
    if (result != SOLOUD_MA_SUCCESS) {
        soloud_ma_free(pVorbis, pAllocationCallbacks);
        return result;
    }

    *ppBackend = pVorbis;

    return SOLOUD_MA_SUCCESS;
}

/* Add to the decoding backend vtable section */
#if !defined(SOLOUD_MA_NO_LIBVORBIS)
static soloud_ma_result soloud_ma_decoding_backend_init_memory__libvorbis(void* pUserData, const void* pData, size_t dataSize, const soloud_ma_decoding_backend_config* pConfig, const soloud_ma_allocation_callbacks* pAllocationCallbacks, soloud_ma_data_source** ppBackend)
{
    soloud_ma_result result;
    soloud_ma_libvorbis* pVorbis;

    (void)pUserData;

    pVorbis = (soloud_ma_libvorbis*)soloud_ma_malloc(sizeof(*pVorbis), pAllocationCallbacks);
    if (pVorbis == NULL) {
        return SOLOUD_MA_OUT_OF_MEMORY;
    }

    result = soloud_ma_libvorbis_init_memory(pData, dataSize, pConfig, pAllocationCallbacks, pVorbis);
    if (result != SOLOUD_MA_SUCCESS) {
        soloud_ma_free(pVorbis, pAllocationCallbacks);
        return result;
    }

    *ppBackend = pVorbis;
    return SOLOUD_MA_SUCCESS;
}
#endif

static void soloud_ma_decoding_backend_uninit__libvorbis(void* pUserData, soloud_ma_data_source* pBackend, const soloud_ma_allocation_callbacks* pAllocationCallbacks)
{
    soloud_ma_libvorbis* pVorbis = (soloud_ma_libvorbis*)pBackend;

    (void)pUserData;

    /* The memory stream will be freed in soloud_ma_libvorbis_uninit */
    soloud_ma_libvorbis_uninit(pVorbis, pAllocationCallbacks);
    soloud_ma_free(pVorbis, pAllocationCallbacks);
}


static soloud_ma_decoding_backend_vtable soloud_ma_gDecodingBackendVTable_libvorbis =
{
    soloud_ma_decoding_backend_init__libvorbis,
    soloud_ma_decoding_backend_init_file__libvorbis,
    NULL, /* onInitFileW() */
    soloud_ma_decoding_backend_init_memory__libvorbis,
    soloud_ma_decoding_backend_uninit__libvorbis
};
soloud_ma_decoding_backend_vtable* soloud_ma_decoding_backend_libvorbis = &soloud_ma_gDecodingBackendVTable_libvorbis;
#else
soloud_ma_decoding_backend_vtable* soloud_ma_decoding_backend_libvorbis = NULL;
#endif

#endif /* miniaudio_libvorbis_c */

#endif /* !defined(SOLOUD_MA_NO_LIBVORBIS) */
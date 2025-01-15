#include "game/data.h"
#include "game/memory.h"
#include "dolphin/os.h"
#include "zlib.h"

typedef struct Decode_s
{
    u8 *src;
    u8 *dst;
    u32 size;
} DECODE;

static u8 TextBuffer[1024];

static int HuDecodeZlib(DECODE *decode);

static void HuDecodeNone(DECODE *decode)
{
    while(decode->size) {
        *decode->dst++ = *decode->src++;
        decode->size--;
    }
}

static void HuDecodeLz(DECODE *decode)
{
    u16 flag, pos;
    s32 i, j, copyLen;
    flag = 0;
    pos = 958;
    
    for(i=0; i<1024; i++) {
        TextBuffer[i] = 0;
    }
    while(decode->size) {
        flag >>= 1;
        if(!(flag & 0x100)) {
            flag = (*decode->src++)|0xFF00;
        }
        if(flag & 0x1) {
            TextBuffer[pos++] = *decode->dst++ = *decode->src++;
            pos = pos & 0x3FF;
            decode->size--;
        } else {
            i = *decode->src++;
            copyLen = *decode->src++;
            i |= ((copyLen & ~0x3F) << 2);
            copyLen = (copyLen & 0x3F)+3;
            for(j=0; j<copyLen; j++) {
                TextBuffer[pos++] = *decode->dst++ = TextBuffer[(i+j) & 0x3FF];
                pos &= 0x3FF;
            }
            decode->size -= j;
        }
    }
}

#define SlideReadUint(decode, dest) \
    (*(dest)) = (*(decode)->src++) << 24; \
    (*(dest)) += (*(decode)->src++) << 16; \
    (*(dest)) += (*(decode)->src++) << 8; \
    (*(dest)) += (*(decode)->src++) << 0
    
static void HuDecodeSlide(DECODE *decode)
{
    u8 *dstPOrig;
    u32 flagLen, flag;
    u32 size;
    SlideReadUint(decode, &size);
    flagLen = 0;
    flag = 0;
    dstPOrig = decode->dst;
    while(decode->size) {
        if(flagLen == 0) {
            SlideReadUint(decode, &flag);
            flagLen = 32;
        }
        if(flag >> 31) {
            *decode->dst++ = (s32)*decode->src++;
            decode->size--;
        } else {
            u8 *src;
            u32 dist, len;
            dist = *decode->src++ << 8;
            dist = dist+(*decode->src++);
            len = (dist >> 12) & 0xF;
            dist &= 0xFFF;
            src = decode->dst-dist;
            (void)dist;
            if(len == 0) {
                len = (*decode->src++)+18;
            } else {
                len += 2;
            }
            decode->size -= len;
            while(len) {
                if(src-1 < dstPOrig) {
                    *decode->dst++ = 0;
                } else {
                    *decode->dst++ = src[-1];
                }
                len--;
                src++;
            }
        }
        
        flag <<= 1;
        flagLen--;
    }
}

static void HuDecodeFslide(DECODE *decode)
{
    u32 flagLen, flag;
    u32 size;
    SlideReadUint(decode, &size);
    flagLen = 0;
    flag = 0;
    while(decode->size) {
        if(flagLen == 0) {
            SlideReadUint(decode, &flag);
            flagLen = 32;
        }
        if(flag >> 31) {
            *decode->dst++ = (s32)*decode->src++;
            decode->size--;
        } else {
            u8 *src;
            u32 dist, len;
            dist = *decode->src++ << 8;
            dist += *decode->src++;
            len = (dist >> 12) & 0xF;
            dist &= 0xFFF;
            src = decode->dst-dist;
            (void)dist;
            if(len == 0) {
                len = (*decode->src++)+18;
            } else {
                len += 2;
            }
            decode->size -= len;
            while(len) {
                *decode->dst++ = src[-1];
                len--;
                src++;
            }
        }
        
        flag <<= 1;
        flagLen--;
    }
}

static void HuDecodeRle(DECODE *decode)
{
    s32 i;
    while(decode->size) {
        s32 size = *decode->src++;
        if(size < 128) {
            s32 fill = *decode->src++;
            for(i=0; i<size; i++) {
                *decode->dst++ = fill;
            }
        } else {
            size -= 128;
            for(i=0; i<size; i++) {
                *decode->dst++ = *decode->src++;
            }
        }
        decode->size -= size;
    }
}

void HuDecodeData(void *src, void *dst, u32 size, s32 decodeType)
{
    DECODE decode;
    DECODE *decodeP = &decode;
    decodeP->src = src;
    decodeP->dst = dst;
    decodeP->size = size;
    switch(decodeType) {
        case HU_DECODE_TYPE_NONE:
            HuDecodeNone(decodeP);
            break;
            
        case HU_DECODE_TYPE_LZ:
            HuDecodeLz(decodeP);
            break;
            
        case HU_DECODE_TYPE_SLIDE:
            HuDecodeSlide(decodeP);
            break;
            
        case HU_DECODE_TYPE_FSLIDE_ALT:
            HuDecodeFslide(decodeP);
            break;
            
        case HU_DECODE_TYPE_FSLIDE:
            HuDecodeFslide(decodeP);
            break;
            
        case HU_DECODE_TYPE_RLE:
            HuDecodeRle(decodeP);
            break;
            
        case HU_DECODE_TYPE_ZLIB:
            HuDecodeZlib(decodeP);
            break;
            
        default:
            OSReport("decode tyep unknown.(%x)\n", decodeType);
            break;
    }
    DCFlushRange(dst, size);
}

static void *ZlibCalloc(voidpf opaque, uInt items, uInt size)
{
    s32 allocSize = items*size;
    void *buf = HuMemDirectMallocNum(HEAP_MODEL, allocSize, HU_MEMNUM_OVL);
    memset(buf, 0, allocSize);
    return buf;
}

static void ZlibFree(voidpf opaque, voidpf address)
{
    HuMemDirectFree(address);
}

static int HuDecodeZlib(DECODE *decode)
{
    z_stream stream;
    u32 dummy;
    int result;
    u32 *src = (u32 *)decode->src;
    stream.avail_in = src[1];
    src += 2;
    stream.next_in = (Bytef *)src;
    stream.next_out = decode->dst;
    stream.avail_out = (uInt)(&dummy);
    stream.zalloc = ZlibCalloc;
    stream.zfree = ZlibFree;
    result = inflateInit(&stream);
    if(result) {
        return result;
    }
    result = inflate(&stream, Z_FINISH);
    if(result != Z_STREAM_END) {
        inflateEnd(&stream);
        return (result == Z_OK) ? Z_BUF_ERROR : result;
    } else {
        result = inflateEnd(&stream);
        return result;
    }
}
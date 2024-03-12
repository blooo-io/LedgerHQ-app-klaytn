#include "ethUtils.h"

#include <stdbool.h>
#include <stdlib.h>

#include "cx.h"
#include "ethUstream.h"
#include "globals.h"
#include "os.h"

bool rlpCanDecode(uint8_t *buffer, uint32_t bufferLength, bool *valid) {
    if (*buffer <= 0x7f) {
    } else if (*buffer <= 0xb7) {
    } else if (*buffer <= 0xbf) {
        if (bufferLength < (1 + (*buffer - 0xb7))) {
            return false;
        }
        if (*buffer > 0xbb) {
            *valid = false;  // arbitrary 32 bits length limitation
            return true;
        }
    } else if (*buffer <= 0xf7) {
    } else {
        if (bufferLength < (1 + (*buffer - 0xf7))) {
            return false;
        }
        if (*buffer > 0xfb) {
            *valid = false;  // arbitrary 32 bits length limitation
            return true;
        }
    }
    *valid = true;
    return true;
}

bool rlpDecodeLength(uint8_t *buffer, uint32_t *fieldLength, uint32_t *offset, bool *list) {
    if (*buffer <= 0x7f) {
        *offset = 0;
        *fieldLength = 1;
        *list = false;
    } else if (*buffer <= 0xb7) {
        *offset = 1;
        *fieldLength = *buffer - 0x80;
        *list = false;
    } else if (*buffer <= 0xbf) {
        *offset = 1 + (*buffer - 0xb7);
        *list = false;
        switch (*buffer) {
            case 0xb8:
                *fieldLength = *(buffer + 1);
                break;
            case 0xb9:
                *fieldLength = (*(buffer + 1) << 8) + *(buffer + 2);
                break;
            case 0xba:
                *fieldLength = (*(buffer + 1) << 16) + (*(buffer + 2) << 8) + *(buffer + 3);
                break;
            case 0xbb:
                *fieldLength = (*(buffer + 1) << 24) + (*(buffer + 2) << 16) +
                               (*(buffer + 3) << 8) + *(buffer + 4);
                break;
            default:
                return false;  // arbitrary 32 bits length limitation
        }
    } else if (*buffer <= 0xf7) {
        *offset = 1;
        *fieldLength = *buffer - 0xc0;
        *list = true;
    } else {
        *offset = 1 + (*buffer - 0xf7);
        *list = true;
        switch (*buffer) {
            case 0xf8:
                *fieldLength = *(buffer + 1);
                break;
            case 0xf9:
                *fieldLength = (*(buffer + 1) << 8) + *(buffer + 2);
                break;
            case 0xfa:
                *fieldLength = (*(buffer + 1) << 16) + (*(buffer + 2) << 8) + *(buffer + 3);
                break;
            case 0xfb:
                *fieldLength = (*(buffer + 1) << 24) + (*(buffer + 2) << 16) +
                               (*(buffer + 3) << 8) + *(buffer + 4);
                break;
            default:
                return false;  // arbitrary 32 bits length limitation
        }
    }

    return true;
}

void u64_to_string(uint64_t src, char *dst, uint8_t dst_size) {
    // Copy the numbers in ASCII format.
    uint8_t i = 0;
    do {
        // Checking `i + 1` to make sure we have enough space for '\0'.
        if (i + 1 >= dst_size) {
            THROW(0x6502);
        }
        dst[i] = src % 10 + '0';
        src /= 10;
        i++;
    } while (src);

    // Null terminate string
    dst[i] = '\0';

    // Revert the string
    i--;
    uint8_t j = 0;
    while (j < i) {
        char tmp = dst[i];
        dst[i] = dst[j];
        dst[j] = tmp;
        i--;
        j++;
    }
}

void getEthAddressStringFromBinary(uint8_t *address,
                                   char *out,
                                   cx_sha3_t *sha3Context) {
    // save some precious stack space
    union locals_union {
        uint8_t hashChecksum[HASH_LENGTH];
        uint8_t tmp[51];
    } locals_union;

    uint8_t i;
    uint32_t offset = 0;
    
    for (i = 0; i < 20; i++) {
        uint8_t digit = address[i];
        locals_union.tmp[offset + 2 * i] = HEXDIGITS[(digit >> 4) & 0x0f];
        locals_union.tmp[offset + 2 * i + 1] = HEXDIGITS[digit & 0x0f];
    }

    CX_THROW(cx_keccak_init_no_throw(sha3Context, 256));

    CX_THROW(cx_hash_no_throw((cx_hash_t *) sha3Context,
                              CX_LAST,
                              locals_union.tmp,
                              offset + 40,
                              locals_union.hashChecksum,
                              32));

    for (i = 0; i < 40; i++) {
        uint8_t digit = address[i / 2];
        if ((i % 2) == 0) {
            digit = (digit >> 4) & 0x0f;
        } else {
            digit = digit & 0x0f;
        }
        if (digit < 10) {
            out[i] = HEXDIGITS[digit];
        } else {
            int v = (locals_union.hashChecksum[i / 2] >> (4 * (1 - i % 2))) & 0x0f;
            if (v >= 8) {
                out[i] = HEXDIGITS[digit] - 'a' + 'A';
            } else {
                out[i] = HEXDIGITS[digit];
            }
        }
    }
    out[40] = '\0';
}

void getEthAddressFromKey(cx_ecfp_public_key_t *publicKey, uint8_t *out, cx_sha3_t *sha3Context) {
    uint8_t hashAddress[INT256_LENGTH];

    CX_THROW(cx_keccak_init_no_throw(sha3Context, 256));

    CX_THROW(cx_hash_no_throw((cx_hash_t *) sha3Context,
                              CX_LAST,
                              publicKey->W + 1,
                              64,
                              hashAddress,
                              32));

    memmove(out, hashAddress + 12, 20);
}

void getEthAddressStringFromKey(cx_ecfp_public_key_t *publicKey,
                                char *out,
                                cx_sha3_t *sha3Context) {
    uint8_t hashAddress[HASH_LENGTH];

    CX_THROW(cx_keccak_init_no_throw(sha3Context, 256));

    CX_THROW(cx_hash_no_throw((cx_hash_t *) sha3Context,
                              CX_LAST,
                              publicKey->W + 1,
                              64,
                              hashAddress,
                              32));

    getEthAddressStringFromBinary(hashAddress + 12, out, sha3Context);
}

bool adjustDecimals(const char *src,
                    size_t srcLength,
                    char *target,
                    size_t targetLength,
                    uint8_t decimals) {
    uint32_t startOffset;
    uint32_t lastZeroOffset = 0;
    uint32_t offset = 0;
    if ((srcLength == 1) && (*src == '0')) {
        if (targetLength < 2) {
            return false;
        }
        target[0] = '0';
        target[1] = '\0';
        return true;
    }
    if (srcLength <= decimals) {
        uint32_t delta = decimals - srcLength;
        if (targetLength < srcLength + 1 + 2 + delta) {
            return false;
        }
        target[offset++] = '0';
        target[offset++] = '.';
        for (uint32_t i = 0; i < delta; i++) {
            target[offset++] = '0';
        }
        startOffset = offset;
        for (uint32_t i = 0; i < srcLength; i++) {
            target[offset++] = src[i];
        }
        target[offset] = '\0';
    } else {
        uint32_t sourceOffset = 0;
        uint32_t delta = srcLength - decimals;
        if (targetLength < srcLength + 1 + 1) {
            return false;
        }
        while (offset < delta) {
            target[offset++] = src[sourceOffset++];
        }
        if (decimals != 0) {
            target[offset++] = '.';
        }
        startOffset = offset;
        while (sourceOffset < srcLength) {
            target[offset++] = src[sourceOffset++];
        }
        target[offset] = '\0';
    }
    for (uint32_t i = startOffset; i < offset; i++) {
        if (target[i] == '0') {
            if (lastZeroOffset == 0) {
                lastZeroOffset = i;
            }
        } else {
            lastZeroOffset = 0;
        }
    }
    if (lastZeroOffset != 0) {
        target[lastZeroOffset] = '\0';
        if (target[lastZeroOffset - 1] == '.') {
            target[lastZeroOffset - 1] = '\0';
        }
    }
    return true;
}
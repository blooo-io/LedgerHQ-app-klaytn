#include "os.h"
#include "cx.h"
#include <stdbool.h>
#include <stdlib.h>
#include "ethUtils.h"
#include "globals.h"

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
                                   cx_sha3_t *sha3Context,
                                   uint64_t chainId) {
    // save some precious stack space
    union locals_union {
        uint8_t hashChecksum[HASH_LENGTH];
        uint8_t tmp[51];
    } locals_union;

    uint8_t i;
    bool eip1191 = false;
    uint32_t offset = 0;
    switch (chainId) {
        case 30:
        case 31:
            eip1191 = true;
            break;
    }
    if (eip1191) {
        u64_to_string(chainId, (char *) locals_union.tmp, sizeof(locals_union.tmp));
        offset = strnlen((char *) locals_union.tmp, sizeof(locals_union.tmp));
        strlcat((char *) locals_union.tmp + offset, "0x", sizeof(locals_union.tmp) - offset);
        offset = strnlen((char *) locals_union.tmp, sizeof(locals_union.tmp));
    }
    for (i = 0; i < 20; i++) {
        uint8_t digit = address[i];
        locals_union.tmp[offset + 2 * i] = HEXDIGITS[(digit >> 4) & 0x0f];
        locals_union.tmp[offset + 2 * i + 1] = HEXDIGITS[digit & 0x0f];
    }
    cx_keccak_init(sha3Context, 256);
    cx_hash((cx_hash_t *) sha3Context,
            CX_LAST,
            locals_union.tmp,
            offset + 40,
            locals_union.hashChecksum,
            32);
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

void getEthAddressStringFromKey(cx_ecfp_public_key_t *publicKey,
                                char *out,
                                cx_sha3_t *sha3Context,
                                uint64_t chainId) {
    uint8_t hashAddress[HASH_LENGTH];
    cx_keccak_init(sha3Context, 256);
    cx_hash((cx_hash_t *) sha3Context, CX_LAST, publicKey->W + 1, 64, hashAddress, 32);

    getEthAddressStringFromBinary(hashAddress + 12, out, sha3Context, chainId);
}
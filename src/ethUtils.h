#ifndef _ETHUTILS_H_
#define _ETHUTILS_H_

#include <stdint.h>

#include "cx.h"

void getEthAddressStringFromKey(cx_ecfp_public_key_t *publicKey,
                                char *out,
                                cx_sha3_t *sha3Context,
                                uint64_t chainId);

void u64_to_string(uint64_t src, char *dst, uint8_t dst_size);

void getEthAddressStringFromBinary(uint8_t *address,
                                   char *out,
                                   cx_sha3_t *sha3Context,
                                   uint64_t chainId);

static const char HEXDIGITS[] = "0123456789abcdef";

#endif  // _ETHUTILS_H_

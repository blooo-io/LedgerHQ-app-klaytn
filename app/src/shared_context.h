#ifndef _SHARED_CONTEXT_H_
#define _SHARED_CONTEXT_H_

#include <stdbool.h>
#include <stdint.h>
#include "ethUstream.h"
#include "apdu.h"

#define NETWORK_STRING_MAX_SIZE 16

#define WEI_TO_ETHER 18

typedef struct txStringProperties_t {
    char fullAddress[43];
    char fullAmount[79];  // 2^256 is 78 digits long
    char maxFee[50];
    char nonce[8];  // 10M tx per account ought to be enough for everybody
    char network_name[NETWORK_STRING_MAX_SIZE];
} txStringProperties_t;

#ifdef TARGET_NANOS
#define SHARED_CTX_FIELD_1_SIZE 100
#else
#define SHARED_CTX_FIELD_1_SIZE 256
#endif
#define SHARED_CTX_FIELD_2_SIZE 40

typedef struct strDataTmp_t {
    char tmp[SHARED_CTX_FIELD_1_SIZE];
    char tmp2[SHARED_CTX_FIELD_2_SIZE];
} strDataTmp_t;

typedef union {
    txStringProperties_t common;
    strDataTmp_t tmp;
} strings_t;

extern ApduCommand G_command;
extern txContext_t txContext;
extern tmpContent_t tmpContent;
extern strings_t strings;
extern cx_sha3_t global_sha3;

customStatus_e customProcessor(txContext_t *context);

#endif  // _SHARED_CONTEXT_H_

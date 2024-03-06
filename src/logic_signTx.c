#include "ethUtils.h"
#include "shared_context.h"
#include "ethUstream.h"
#include "utils_copy.h"

#define ERR_SILENT_MODE_CHECK_FAILED 0x6001

void finalizeParsing(txContext_t *txContext) {
    // Store the hash
    CX_THROW(cx_hash_no_throw((cx_hash_t *) txContext->sha3,
                              CX_LAST,
                              G_command.message_hash.data,
                              0,
                              G_command.message_hash.data,
                              32));
}

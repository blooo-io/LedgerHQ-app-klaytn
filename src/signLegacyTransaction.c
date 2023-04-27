#include "apdu.h"
#include "sol/parser.h"
#include "sol/print_config.h"
#include "ethUstream.h"
#include "shared_context.h"

void handle_sign_legacy_transaction(volatile unsigned int *tx) {
    if (!tx || G_command.instruction != InsSignLegacyTransaction ||
        G_command.state != ApduStatePayloadComplete) {
        THROW(ApduReplySdkInvalidParameter);
    }
    cx_sha3_t global_sha3;
    parserStatus_e txResult;

    initTx(&txContext, &global_sha3, &tmpContent.txContent, customProcessor, NULL);

    uint8_t *workBuffer = G_command.message;
    uint8_t dataLength = G_command.message_length;
    PRINTF("workBuffer: %.*H\n", dataLength, workBuffer);

    uint8_t txType = *workBuffer;
    if (txType >= 0x00 && txType <= 0x7f) {
        // Enumerate through all supported txTypes here...
        if (txType == 1 || txType == 2) {
            cx_hash((cx_hash_t *) &global_sha3, 0, workBuffer, 1, NULL, 0);
            txContext.txType = txType;
            workBuffer++;
            dataLength--;
        } else {
            PRINTF("Transaction type %d not supported\n", txType);
            THROW(0x6501);
        }
    } else {
        txContext.txType = LEGACY;
    }

    txResult = processTx(&txContext, workBuffer, dataLength, 0);
}
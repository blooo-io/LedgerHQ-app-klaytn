#include "ethUtils.h"
#include "shared_context.h"
#include "ethUstream.h"
#include "utils_copy.h"

#define ERR_SILENT_MODE_CHECK_FAILED 0x6001

customStatus_e customProcessor(txContext_t *context) {

    strings_t strings;

    if (((context->txType == LEGACY && context->currentField == LEGACY_RLP_DATA) ||
         (context->txType == EIP2930 && context->currentField == EIP2930_RLP_DATA) ||
         (context->txType == EIP1559 && context->currentField == EIP1559_RLP_DATA)) &&
        (context->currentFieldLength != 0)) {
        context->content->dataPresent = true;
        if (tmpContent.txContent.destinationLength == 0) { // TODO : check why this part is killing some tests
            return CUSTOM_NOT_HANDLED;
        }
        if (context->currentFieldLength < 4) {
            return CUSTOM_NOT_HANDLED;
        }
        if (context->currentFieldPos == 0) {
            copyTxData(context, NULL, 4);
            if (context->currentFieldLength == 4) {
                return CUSTOM_NOT_HANDLED;
            }
        }
        uint32_t blockSize;
        uint32_t copySize;
        uint32_t fieldPos = context->currentFieldPos;
        blockSize = 32 - (fieldPos % 32);
        if ((context->currentFieldLength - fieldPos) < blockSize) {
            blockSize = context->currentFieldLength - fieldPos;
        }
        copySize = (context->commandLength < blockSize ? context->commandLength : blockSize);
        PRINTF("currentFieldPos %d copySize %d\n", context->currentFieldPos, copySize);
        // copyTxData(context, dataContext.tokenContext.data + fieldPos, copySize); // ORIGINAL
        copyTxData(context, NULL, copySize);
        if (context->currentFieldPos == context->currentFieldLength) {
            PRINTF("\n\nIncrementing one\n");
            context->currentField++;
            context->processingField = false;
        }
        if (copySize == blockSize) {
            if (fieldPos == 0) {
                // array_hexstr(strings.tmp.tmp, dataContext.tokenContext.data, 4);
                // ux_flow_init(0, ux_confirm_selector_flow, NULL);
                PRINTF("ux_flow_init\n");
            } else {
                uint32_t offset = 0;
                uint32_t i;
                snprintf(strings.tmp.tmp2, sizeof(strings.tmp.tmp2), "Field %d", fieldPos / 32);
                for (i = 0; i < 4; i++) {
                    if (i != 3) {
                        strings.tmp.tmp[offset++] = ':';
                    }
                }
                // ux_flow_init(0, ux_confirm_parameter_flow, NULL);
            }
            return CUSTOM_SUSPENDED;
        } else {
            return CUSTOM_HANDLED;
        }
    }
    return CUSTOM_NOT_HANDLED;
}

void finalizeParsing(txContext_t *txContext) {
    
    // Store the hash
    CX_THROW(cx_hash_no_throw((cx_hash_t *) txContext->sha3,
                              CX_LAST,
                              G_command.message_hash.data,
                              0,
                              G_command.message_hash.data,
                              32));
}

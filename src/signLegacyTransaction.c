#include "apdu.h"
#include "cx.h"
#include "ethUstream.h"
#include "shared_context.h"
#include "parser.h"
#include "print_config.h"
#include "transaction_summary.h"
#include "message.h"
#include "uint_common.h"
#include "utils.h"
#include "utils_copy.h"
#include "ux.h"

void format_signature_out(const uint8_t *signature) {
    memset(G_io_apdu_buffer + 1, 0x00, 64);
    uint8_t offset = 1;
    uint8_t xoffset = 4;  // point to r value
    // copy r
    uint8_t xlength = signature[xoffset - 1];
    if (xlength == 33) {
        xlength = 32;
        xoffset++;
    }
    memmove(G_io_apdu_buffer + offset + 32 - xlength, signature + xoffset, xlength);
    offset += 32;
    xoffset += xlength + 2;  // move over rvalue and TagLEn
    // copy s value
    xlength = signature[xoffset - 1];
    if (xlength == 33) {
        xlength = 32;
        xoffset++;
    }
    memmove(G_io_apdu_buffer + offset + 32 - xlength, signature + xoffset, xlength);
}

static uint8_t set_result_sign_message() {
    size_t sig_len = 100;
    uint8_t signature[100];
    unsigned int info = 0;
    cx_ecfp_private_key_t privateKey;
    BEGIN_TRY {
        TRY {
            get_private_key(&privateKey,
                            G_command.derivation_path,
                            G_command.derivation_path_length);

            CX_THROW(cx_ecdsa_sign_no_throw(&privateKey,
                                            CX_RND_RFC6979 | CX_LAST,
                                            CX_SHA256,
                                            G_command.message_hash.data,
                                            sizeof(G_command.message_hash.data),
                                            signature,
                                            &sig_len,
                                            &info));

            // Taking only the 4 highest bytes
            uint32_t v = (uint32_t) u64_from_BE(chainID.value,
                                                MIN(4, chainID.length));

            G_io_apdu_buffer[0] = (v * 2) + 35;
            if (info & CX_ECCINFO_PARITY_ODD) {
                G_io_apdu_buffer[0]++;
            }
            if (info & CX_ECCINFO_xGTn) {
                G_io_apdu_buffer[0] += 2;
            }
            format_signature_out(signature);
        }
        CATCH_OTHER(e) {
            MEMCLEAR(privateKey);
            THROW(e);
        }
        FINALLY {
            MEMCLEAR(privateKey);
        }
    }
    END_TRY;
    return SIGNATURE_LENGTH + 1;  // 1 byte for v
}

static void send_result_sign_message(void) {
    sendResponse(set_result_sign_message(), true);
}

//////////////////////////////////////////////////////////////////////

UX_STEP_CB(ux_approve_step,
           pb,
           send_result_sign_message(),
           {
               &C_icon_validate_14,
               "Approve",
           });
UX_STEP_CB(ux_reject_step,
           pb,
           sendResponse(0, false),
           {
               &C_icon_crossmark,
               "Reject",
           });
UX_STEP_NOCB_INIT(ux_summary_step,  // rename after deleting the singmessage one
                  bnnn_paging,
                  {
                      size_t step_index = G_ux.flow_stack[stack_slot].index;
                      enum DisplayFlags flags = DisplayFlagNone;
                      if (N_storage.settings.pubkey_display == PubkeyDisplayLong) {
                          flags |= DisplayFlagLongPubkeys;
                      }
                      if (transaction_summary_display_item(step_index)) {
                          THROW(ApduReplyKlaytnSummaryUpdateFailed);
                      }
                  },
                  {
                      .title = G_transaction_summary_title,
                      .text = G_transaction_summary_text,
                  });

#define MAX_FLOW_STEPS                                     \
    (MAX_TRANSACTION_SUMMARY_ITEMS + 1 /* approve */       \
     + 1                               /* reject */        \
     + 1                               /* FLOW_END_STEP */ \
    )
ux_flow_step_t static const *flow_steps[MAX_FLOW_STEPS];

void handle_sign_legacy_transaction(volatile unsigned int *tx) {
    cx_sha3_t sha3;
    txContext_t txContext;
    tmpContent_t tmpContent;
    
    if (!tx || G_command.state != ApduStatePayloadComplete ||
        (G_command.instruction != InsSignLegacyTransaction &&
         G_command.instruction != InsSignValueTransfer &&
         G_command.instruction != InsSignValueTransferMemo &&
         G_command.instruction != InsSignSmartContractDeploy &&
         G_command.instruction != InsSignSmartContractExecution &&
         G_command.instruction != InsSignCancel)) {
        THROW(ApduReplySdkInvalidParameter);
    }

    parserStatus_e txResult;

    initTx(&txContext, &sha3, &tmpContent.txContent, customProcessor, NULL);

    uint8_t *workBuffer = G_command.message;
    uint8_t dataLength = G_command.message_length;
    PRINTF("workBuffer: %.*H\n", dataLength, workBuffer);

    uint8_t txType = getTxType();
    // Enumerate through all supported txTypes here...
    switch (txType) {
        case LEGACY:
            txContext.txType = LEGACY;
            txContext.outerRLP = false;
            break;
        case 1:
        case 2:
        case VALUE_TRANSFER:
        case FEE_DELEGATED_VALUE_TRANSFER:
        case PARTIAL_FEE_DELEGATED_VALUE_TRANSFER:
        case VALUE_TRANSFER_MEMO:
        case FEE_DELEGATED_VALUE_TRANSFER_MEMO:
        case PARTIAL_FEE_DELEGATED_VALUE_TRANSFER_MEMO:
        case SMART_CONTRACT_DEPLOY:
        case FEE_DELEGATED_SMART_CONTRACT_DEPLOY:
        case PARTIAL_FEE_DELEGATED_SMART_CONTRACT_DEPLOY:
        case SMART_CONTRACT_EXECUTION:
        case FEE_DELEGATED_SMART_CONTRACT_EXECUTION:
        case PARTIAL_FEE_DELEGATED_SMART_CONTRACT_EXECUTION:
        case CANCEL:
        case FEE_DELEGATED_CANCEL:
        case PARTIAL_FEE_DELEGATED_CANCEL:
            txContext.txType = txType;
            txContext.outerRLP = true;
            break;
        default:
            PRINTF("Transaction type %d not supported\n", txType);
            THROW(0x6501);
            break;
    }
    txResult = processTx(&txContext, workBuffer, dataLength, 0);
    if (txResult == USTREAM_FINISHED) {
        finalizeParsing(&txContext);
    }
    transaction_summary_reset();
    if (process_message_body(&txContext) != 0) {
        // Message not processed, throw if blind signing is not enabled
        if (N_storage.settings.allow_blind_sign == BlindSignEnabled) {
            SummaryItem *item = transaction_summary_primary_item();
            summary_item_set_string(item, "Unrecognized", "format");

            cx_hash_sha256(G_command.message,
                           G_command.message_length,
                           (uint8_t *) &G_command.message_hash,
                           HASH_LENGTH);

            item = transaction_summary_general_item();
            summary_item_set_hash(item, "Message Hash", &G_command.message_hash);
        } else {
            THROW(ApduReplySdkNotSupported);
        }
    }
}

void handle_sign_legacy_transaction_ui(volatile unsigned int *flags) {
    // Display the transaction summary
    SummaryItemKind_t summary_step_kinds[MAX_TRANSACTION_SUMMARY_ITEMS];
    size_t num_summary_steps = 0;
    if (transaction_summary_finalize(summary_step_kinds, &num_summary_steps) == 0) {
        size_t num_flow_steps = 0;

        for (size_t i = 0; i < num_summary_steps; i++) {
            flow_steps[num_flow_steps++] = &ux_summary_step;
        }

        flow_steps[num_flow_steps++] = &ux_approve_step;
        flow_steps[num_flow_steps++] = &ux_reject_step;
        flow_steps[num_flow_steps++] = FLOW_END_STEP;

        ux_flow_init(0, flow_steps, NULL);
    } else {
        THROW(ApduReplyKlaytnSummaryFinalizeFailed);
    }

    *flags |= IO_ASYNCH_REPLY;
}
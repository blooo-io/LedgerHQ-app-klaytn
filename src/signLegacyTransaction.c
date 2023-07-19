#include "apdu.h"
#include "sol/parser.h"
#include "sol/print_config.h"
#include "ethUstream.h"
#include "shared_context.h"
#include "ux.h"
#include "cx.h"
#include "utils.h"
#include "sol/transaction_summary.h"

static uint8_t set_result_sign_message() {
    uint8_t signature[SIGNATURE_LENGTH];
    cx_ecfp_private_key_t privateKey;
    BEGIN_TRY {
        TRY {
            get_private_key_with_seed(&privateKey,
                                      G_command.derivation_path,
                                      G_command.derivation_path_length);
            cx_eddsa_sign(&privateKey,
                          CX_LAST,
                          CX_SHA512,
                          G_command.message,
                          G_command.message_length,
                          NULL,
                          0,
                          signature,
                          SIGNATURE_LENGTH,
                          NULL);
            memcpy(G_io_apdu_buffer, signature, SIGNATURE_LENGTH);
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
    return SIGNATURE_LENGTH;
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
                      if (transaction_summary_display_item(step_index, flags)) {
                          THROW(ApduReplySolanaSummaryUpdateFailed);
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
    if (!tx || G_command.state != ApduStatePayloadComplete ||
        (G_command.instruction != InsSignLegacyTransaction &&
         G_command.instruction != InsSignValueTransfer &&
         G_command.instruction != InsSignValueTransferMemo &&
         G_command.instruction != InsSignSmartContractDeploy)) {
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
        switch (txType) {
            case 1:
            case 2:
            case InsSignValueTransfer:
            case InsSignValueTransferMemo:
            case InsSignSmartContractDeploy:
                cx_hash((cx_hash_t *) &global_sha3, 0, workBuffer, 1, NULL, 0);
                txContext.txType = txType;
                workBuffer++;
                dataLength--;
                break;
            default:
                PRINTF("Transaction type %d not supported\n", txType);
                THROW(0x6501);
                break;
        }
    } else {
        txContext.txType = LEGACY;
    }

    txResult = processTx(&txContext, workBuffer, dataLength, 0);
    transaction_summary_reset();
    if (process_message_body() != 0) {
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
        THROW(ApduReplySolanaSummaryFinalizeFailed);
    }

    *flags |= IO_ASYNCH_REPLY;
}
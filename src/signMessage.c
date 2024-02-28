#include "getPubkey.h"
#include "os.h"
#include "ux.h"
#include "cx.h"
#include "menu.h"
#include "utils.h"
#include "sol/parser.h"
#include "sol/printer.h"
#include "sol/print_config.h"
#include "sol/message.h"
#include "sol/transaction_summary.h"
#include "globals.h"
#include "apdu.h"

//////////////////////////////////////////////////////////////////////

static int scan_header_for_signer(const uint32_t *derivation_path,
                                  uint32_t derivation_path_length,
                                  size_t *signer_index,
                                  const MessageHeader *header) {
    // to delete warnings
    uint8_t signer_pubkey[PUBKEY_SIZE] = {0};
    publicKeyContext_t publicKeyContext;
    // get_public_key(signer_pubkey, derivation_path, derivation_path_length);
    get_public_key(&publicKeyContext, derivation_path, derivation_path_length);
    for (size_t i = 0; i < header->pubkeys_header.num_required_signatures; ++i) {
        const Pubkey *current_pubkey = &(header->pubkeys[i]);
        if (memcmp(current_pubkey, signer_pubkey, PUBKEY_SIZE) == 0) {
            *signer_index = i;
            return 0;
        }
    }
    return -1;
}

void handle_sign_message_parse_message(volatile unsigned int *tx) {
    if (!tx ||
        (G_command.instruction != InsDeprecatedSignMessage &&
         G_command.instruction != InsSignMessage) ||
        G_command.state != ApduStatePayloadComplete) {
        THROW(ApduReplySdkInvalidParameter);
    }
    // Handle the transaction message signing
    Parser parser = {G_command.message, G_command.message_length};
    PrintConfig print_config;
    print_config.expert_mode = (N_storage.settings.display_mode == DisplayModeExpert);
    print_config.signer_pubkey = NULL;
    MessageHeader *header = &print_config.header;
    size_t signer_index;

    if (parse_message_header(&parser, header) != 0) {
        THROW(ApduReplyKlaytnInvalidMessage);
    }

    // Ensure the requested signer is present in the header
    if (scan_header_for_signer(G_command.derivation_path,
                               G_command.derivation_path_length,
                               &signer_index,
                               header) != 0) {
        THROW(ApduReplyKlaytnInvalidMessageHeader);
    }
    print_config.signer_pubkey = &header->pubkeys[signer_index];

    if (G_command.non_confirm) {
        // Uncomment this to allow unattended signing.
        //*tx = set_result_sign_message();
        // THROW(ApduReplySuccess);
        UNUSED(tx);
        THROW(ApduReplySdkNotSupported);
    }

    // Set the transaction summary
    transaction_summary_reset();
    if (process_message_body(parser.buffer, parser.buffer_length, &print_config) != 0) {
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

    // Add fee payer to summary if needed
    const Pubkey *fee_payer = &header->pubkeys[0];
    if (print_config_show_authority(&print_config, fee_payer)) {
        transaction_summary_set_fee_payer_pubkey(fee_payer);
    }
}

void handle_sign_message_ui(volatile unsigned int *flags) {
    // Display the transaction summary
    SummaryItemKind_t summary_step_kinds[MAX_TRANSACTION_SUMMARY_ITEMS];
    size_t num_summary_steps = 0;
    if (transaction_summary_finalize(summary_step_kinds, &num_summary_steps) == 0) {
        // size_t num_flow_steps = 0;

        // for (size_t i = 0; i < num_summary_steps; i++) {
        //     flow_steps[num_flow_steps++] = &ux_summary_step;
        // }

        // flow_steps[num_flow_steps++] = &ux_approve_step;
        // flow_steps[num_flow_steps++] = &ux_reject_step;
        // flow_steps[num_flow_steps++] = FLOW_END_STEP;

        // ux_flow_init(0, flow_steps, NULL);
    } else {
        THROW(ApduReplyKlaytnSummaryFinalizeFailed);
    }

    *flags |= IO_ASYNCH_REPLY;
}

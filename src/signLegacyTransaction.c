#include "apdu.h"
#include "sol/parser.h"
#include "sol/print_config.h"

void handle_sign_legacy_transaction(volatile unsigned int *tx) {
    if (!tx || G_command.instruction != InsSignLegacyTransaction ||
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

    // PRINTF("%d\n", parser.buffer);

    if (parse_legacy(&parser, header) != 0) {
        // This is not a valid Solana message
        THROW(ApduReplySolanaInvalidMessage);
    }
}
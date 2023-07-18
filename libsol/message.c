#include "instruction.h"
#include "sol/parser.h"
#include "sol/message.h"
#include "sol/print_config.h"
#include "util.h"
#include <string.h>
#include "globals.h"
#include "shared_context.h"

// change this if you want to be able to add successive tx
#define MAX_INSTRUCTIONS 1

int process_message_body() {
    size_t instruction_count = 0;
    InstructionInfo instruction_info[MAX_INSTRUCTIONS];
    explicit_bzero(instruction_info, sizeof(InstructionInfo) * MAX_INSTRUCTIONS);
    size_t display_instruction_count = 0;
    InstructionInfo* display_instruction_info[MAX_INSTRUCTIONS];

    InstructionInfo* info = &instruction_info[instruction_count];

    switch (G_command.instruction) {
        case InsSignLegacyTransaction:
            parse_system_transfer_instruction(&txContext, &info->transaction, "Legacy Transaction");
            break;
        case InsSignValueTransfer:
            parse_system_transfer_instruction(&txContext, &info->transaction, "Value Transfer");
            break;
        case InsSignValueTransferMemo:
            parse_system_transfer_instruction(&txContext,
                                              &info->transaction,
                                              "Value Transfer Memo");
            break;
        default:
            return 0;
    };

    display_instruction_info[display_instruction_count++] = info;

    switch (G_command.instruction) {
        case InsSignLegacyTransaction:
            return print_legacy_transaction_info(&display_instruction_info[0]->transaction);
        case InsSignValueTransfer:
        case InsSignValueTransferMemo:
            return print_value_transfer_info(&display_instruction_info[0]->transaction);
    };
    return 1;
}
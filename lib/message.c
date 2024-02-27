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

    static char fee_delegation_prefix[50];
    if (G_command.p1 == P1_BASIC) {
        strcpy(fee_delegation_prefix, "");
    } else if (G_command.p1 == P1_FEE_DELEGATED) {
        strcpy(fee_delegation_prefix, "Fee Delegated ");
    } else if (G_command.p1 == P1_FEE_DELEGATED_WITH_RATIO) {
        strcpy(fee_delegation_prefix, "Partial Fee Delegated ");
    }

    switch (G_command.instruction) {
        case InsSignLegacyTransaction:
            parse_system_transfer_instruction(&txContext, &info->transaction, (char *)"Legacy Transaction");
            break;
        case InsSignValueTransfer:
            parse_system_transfer_instruction(&txContext,
                                              &info->transaction,
                                              strcat(fee_delegation_prefix, "Value Transfer"));
            break;
        case InsSignValueTransferMemo:
            parse_system_transfer_instruction(&txContext,
                                              &info->transaction,
                                              strcat(fee_delegation_prefix, "Value Transfer Memo"));
            break;
        case InsSignSmartContractDeploy:
            parse_system_transfer_instruction(
                &txContext,
                &info->transaction,
                strcat(fee_delegation_prefix, "Smart Contract Deploy"));

            break;
        case InsSignSmartContractExecution:
            parse_system_transfer_instruction(
                &txContext,
                &info->transaction,
                strcat(fee_delegation_prefix, "Smart Contract Execution"));

            break;
        case InsSignCancel:
            parse_system_transfer_instruction(&txContext,
                                              &info->transaction,
                                              strcat(fee_delegation_prefix, "Cancel"));

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
        case InsSignSmartContractDeploy:
            return print_smart_contract_deploy_info(&display_instruction_info[0]->transaction);
        case InsSignSmartContractExecution:
            return print_smart_contract_execution_info(&display_instruction_info[0]->transaction);
        case InsSignCancel:
            return print_cancel_info(&display_instruction_info[0]->transaction);
    };
    return 1;
}
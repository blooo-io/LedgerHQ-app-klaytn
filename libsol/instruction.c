#include "instruction.h"
#include "serum_assert_owner_instruction.h"
#include "spl_memo_instruction.h"
#include "spl_token_instruction.h"
#include "stake_instruction.h"
#include "util.h"
#include <string.h>
#include "ethUstream.h"
#include "sol/parser.h"

const SYMBOL = "KLAY";
const int DECIMALS = 18;

void instruction_accounts_iterator_init(InstructionAccountsIterator* it,
                                        const MessageHeader* header,
                                        const Instruction* instruction) {
    it->message_header_pubkeys = header->pubkeys;
    it->instruction_accounts_length = instruction->accounts_length;
    it->instruction_accounts = instruction->accounts;
    it->current_instruction_account = 0;
}

int instruction_accounts_iterator_next(InstructionAccountsIterator* it,
                                       const Pubkey** next_account) {
    if (it->current_instruction_account < it->instruction_accounts_length) {
        size_t pubkeys_index = it->instruction_accounts[it->current_instruction_account++];
        if (next_account) {
            *next_account = &it->message_header_pubkeys[pubkeys_index];
        }
        return 0;
    }
    return 1;
}

size_t instruction_accounts_iterator_remaining(const InstructionAccountsIterator* it) {
    if (it->current_instruction_account < it->instruction_accounts_length) {
        return it->instruction_accounts_length - it->current_instruction_account;
    }
    return 0;
}

uint64_t convertUint256ToUint64(const txInt256_t* bytes) {
    uint64_t result = 0;
    for (int i = 0; i < bytes->length && i < 8; i++) {
        result <<= 8;  // Shift existing value left by 8 bits
        result |= (uint64_t) bytes->value[i];
    }
    return result;
}

int parse_system_transfer_instruction(txContext_t* context,
                                      SystemTransferInfo* info,
                                      char* method_name) {
    SizedString method_name_ss = {
        strlen(method_name),
        (char*) method_name,
    };
    // Method name
    info->method_name = method_name_ss;

    // Address to
    info->to = context->content->destination;

    // Amount
    // info->amount = convertUint256ToUint64(&context->content->value); //0x2b5e3af16b1880000

    // Display Amount

    info->display_amount = context->content->value;  // maybe display_amount.value

    // Nonce
    info->nonce = convertUint256ToUint64(&context->content->nonce);

    // Gas Price
    info->gas_price = convertUint256ToUint64(&context->content->gasprice);

    // Gas
    info->gas = convertUint256ToUint64(&context->content->startgas);

    // Fee Ratio
    info->fee_ratio = context->content->ratio;

    return 1;
}

int print_legacy_transaction_info(const SystemTransferInfo* info) {
    SummaryItem* item;

    item = transaction_summary_primary_item();
    summary_item_set_sized_string(item, "Transaction", &info->method_name);

    item = transaction_summary_general_item();
    summary_item_set_token_amount(item, "Amount", info->display_amount, SYMBOL, DECIMALS);

    item = transaction_summary_general_item();
    summary_item_set_pubkey(item, "Recipient", info->to);

    item = transaction_summary_general_item();
    summary_item_set_u64(item, "Nonce", info->nonce);

    item = transaction_summary_general_item();
    summary_item_set_u64(item, "Gas Price", info->gas_price);

    item = transaction_summary_general_item();
    summary_item_set_u64(item, "Gas", info->gas);

    return 0;
}

int print_value_transfer_info(const SystemTransferInfo* info) {
    SummaryItem* item;

    item = transaction_summary_primary_item();
    summary_item_set_sized_string(item, "Transaction", &info->method_name);

    item = transaction_summary_general_item();
    summary_item_set_token_amount(item, "Amount", info->display_amount, SYMBOL, DECIMALS);

    item = transaction_summary_general_item();
    summary_item_set_pubkey(item, "Recipient", info->to);

    item = transaction_summary_general_item();
    summary_item_set_u64(item, "Nonce", info->nonce);

    item = transaction_summary_general_item();
    summary_item_set_u64(item, "Gas Price", info->gas_price);

    item = transaction_summary_general_item();
    summary_item_set_u64(item, "Gas", info->gas);

    if (G_command.p1 == P1_FEE_DELEGATED_WITH_RATIO) {
        item = transaction_summary_general_item();
        summary_item_set_u64(item, "Fee Ratio", info->fee_ratio);
    }

    return 0;
}

int print_smart_contract_deploy_info(const SystemTransferInfo* info) {
    SummaryItem* item;

    item = transaction_summary_primary_item();
    summary_item_set_sized_string(item, "Transaction", &info->method_name);

    item = transaction_summary_general_item();
    summary_item_set_amount(item, "Amount", info->amount);

    item = transaction_summary_general_item();
    summary_item_set_u64(item, "Nonce", info->nonce);

    item = transaction_summary_general_item();
    summary_item_set_u64(item, "Gas Price", info->gas_price);

    item = transaction_summary_general_item();
    summary_item_set_u64(item, "Gas", info->gas);

    if (G_command.p1 == P1_FEE_DELEGATED_WITH_RATIO) {
        item = transaction_summary_general_item();
        summary_item_set_u64(item, "Fee Ratio", info->fee_ratio);
    }

    return 0;
}

int print_smart_contract_execution_info(const SystemTransferInfo* info) {
    SummaryItem* item;

    item = transaction_summary_primary_item();
    summary_item_set_sized_string(item, "Transaction", &info->method_name);

    item = transaction_summary_general_item();
    summary_item_set_amount(item, "Amount", info->amount);

    item = transaction_summary_general_item();
    summary_item_set_pubkey(item, "Smart Contract", info->to);

    item = transaction_summary_general_item();
    summary_item_set_u64(item, "Nonce", info->nonce);

    item = transaction_summary_general_item();
    summary_item_set_u64(item, "Gas Price", info->gas_price);

    item = transaction_summary_general_item();
    summary_item_set_u64(item, "Gas", info->gas);

    if (G_command.p1 == P1_FEE_DELEGATED_WITH_RATIO) {
        item = transaction_summary_general_item();
        summary_item_set_u64(item, "Fee Ratio", info->fee_ratio);
    }

    return 0;
}

int print_cancel_info(const SystemTransferInfo* info) {
    SummaryItem* item;

    item = transaction_summary_primary_item();
    summary_item_set_sized_string(item, "Transaction", &info->method_name);

    item = transaction_summary_general_item();
    summary_item_set_u64(item, "Cancelled Tx Nonce", info->nonce);

    item = transaction_summary_general_item();
    summary_item_set_u64(item, "Gas Price", info->gas_price);

    item = transaction_summary_general_item();
    summary_item_set_u64(item, "Gas", info->gas);

    if (G_command.p1 == P1_FEE_DELEGATED_WITH_RATIO) {
        item = transaction_summary_general_item();
        summary_item_set_u64(item, "Fee Ratio", info->fee_ratio);
    }

    return 0;
}
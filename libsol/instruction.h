#pragma once

#include "sol/parser.h"
#include "spl_associated_token_account_instruction.h"
#include "spl_token_instruction.h"
#include "stake_instruction.h"
#include "vote_instruction.h"
#include <stdbool.h>
#include "ethUstream.h"

enum ProgramId {
    ProgramIdUnknown = 0,
    ProgramIdStake,
    ProgramIdSystem,
    ProgramIdVote,
    ProgramIdSplToken,
    ProgramIdSplAssociatedTokenAccount,
    ProgramIdSplMemo,
    ProgramIdSerumAssertOwner,
};

typedef struct SystemTransferInfo {
    const Pubkey* from;
    const Pubkey* to;
    uint32_t ref_block_number;
    SizedString ref_block_prefix;
    SizedString method_name;
    const Pubkey* dest;
    SizedString ticker;
    // TODO: change following to int256
    uint64_t amount;
    uint64_t nonce;
    uint64_t gas_price;
    uint64_t gas;
} SystemTransferInfo;

typedef struct InstructionInfo {
    union {
        SystemTransferInfo transaction;
    };
} InstructionInfo;

#define SPL_ASSOCIATED_TOKEN_ACCOUNT_IX_BRIEF \
    { ProgramIdSplAssociatedTokenAccount, .none = 0 }
#define SPL_TOKEN_IX_BRIEF(spl_token_ix) \
    { ProgramIdSplToken, .spl_token = (spl_token_ix) }
#define SYSTEM_IX_BRIEF(system_ix) \
    { ProgramIdSystem, .system = (system_ix) }
#define STAKE_IX_BRIEF(stake_ix) \
    { ProgramIdStake, .stake = (stake_ix) }
#define VOTE_IX_BRIEF(vote_ix) \
    { ProgramIdVote, .vote = (vote_ix) }

typedef struct InstructionAccountsIterator {
    const Pubkey* message_header_pubkeys;
    uint8_t instruction_accounts_length;
    const uint8_t* instruction_accounts;
    size_t current_instruction_account;
} InstructionAccountsIterator;

void instruction_accounts_iterator_init(InstructionAccountsIterator* it,
                                        const MessageHeader* header,
                                        const Instruction* instruction);

int instruction_accounts_iterator_next(InstructionAccountsIterator* it,
                                       const Pubkey** next_account);

size_t instruction_accounts_iterator_remaining(const InstructionAccountsIterator* it);

uint64_t convertUint256ToUint64(const txInt256_t* bytes);

int parse_system_transfer_instruction(txContext_t* context,
                                      SystemTransferInfo* info,
                                      char* method_name);

int print_legacy_transaction_info(const SystemTransferInfo* info);
int print_value_transfer_info(const SystemTransferInfo* info);
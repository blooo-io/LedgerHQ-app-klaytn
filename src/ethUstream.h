/*******************************************************************************
 *   Ledger Ethereum App
 *   (c) 2016-2019 Ledger
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 ********************************************************************************/

#ifndef _ETHUSTREAM_H_
#define _ETHUSTREAM_H_

#include <stdbool.h>
#include <stdint.h>

#include "os.h"
#include "cx.h"

#include "apdu.h"

#define TX_FLAG_TYPE   0x01
#define ADDRESS_LENGTH 20
#define INT128_LENGTH  16
#define INT256_LENGTH  32

struct txContext_t;

typedef struct txInt256_t {
    uint8_t value[INT256_LENGTH];
    uint8_t length;
} txInt256_t;

typedef struct txContent_t {
    txInt256_t gasprice;  // Used as MaxFeePerGas when dealing with EIP1559 transactions.
    txInt256_t startgas;  // Also known as `gasLimit`.
    txInt256_t value;
    txInt256_t nonce;
    txInt256_t chainID;
    uint8_t destination[ADDRESS_LENGTH];
    uint8_t destinationLength;
    uint8_t v[8];
    uint8_t vLength;
    bool dataPresent;
} txContent_t;

typedef union {
    txContent_t txContent;
    cx_sha256_t sha2;
    char tmp[100];
} tmpContent_t;

typedef enum customStatus_e {
    CUSTOM_NOT_HANDLED,
    CUSTOM_HANDLED,
    CUSTOM_SUSPENDED,
    CUSTOM_FAULT
} customStatus_e;

typedef customStatus_e (*ustreamProcess_t)(struct txContext_t *context);

// First variant of every Tx enum.
#define RLP_NONE 0

#define PARSING_IS_DONE(ctx)                                                                      \
    ((ctx->txType == LEGACY && ctx->currentField == LEGACY_RLP_DONE) ||                           \
     (ctx->txType == EIP2930 && ctx->currentField == EIP2930_RLP_DONE) ||                         \
     (ctx->txType == EIP1559 && ctx->currentField == EIP1559_RLP_DONE) ||                         \
     (ctx->txType == VALUE_TRANSFER && ctx->currentField == VALUE_TRANSFER_RLP_DONE) ||           \
     (ctx->txType == VALUE_TRANSFER_MEMO && ctx->currentField == VALUE_TRANSFER_MEMO_RLP_DONE) || \
     (ctx->txType == SMART_CONTRACT_DEPLOY &&                                                     \
      ctx->currentField == SMART_CONTRACT_DEPLOY_RLP_DONE) ||                                     \
     (ctx->txType == SMART_CONTRACT_EXECUTION &&                                                  \
      ctx->currentField == SMART_CONTRACT_EXECUTION_RLP_DONE) ||                                  \
     (ctx->txType == CANCEL && ctx->currentField == CANCEL_RLP_DONE))

typedef enum rlpLegacyTxField_e {
    LEGACY_RLP_NONE = RLP_NONE,
    LEGACY_RLP_CONTENT,
    LEGACY_RLP_TYPE,
    LEGACY_RLP_NONCE,
    LEGACY_RLP_GASPRICE,
    LEGACY_RLP_STARTGAS,
    LEGACY_RLP_TO,
    LEGACY_RLP_VALUE,
    LEGACY_RLP_DATA,
    LEGACY_RLP_V,
    LEGACY_RLP_R,
    LEGACY_RLP_S,
    LEGACY_RLP_DONE
} rlpLegacyTxField_e;

typedef enum rlpValueTransferTxField_e {
    VALUE_TRANSFER_RLP_NONE = RLP_NONE,
    VALUE_TRANSFER_RLP_CONTENT,
    VALUE_TRANSFER_RLP_TYPE,
    VALUE_TRANSFER_RLP_NONCE,
    VALUE_TRANSFER_RLP_GASPRICE,
    VALUE_TRANSFER_RLP_GASLIMIT,
    VALUE_TRANSFER_RLP_TO,
    VALUE_TRANSFER_RLP_VALUE,
    VALUE_TRANSFER_RLP_DONE
} rlpValueTransferTxField_e;

typedef enum rlpValueTransferMemoTxField_e {
    VALUE_TRANSFER_MEMO_RLP_NONE = RLP_NONE,
    VALUE_TRANSFER_MEMO_RLP_CONTENT,
    VALUE_TRANSFER_MEMO_RLP_TYPE,
    VALUE_TRANSFER_MEMO_RLP_NONCE,
    VALUE_TRANSFER_MEMO_RLP_GASPRICE,
    VALUE_TRANSFER_MEMO_RLP_GASLIMIT,
    VALUE_TRANSFER_MEMO_RLP_TO,
    VALUE_TRANSFER_MEMO_RLP_VALUE,
    VALUE_TRANSFER_MEMO_RLP_FROM,
    VALUE_TRANSFER_MEMO_RLP_DATA,
    VALUE_TRANSFER_MEMO_RLP_DONE
} rlpValueTransferMemoTxField_e;

typedef enum rlpSmartContractDeployTxField_e {
    SMART_CONTRACT_DEPLOY_RLP_NONE = RLP_NONE,
    SMART_CONTRACT_DEPLOY_RLP_CONTENT,
    SMART_CONTRACT_DEPLOY_RLP_TYPE,
    SMART_CONTRACT_DEPLOY_RLP_NONCE,
    SMART_CONTRACT_DEPLOY_RLP_GASPRICE,
    SMART_CONTRACT_DEPLOY_RLP_GASLIMIT,
    SMART_CONTRACT_DEPLOY_RLP_TO,
    SMART_CONTRACT_DEPLOY_RLP_VALUE,
    SMART_CONTRACT_DEPLOY_RLP_FROM,
    SMART_CONTRACT_DEPLOY_RLP_DATA,
    SMART_CONTRACT_DEPLOY_RLP_HUMAN_READABLE,
    SMART_CONTRACT_DEPLOY_RLP_CODE_FORMAT,
    SMART_CONTRACT_DEPLOY_RLP_DONE
} rlpSmartContractDeployTxField_e;

typedef enum rlpSmartContractExecutionTxField_e {
    SMART_CONTRACT_EXECUTION_RLP_NONE = RLP_NONE,
    SMART_CONTRACT_EXECUTION_RLP_CONTENT,
    SMART_CONTRACT_EXECUTION_RLP_TYPE,
    SMART_CONTRACT_EXECUTION_RLP_NONCE,
    SMART_CONTRACT_EXECUTION_RLP_GASPRICE,
    SMART_CONTRACT_EXECUTION_RLP_GASLIMIT,
    SMART_CONTRACT_EXECUTION_RLP_TO,
    SMART_CONTRACT_EXECUTION_RLP_VALUE,
    SMART_CONTRACT_EXECUTION_RLP_FROM,
    SMART_CONTRACT_EXECUTION_RLP_DATA,
    SMART_CONTRACT_EXECUTION_RLP_DONE
} rlpSmartContractExecutionTxField_e;

typedef enum rlpCancelTxField_e {
    CANCEL_RLP_NONE = RLP_NONE,
    CANCEL_RLP_CONTENT,
    CANCEL_RLP_TYPE,
    CANCEL_RLP_NONCE,
    CANCEL_RLP_GASPRICE,
    CANCEL_RLP_GASLIMIT,
    CANCEL_RLP_FROM,
    CANCEL_RLP_DONE
} rlpCancelTxField_e;

typedef enum rlpEIP2930TxField_e {
    EIP2930_RLP_NONE = RLP_NONE,
    EIP2930_RLP_CONTENT,
    EIP2930_RLP_TYPE,  // For wanchain
    EIP2930_RLP_CHAINID,
    EIP2930_RLP_NONCE,
    EIP2930_RLP_GASPRICE,
    EIP2930_RLP_GASLIMIT,
    EIP2930_RLP_TO,
    EIP2930_RLP_VALUE,
    EIP2930_RLP_DATA,
    EIP2930_RLP_ACCESS_LIST,
    EIP2930_RLP_DONE
} rlpEIP2930TxField_e;

typedef enum rlpEIP1559TxField_e {
    EIP1559_RLP_NONE = RLP_NONE,
    EIP1559_RLP_CONTENT,
    EIP1559_RLP_TYPE,  // For wanchain
    EIP1559_RLP_CHAINID,
    EIP1559_RLP_NONCE,
    EIP1559_RLP_MAX_PRIORITY_FEE_PER_GAS,
    EIP1559_RLP_MAX_FEE_PER_GAS,
    EIP1559_RLP_GASLIMIT,
    EIP1559_RLP_TO,
    EIP1559_RLP_VALUE,
    EIP1559_RLP_DATA,
    EIP1559_RLP_ACCESS_LIST,
    EIP1559_RLP_DONE
} rlpEIP1559TxField_e;

#define MIN_TX_TYPE 0x00
#define MAX_TX_TYPE 0x7f

// EIP 2718 TransactionType
// Valid transaction types should be in [0x00, 0x7f]
typedef enum txType_e {
    EIP2930 = 0x01,
    EIP1559 = 0x02,

    VALUE_TRANSFER = 0x08,
    FEE_DELEGATED_VALUE_TRANSFER = 0x09,

    VALUE_TRANSFER_MEMO = 0x10,
    FEE_DELEGATED_VALUE_TRANSFER_MEMO = 0x11,

    SMART_CONTRACT_DEPLOY = 0x28,
    FEE_DELEGATED_SMART_CONTRACT_DEPLOY = 0x29,

    SMART_CONTRACT_EXECUTION = 0x30,
    FEE_DELEGATED_SMART_CONTRACT_EXECUTION = 0x31,

    CANCEL = 0x38,
    FEE_DELEGATED_CANCEL = 0x39,

    LEGACY = 0xc0  // Legacy tx are greater than or equal to 0xc0.
} txType_e;

static inline uint8_t getTxType() {
    switch (G_command.instruction) {
        case InsSignLegacyTransaction:
            return LEGACY;
        case InsSignValueTransfer:
            if (G_command.p1 == P1_FEE_DELEGATED) {
                return FEE_DELEGATED_VALUE_TRANSFER;
            } else {
                return VALUE_TRANSFER;
            }
            return VALUE_TRANSFER;
        case InsSignValueTransferMemo:
            if (G_command.p1 == P1_FEE_DELEGATED) {
                return FEE_DELEGATED_VALUE_TRANSFER_MEMO;
            } else {
                return VALUE_TRANSFER_MEMO;
            }
        case InsSignSmartContractDeploy:
            if (G_command.p1 == P1_FEE_DELEGATED) {
                return FEE_DELEGATED_SMART_CONTRACT_DEPLOY;
            } else {
                return SMART_CONTRACT_DEPLOY;
            }
        case InsSignSmartContractExecution:
            if (G_command.p1 == P1_FEE_DELEGATED) {
                return FEE_DELEGATED_SMART_CONTRACT_EXECUTION;
            } else {
                return SMART_CONTRACT_EXECUTION;
            }
        case InsSignCancel:
            if (G_command.p1 == P1_FEE_DELEGATED) {
                return FEE_DELEGATED_CANCEL;
            } else {
                return CANCEL;
            }
        default:
            THROW(ApduReplySdkInvalidParameter);
    }
}

typedef enum parserStatus_e {
    USTREAM_PROCESSING,  // Parsing is in progress
    USTREAM_SUSPENDED,   // Parsing has been suspended
    USTREAM_FINISHED,    // Parsing is done
    USTREAM_FAULT,       // An error was encountered while parsing
    USTREAM_CONTINUE     // Used internally to signify we can keep on parsing
} parserStatus_e;

typedef struct txContext_t {
    // To process the outer RLP in Klaytn specific txs
    bool outerRLP;
    bool processingOuterRLPField;
    // To process eth compatible txs and the inner RLP in Klaytn specific txs
    uint8_t currentField;
    cx_sha3_t *sha3;
    uint32_t currentFieldLength;
    uint32_t currentFieldPos;
    bool currentFieldIsList;
    bool processingField;
    bool fieldSingleByte;
    uint32_t dataLength;
    uint8_t rlpBuffer[5];
    uint32_t rlpBufferPos;
    const uint8_t *workBuffer;
    uint32_t commandLength;
    uint32_t processingFlags;
    ustreamProcess_t customProcessor;
    txContent_t *content;
    void *extra;
    uint8_t txType;
} txContext_t;

void initTx(txContext_t *context,
            cx_sha3_t *sha3,
            txContent_t *content,
            ustreamProcess_t customProcessor,
            void *extra);
parserStatus_e processTx(txContext_t *context,
                         const uint8_t *buffer,
                         uint32_t length,
                         uint32_t processingFlags);
parserStatus_e continueTx(txContext_t *context);
void copyTxData(txContext_t *context, uint8_t *out, uint32_t length);
uint8_t readTxByte(txContext_t *context);

#endif  // _ETHUSTREAM_H_

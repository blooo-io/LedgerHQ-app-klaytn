#include "shared_context.h"
#include "utils_copy.h"
#include "ethUtils.h"

#define ERR_SILENT_MODE_CHECK_FAILED 0x6001

uint32_t splitBinaryParameterPart(char *result, uint8_t *parameter) {
    uint32_t i;
    for (i = 0; i < 8; i++) {
        if (parameter[i] != 0x00) {
            break;
        }
    }
    if (i == 8) {
        result[0] = '0';
        result[1] = '0';
        result[2] = '\0';
        return 2;
    } else {
        array_hexstr(result, parameter + i, 8 - i);
        return ((8 - i) * 2);
    }
}

customStatus_e customProcessor(txContext_t *context) {
    if (((context->txType == LEGACY && context->currentField == LEGACY_RLP_DATA) ||
         (context->txType == EIP2930 && context->currentField == EIP2930_RLP_DATA) ||
         (context->txType == EIP1559 && context->currentField == EIP1559_RLP_DATA)) &&
        (context->currentFieldLength != 0)) {
        context->content->dataPresent = true;
        if (tmpContent.txContent.destinationLength == 0) {
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
                    // offset +=
                    //     splitBinaryParameterPart(strings.tmp.tmp + offset,
                    //                              dataContext.tokenContext.data + fieldPos + 8 *
                    //                              i);
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

void to_uppercase(char *str, unsigned char size) {
    for (unsigned char i = 0; i < size && str[i] != 0; i++) {
        str[i] = str[i] >= 'a' ? str[i] - ('a' - 'A') : str[i];
    }
}

void compareOrCopy(char *preapproved_string, size_t size, char *parsed_string, bool silent_mode) {
    if (silent_mode) {
        /* ETH address are not fundamentally case sensitive but might
        have some for checksum purpose, so let's get rid of these diffs */
        to_uppercase(preapproved_string, strlen(preapproved_string));
        to_uppercase(parsed_string, strlen(parsed_string));
        if (memcmp(preapproved_string, parsed_string, strlen(preapproved_string))) {
            THROW(ERR_SILENT_MODE_CHECK_FAILED);
        }
    } else {
        strlcpy(preapproved_string, parsed_string, size);
    }
}

// void reportFinalizeError(bool direct) {
//     reset_main_globals();
//     if (direct) {
//         THROW(0x6A80);
//     } else {
//         io_seproxyhal_send_status(0x6A80);
//         ui_idle();
//     }
// }

// Convert `BEgasPrice` and `BEgasLimit` to Uint256 and then stores the multiplication of both in
// `output`.
static void computeFees(txInt256_t *BEgasPrice, txInt256_t *BEgasLimit, uint256_t *output) {
    uint256_t gasPrice = {0};
    uint256_t gasLimit = {0};

    PRINTF("Gas price %.*H\n", BEgasPrice->length, BEgasPrice->value);
    PRINTF("Gas limit %.*H\n", BEgasLimit->length, BEgasLimit->value);
    convertUint256BE(BEgasPrice->value, BEgasPrice->length, &gasPrice);
    convertUint256BE(BEgasLimit->value, BEgasLimit->length, &gasLimit);
    mul256(&gasPrice, &gasLimit, output);
}

static void feesToString(uint256_t *rawFee, char *displayBuffer, uint32_t displayBufferSize) {
    const char *feeTicker = "KLAY";
    uint8_t tickerOffset = 0;
    uint32_t i;

    tostring256(rawFee, 10, (char *) (G_io_apdu_buffer + 100), 100);
    i = 0;
    while (G_io_apdu_buffer[100 + i]) {
        i++;
    }
    adjustDecimals((char *) (G_io_apdu_buffer + 100),
                   i,
                   (char *) G_io_apdu_buffer,
                   100,
                   WEI_TO_ETHER);
    i = 0;
    tickerOffset = 0;
    memset(displayBuffer, 0, displayBufferSize);

    while (feeTicker[tickerOffset]) {
        if ((uint32_t) tickerOffset >= displayBufferSize) {
            break;
        }

        displayBuffer[tickerOffset] = feeTicker[tickerOffset];
        tickerOffset++;
    }
    while (G_io_apdu_buffer[i]) {
        if ((uint32_t) (tickerOffset) + i >= displayBufferSize) {
            break;
        }
        displayBuffer[tickerOffset + i] = G_io_apdu_buffer[i];
        i++;
    }

    if ((uint32_t) (tickerOffset) + i < displayBufferSize) {
        displayBuffer[tickerOffset + i] = '\0';
    }
}

// Compute the fees, transform it to a string, prepend a ticker to it and copy everything to
// `displayBuffer`.
void prepareAndCopyFees(txInt256_t *BEGasPrice,
                        txInt256_t *BEGasLimit,
                        char *displayBuffer,
                        uint32_t displayBufferSize) {
    uint256_t rawFee = {0};
    computeFees(BEGasPrice, BEGasLimit, &rawFee);
    feesToString(&rawFee, displayBuffer, displayBufferSize);
}

void prepareFeeDisplay() {
    prepareAndCopyFees(&tmpContent.txContent.gasprice,
                       &tmpContent.txContent.startgas,
                       strings.common.maxFee,
                       sizeof(strings.common.maxFee));
}

void prepareNetworkDisplay() {
    const char *name = "Klaytn";
    if (name == NULL) {
        // No network name found so simply copy the chain ID as the network name.

        uint64_t chain_id = u64_from_BE(txContext.content->chainID.value,
                                        MIN(4, txContext.content->chainID.length));
        ;  // 1001
        u64_to_string(chain_id, strings.common.network_name, sizeof(strings.common.network_name));
    } else {
        // Network name found, simply copy it.
        strlcpy(strings.common.network_name, name, sizeof(strings.common.network_name));
    }
}

static void get_public_key(uint8_t *out, uint8_t outLength) {
    uint8_t privateKeyData[INT256_LENGTH] = {0};
    cx_ecfp_private_key_t privateKey = {0};
    cx_ecfp_public_key_t publicKey = {0};

    if (outLength < ADDRESS_LENGTH) {
        return;
    }
    os_perso_derive_node_bip32(CX_CURVE_256K1,
                               G_command.derivation_path,
                               G_command.derivation_path_length,
                               privateKeyData,
                               NULL);
    cx_ecfp_init_private_key(CX_CURVE_256K1, privateKeyData, 32, &privateKey);
    cx_ecfp_generate_pair(CX_CURVE_256K1, &publicKey, &privateKey, 1);
    explicit_bzero(&privateKey, sizeof(privateKey));
    explicit_bzero(privateKeyData, sizeof(privateKeyData));
    getEthAddressFromKey(&publicKey, out, &global_sha3);
}

void finalizeParsing(bool direct) {
    // Store the hash
    cx_hash_no_throw((cx_hash_t *) &global_sha3,
                     CX_LAST,
                     G_command.message_hash.data,
                     0,
                     G_command.message_hash.data,
                     32);
}

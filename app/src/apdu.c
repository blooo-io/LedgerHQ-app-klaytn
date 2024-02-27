#include "apdu.h"
#include "utils.h"

/**
 * Deserialize APDU into ApduCommand structure.
 *
 * @param[in] apdu_message
 *   Pointer to raw APDU buffer.
 * @param[in] apdu_message_len
 *   Size of the APDU buffer.
 * @param[out] apdu_command
 *   Pointer to ApduCommand structure.
 *
 * @return zero on success, ApduReply error code otherwise.
 *
 */
int apdu_handle_message(const uint8_t* apdu_message,
                        size_t apdu_message_len,
                        ApduCommand* apdu_command) {
    if (!apdu_command || !apdu_message) {
        return ApduReplySdkInvalidParameter;
    }

    // parse header
    ApduHeader header = {0};

    // must at least hold the class and instruction
    if (apdu_message_len <= OFFSET_INS) {
        return ApduReplySolanaInvalidMessageSize;
    }

    header.class = apdu_message[OFFSET_CLA];
    if (header.class != CLA) {
        return ApduReplySolanaInvalidMessageHeader;
    }

    header.instruction = apdu_message[OFFSET_INS];
    switch (header.instruction) {
        case InsDeprecatedGetAppConfiguration:
        case InsDeprecatedGetPubkey:
        case InsDeprecatedSignMessage: {
            // must at least hold a full deprecated header
            if (apdu_message_len < DEPRECATED_OFFSET_CDATA) {
                return ApduReplySolanaInvalidMessageSize;
            }

            // deprecated data may be up to 64KiB
            if (apdu_message_len > UINT16_MAX) {
                return ApduReplySolanaInvalidMessageSize;
            }

            header.data_length = apdu_message ? U2BE(apdu_message, OFFSET_LC) : 0;
            if (apdu_message_len != header.data_length + DEPRECATED_OFFSET_CDATA) {
                return ApduReplySolanaInvalidMessageSize;
            }

            if (header.data_length > 0) {
                header.data = apdu_message + DEPRECATED_OFFSET_CDATA;
            }

            header.deprecated_host = true;

            break;
        }
        case InsSignLegacyTransaction:
        case InsSignValueTransfer:
        case InsSignValueTransferMemo:
        case InsSignSmartContractDeploy:
        case InsSignSmartContractExecution:
        case InsSignCancel:
        case InsGetAppConfiguration:
        case InsGetPubkey:
        case InsSignMessage:
        case InsSignOffchainMessage: {
            // must at least hold a full modern header
            if (apdu_message_len < OFFSET_CDATA) {
                return ApduReplySolanaInvalidMessageSize;
            }
            // modern data may be up to 255B
            if (apdu_message_len > UINT8_MAX + OFFSET_CDATA) {
                return ApduReplySolanaInvalidMessageSize;
            }

            header.data_length = apdu_message[OFFSET_LC];
            if (apdu_message_len != header.data_length + OFFSET_CDATA) {
                return ApduReplySolanaInvalidMessageSize;
            }

            if (header.data_length > 0) {
                header.data = apdu_message + OFFSET_CDATA;
            }

            header.deprecated_host = false;

            break;
        }
        default:
            return ApduReplyUnimplementedInstruction;
    }

    header.p1 = apdu_message[OFFSET_P1];
    header.p2 = apdu_message[OFFSET_P2];
    // P2_EXTEND is set to signal that this APDU buffer extends, rather
    // than replaces, the current message buffer
    const bool first_data_chunk = !(header.p2 & P2_EXTEND);

    if (header.instruction == InsDeprecatedGetAppConfiguration ||
        header.instruction == InsGetAppConfiguration) {
        // return early if no data is expected for the command
        explicit_bzero(apdu_command, sizeof(ApduCommand));
        apdu_command->state = ApduStatePayloadComplete;
        apdu_command->instruction = header.instruction;
        apdu_command->non_confirm = (header.p1 == P1_NON_CONFIRM);
        apdu_command->deprecated_host = header.deprecated_host;
        return 0;
    } else if (header.instruction == InsDeprecatedSignMessage ||
               header.instruction == InsSignMessage ||
               header.instruction == InsSignOffchainMessage) {
        if (!first_data_chunk) {
            // validate the command in progress
            if (apdu_command->state != ApduStatePayloadInProgress ||
                apdu_command->instruction != header.instruction ||
                apdu_command->non_confirm != (header.p1 == P1_NON_CONFIRM) ||
                apdu_command->deprecated_host != header.deprecated_host) {
                return ApduReplySolanaInvalidMessage;
            }
        } else {
            explicit_bzero(apdu_command, sizeof(ApduCommand));
        }
    } else {
        explicit_bzero(apdu_command, sizeof(ApduCommand));
    }

    // read derivation path
    if (first_data_chunk) {
        if (!header.deprecated_host && header.instruction != InsGetPubkey) {
            if (!header.data_length) {
                return ApduReplySolanaInvalidMessageSize;
            }
        }
        const int ret = read_derivation_path(header.data,
                                             header.data_length,
                                             apdu_command->derivation_path,
                                             &apdu_command->derivation_path_length);
        PRINTF("read_derivation_path: %.*H\n",
               sizeof(uint32_t) * apdu_command->derivation_path_length,
               apdu_command->derivation_path);
        if (ret) {
            return ret;
        }
        header.data += 1 + apdu_command->derivation_path_length * 4;
        header.data_length -= 1 + apdu_command->derivation_path_length * 4;
    }
    apdu_command->p1 = header.p1;
    apdu_command->p2 = header.p2;
    apdu_command->state = ApduStatePayloadInProgress;
    apdu_command->instruction = header.instruction;
    apdu_command->non_confirm = (header.p1 == P1_NON_CONFIRM);
    apdu_command->deprecated_host = header.deprecated_host;

    // copy data to the buffer
    if (header.instruction == InsDeprecatedSignMessage) {
        // deprecated signmessage had a u16 data length prefix... deal with that
        if (header.data_length < 2) {
            return ApduReplySolanaInvalidMessageSize;
        }
        const size_t data_len = header.data ? U2BE(header.data, 0) : 0;
        header.data += 2;
        header.data_length -= 2;
        if (header.data_length != data_len) {
            return ApduReplySolanaInvalidMessageSize;
        }
    }

    if (header.data) {
        if (apdu_command->message_length + header.data_length > MAX_MESSAGE_LENGTH) {
            return ApduReplySolanaInvalidMessageSize;
        }

        memcpy(apdu_command->message + apdu_command->message_length,
               header.data,
               header.data_length);
        apdu_command->message_length += header.data_length;
    } else if (header.instruction != InsDeprecatedGetPubkey && header.instruction != InsGetPubkey) {
        return ApduReplySolanaInvalidMessageSize;
    }

    // check if more data is expected
    if (header.p2 & P2_MORE) {
        return 0;
    }
    PRINTF("apdu_command->message: %.*H\n", apdu_command->message_length, apdu_command->message);
    apdu_command->state = ApduStatePayloadComplete;

    return 0;
}
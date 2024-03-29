#include "utils.h"

#include <stdbool.h>
#include <stdlib.h>

#include "cx.h"
#include "ethUtils.h"
#include "ui_api.h"
#include "os.h"

uint32_t set_result_get_publicKey(publicKeyContext_t *publicKeyContext) {
    uint32_t tx = 0;
    G_io_apdu_buffer[tx++] = 65;
    memmove(G_io_apdu_buffer + tx, publicKeyContext->publicKey.W, 65);
    tx += 65;
    G_io_apdu_buffer[tx++] = 40;
    memmove(G_io_apdu_buffer + tx, publicKeyContext->address, 40);
    tx += 40;
    return tx;
}

void get_public_key(publicKeyContext_t *publicKeyContext,
                    const uint32_t *derivationPath,
                    size_t pathLength) {
    cx_ecfp_private_key_t privateKey;

    get_private_key(&privateKey, derivationPath, pathLength);
    BEGIN_TRY {
        TRY {
            CX_THROW(cx_ecfp_generate_pair_no_throw(CX_CURVE_256K1,
                                                    &(publicKeyContext->publicKey),
                                                    &privateKey,
                                                    1));
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

    cx_sha3_t sha3_hash;
    getEthAddressStringFromKey(&(publicKeyContext->publicKey),
                               publicKeyContext->address,
                               &sha3_hash);
}

uint32_t readUint32BE(uint8_t *buffer) {
    return ((buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | (buffer[3]));
}

void get_private_key(cx_ecfp_private_key_t *privateKey,
                     const uint32_t *derivationPath,
                     size_t pathLength) {
    uint8_t privateKeyData[64];

    BEGIN_TRY {
        TRY {
            cx_err_t result_derive = os_derive_bip32_no_throw(CX_CURVE_256K1,
                                                              derivationPath,
                                                              pathLength,
                                                              privateKeyData,
                                                              NULL);
            if (result_derive != CX_OK) {
                THROW(result_derive);
            }

            io_seproxyhal_io_heartbeat();
            CX_THROW(cx_ecfp_init_private_key_no_throw(CX_CURVE_256K1,
                                                       privateKeyData,
                                                       PRIVATEKEY_LENGTH,
                                                       privateKey));
            io_seproxyhal_io_heartbeat();
        }
        CATCH_OTHER(e) {
            MEMCLEAR(privateKeyData);
            THROW(e);
        }
        FINALLY {
            MEMCLEAR(privateKeyData);
        }
    }
    END_TRY;
}

void get_private_key_with_seed(cx_ecfp_private_key_t *privateKey,
                               const uint32_t *derivationPath,
                               uint8_t pathLength) {
    uint8_t privateKeyData[64];
    BEGIN_TRY {
        TRY {
            cx_err_t result_derive =
                os_derive_bip32_with_seed_no_throw(HDW_ED25519_SLIP10,
                                                   CX_CURVE_Ed25519,
                                                   derivationPath,
                                                   pathLength,
                                                   privateKeyData,
                                                   NULL,
                                                   (unsigned char *) "ed25519 seed",
                                                   12);
            if (result_derive != CX_OK) {
                THROW(result_derive);
            }

            CX_THROW(cx_ecfp_init_private_key_no_throw(CX_CURVE_Ed25519,
                                                       privateKeyData,
                                                       PRIVATEKEY_LENGTH,
                                                       privateKey));
        }
        CATCH_OTHER(e) {
            MEMCLEAR(privateKeyData);
            THROW(e);
        }
        FINALLY {
            MEMCLEAR(privateKeyData);
        }
    }
    END_TRY;
}

int read_derivation_path(const uint8_t *data_buffer,
                         size_t data_size,
                         uint32_t *derivation_path,
                         uint32_t *derivation_path_length) {
    if (!data_buffer || !derivation_path || !derivation_path_length) {
        return ApduReplySdkInvalidParameter;
    }
    if (!data_size) {
        return ApduReplyKlaytnInvalidMessageSize;
    }
    const size_t len = data_buffer[0];
    data_buffer += 1;
    if (len < 1 || len > MAX_BIP32_PATH_LENGTH) {
        return ApduReplyKlaytnInvalidMessage;
    }
    if (1 + 4 * len > data_size) {
        return ApduReplyKlaytnInvalidMessageSize;
    }

    for (size_t i = 0; i < len; i++) {
        derivation_path[i] = ((data_buffer[0] << 24u) | (data_buffer[1] << 16u) |
                              (data_buffer[2] << 8u) | (data_buffer[3]));
        data_buffer += 4;
    }

    *derivation_path_length = len;
    return 0;
}

void sendResponse(uint8_t tx, bool approve) {
    G_io_apdu_buffer[tx++] = approve ? 0x90 : 0x69;
    G_io_apdu_buffer[tx++] = approve ? 0x00 : 0x85;
    // Send back the response, do not restart the event loop
    io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, tx);
    // Display back the original UX
    ui_idle();
}

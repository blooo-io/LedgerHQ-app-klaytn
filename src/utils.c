#include "os.h"
#include "cx.h"
#include <stdbool.h>
#include <stdlib.h>
#include "utils.h"
#include "menu.h"
#include "ethUtils.h"

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

    get_private_key(&privateKey, publicKeyContext, derivationPath, pathLength);
    BEGIN_TRY {
        TRY {
            cx_ecfp_generate_pair(CX_CURVE_256K1, &(publicKeyContext->publicKey), &privateKey, 1);
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
    uint64_t chainId = 8217;

    getEthAddressStringFromKey(&(publicKeyContext->publicKey),
                               publicKeyContext->address,
                               &sha3_hash,
                               chainId);
}

uint32_t readUint32BE(uint8_t *buffer) {
    return ((buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | (buffer[3]));
}

void get_private_key(cx_ecfp_private_key_t *privateKey,
                     publicKeyContext_t *publicKeyContext,
                     const uint32_t *derivationPath,
                     size_t pathLength) {
    uint8_t privateKeyData[PRIVATEKEY_LENGTH];

    BEGIN_TRY {
        TRY {
            os_perso_derive_node_bip32(CX_CURVE_256K1,
                                       derivationPath,
                                       pathLength,
                                       privateKeyData,
                                       NULL);
            io_seproxyhal_io_heartbeat();
            cx_ecfp_init_private_key(CX_CURVE_256K1, privateKeyData, PRIVATEKEY_LENGTH, privateKey);
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
    uint8_t privateKeyData[PRIVATEKEY_LENGTH];
    BEGIN_TRY {
        TRY {
            os_perso_derive_node_bip32_seed_key(HDW_ED25519_SLIP10,
                                                CX_CURVE_Ed25519,
                                                derivationPath,
                                                pathLength,
                                                privateKeyData,
                                                NULL,
                                                (unsigned char *) "ed25519 seed",
                                                12);
            cx_ecfp_init_private_key(CX_CURVE_Ed25519,
                                     privateKeyData,
                                     PRIVATEKEY_LENGTH,
                                     privateKey);
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
        return ApduReplySolanaInvalidMessageSize;
    }
    const size_t len = data_buffer[0];
    data_buffer += 1;
    if (len < 1 || len > MAX_BIP32_PATH_LENGTH) {
        return ApduReplySolanaInvalidMessage;
    }
    if (1 + 4 * len > data_size) {
        return ApduReplySolanaInvalidMessageSize;
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

unsigned int ui_prepro(const bagl_element_t *element) {
    unsigned int display = 1;
    if (element->component.userid > 0) {
        display = (ux_step == element->component.userid - 1);
        if (display) {
            if (element->component.userid == 1) {
                UX_CALLBACK_SET_INTERVAL(2000);
            } else {
                UX_CALLBACK_SET_INTERVAL(
                    MAX(3000, 1000 + bagl_label_roundtrip_duration_ms(element, 7)));
            }
        }
    }
    return display;
}

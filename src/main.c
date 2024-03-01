/*******************************************************************************
 *   Ledger Blue
 *   (c) 2016 Ledger
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



#ifdef HAVE_NBGL
#include "nbgl_touch.h"
#include "nbgl_page.h"
#endif  // HAVE_NBGL


#include "apdu.h"
#include "ethUstream.h"
#include "getPubkey.h"
#include "menu.h"
#include "shared_context.h"
#include "signLegacyTransaction.h"
#include "signMessage.h"
#include "signOffchainMessage.h"
#include "utils.h"

ApduCommand G_command;
txContext_t txContext;
tmpContent_t tmpContent;
strings_t strings;
cx_sha3_t global_sha3;

unsigned char G_io_seproxyhal_spi_buffer[IO_SEPROXYHAL_BUFFER_SIZE_B];

static void reset_main_globals(void) {
    MEMCLEAR(G_command);
    MEMCLEAR(G_io_seproxyhal_spi_buffer);
    MEMCLEAR(txContext);
}

void handleApdu(volatile unsigned int *flags, volatile unsigned int *tx, int rx) {
    if (!flags || !tx) {
        THROW(ApduReplySdkInvalidParameter);
    }

    if (rx < 0) {
        THROW(ApduReplySdkExceptionIoOverflow);
    }

    int ret = apdu_handle_message(G_io_apdu_buffer, rx, &G_command);
    if (ret != 0) {
        THROW(ret);
    }
    if (G_command.state == ApduStatePayloadInProgress) {
        THROW(ApduReplySuccess);
    }

    switch (G_command.instruction) {
        case InsDeprecatedGetAppConfiguration:
        case InsGetAppConfiguration:
            G_io_apdu_buffer[0] = N_storage.settings.allow_blind_sign;
            G_io_apdu_buffer[1] = MAJOR_VERSION;
            G_io_apdu_buffer[2] = MINOR_VERSION;
            G_io_apdu_buffer[3] = PATCH_VERSION;
            *tx = 4;
            THROW(ApduReplySuccess);

        case InsDeprecatedGetPubkey:
        case InsGetPubkey:
            handle_get_pubkey(flags, tx);
            break;
        case InsSignLegacyTransaction:
        case InsSignValueTransfer:
        case InsSignValueTransferMemo:
        case InsSignSmartContractDeploy:
        case InsSignSmartContractExecution:
        case InsSignCancel:
            handle_sign_legacy_transaction(tx);
            handle_sign_legacy_transaction_ui(flags);
            break;
        case InsDeprecatedSignMessage:
        case InsSignMessage:
            handle_sign_message_parse_message(tx);
            handle_sign_message_ui(flags);
            break;

        case InsSignOffchainMessage:
            handle_sign_offchain_message(flags, tx);
            break;

        default:
            THROW(ApduReplyUnimplementedInstruction);
    }
}

void app_main(void) {
    volatile unsigned int rx = 0;
    volatile unsigned int tx = 0;
    volatile unsigned int flags = 0;

    // Stores the information about the current command. Some commands expect
    // multiple APDUs before they become complete and executed.
    reset_getpubkey_globals();
    reset_main_globals();

    // DESIGN NOTE: the bootloader ignores the way APDU are fetched. The only
    // goal is to retrieve APDU.
    // When APDU are to be fetched from multiple IOs, like NFC+USB+BLE, make
    // sure the io_event is called with a
    // switch event, before the apdu is replied to the bootloader. This avoid
    // APDU injection faults.
    for (;;) {
        volatile unsigned short sw = 0;

        BEGIN_TRY {
            TRY {
                rx = tx;
                tx = 0;  // ensure no race in catch_other if io_exchange throws
                         // an error
                rx = io_exchange(CHANNEL_APDU | flags, rx);
                flags = 0;

                // no apdu received, well, reset the session, and reset the
                // bootloader configuration
                if (rx == 0) {
                    THROW(ApduReplyNoApduReceived);
                }

                PRINTF("New APDU received:\n%.*H\n", rx, G_io_apdu_buffer);

                handleApdu(&flags, &tx, rx);
            }
            CATCH(ApduReplySdkExceptionIoReset) {
                THROW(ApduReplySdkExceptionIoReset);
            }
            CATCH_OTHER(e) {
                switch (e & 0xF000) {
                    case 0x6000:
                        sw = e;
                        break;
                    case 0x9000:
                        // All is well
                        sw = e;
                        break;
                    default:
                        // Internal error
                        sw = 0x6800 | (e & 0x7FF);
                        break;
                }
                if (e != 0x9000) {
                    flags &= ~IO_ASYNCH_REPLY;
                }
                // Unexpected exception => report
                G_io_apdu_buffer[tx] = sw >> 8;
                G_io_apdu_buffer[tx + 1] = sw;
                tx += 2;
            }
            FINALLY {
            }
        }
        END_TRY;
    }
}

#ifdef HAVE_BAGL
// override point, but nothing more to do
void io_seproxyhal_display(const bagl_element_t *element) {
    io_seproxyhal_display_default(element);
}
#endif  // HAVE_BAGL

uint8_t io_event(uint8_t channel) {
    (void) channel;

    switch (G_io_seproxyhal_spi_buffer[0]) {
        case SEPROXYHAL_TAG_BUTTON_PUSH_EVENT:
#ifdef HAVE_BAGL
            UX_BUTTON_PUSH_EVENT(G_io_seproxyhal_spi_buffer);
#endif  // HAVE_BAGL
            break;
        case SEPROXYHAL_TAG_STATUS_EVENT:
            if (G_io_apdu_media == IO_APDU_MEDIA_USB_HID &&  //
                !(U4BE(G_io_seproxyhal_spi_buffer, 3) &      //
                  SEPROXYHAL_TAG_STATUS_EVENT_FLAG_USB_POWERED)) {
                THROW(ApduReplySdkExceptionIoReset);
            }
            /* fallthrough */
            __attribute__((fallthrough));
        case SEPROXYHAL_TAG_DISPLAY_PROCESSED_EVENT:
#ifdef HAVE_BAGL
            UX_DISPLAYED_EVENT({});
#endif  // HAVE_BAGL
#ifdef HAVE_NBGL
            UX_DEFAULT_EVENT();
#endif  // HAVE_NBGL
            break;
#ifdef HAVE_NBGL
        case SEPROXYHAL_TAG_FINGER_EVENT:
            UX_FINGER_EVENT(G_io_seproxyhal_spi_buffer);
            break;
#endif  // HAVE_NBGL
        case SEPROXYHAL_TAG_TICKER_EVENT:
            UX_TICKER_EVENT(G_io_seproxyhal_spi_buffer, {});
            break;
        default:
            UX_DEFAULT_EVENT();
            break;
    }

    if (!io_seproxyhal_spi_is_status_sent()) {
        io_seproxyhal_general_status();
    }

    return 1;
}

unsigned short io_exchange_al(unsigned char channel, unsigned short tx_len) {
    switch (channel & ~(IO_FLAGS)) {
        case CHANNEL_KEYBOARD:
            break;

        // multiplexed io exchange over a SPI channel and
        // TLV encapsulated protocol
        case CHANNEL_SPI:
            if (tx_len) {
                io_seproxyhal_spi_send(G_io_apdu_buffer, tx_len);

                if (channel & IO_RESET_AFTER_REPLIED) {
                    reset();
                }
                return 0;  // nothing received from the master so far
                           // (it's a tx transaction)
            } else {
                return io_seproxyhal_spi_recv(G_io_apdu_buffer, sizeof(G_io_apdu_buffer), 0);
            }

        default:
            THROW(ApduReplySdkInvalidParameter);
    }
    return 0;
}

void app_exit(void) {
    BEGIN_TRY_L(exit) {
        TRY_L(exit) {
            os_sched_exit(-1);
        }
        FINALLY_L(exit) {
        }
    }
    END_TRY_L(exit);
}

void nv_app_state_init() {
    if (N_storage.initialized != 0x01) {
        internalStorage_t storage;
        storage.settings.allow_blind_sign = BlindSignDisabled;
#if defined(TARGET_NANOX) || defined(TARGET_NANOSP) || defined(TARGET_STAX)
        storage.settings.pubkey_display = PubkeyDisplayLong;
#else
        storage.settings.pubkey_display = PubkeyDisplayShort;
#endif
        storage.settings.display_mode = DisplayModeUser;
        storage.initialized = 0x01;
        nvm_write((void *) &N_storage, (void *) &storage, sizeof(internalStorage_t));
    }
}

__attribute__((section(".boot"))) int main(void) {
    // exit critical section
    __asm volatile("cpsie i");

    // ensure exception will work as planned
    os_boot();

    for (;;) {
        UX_INIT();

        BEGIN_TRY {
            TRY {
                io_seproxyhal_init();

                nv_app_state_init();

                USB_power(0);
                USB_power(1);

#ifdef HAVE_BLE
                // Grab the current plane mode setting. os_settings_get() is enabled by
                // appFlags bit #9 set to 1 in Makefile (i.e. "--appFlags 0x2xx")
                G_io_app.plane_mode = os_setting_get(OS_SETTING_PLANEMODE, NULL, 0);
#endif  // HAVE_BLE

                ui_idle();

#ifdef HAVE_BLE
                BLE_power(0, NULL);
                BLE_power(1, "Nano X");
#endif  // HAVE_BLE

                app_main();
            }
            CATCH(ApduReplySdkExceptionIoReset) {
                // reset IO and UX before continuing
                continue;
            }
            CATCH_ALL {
                break;
            }
            FINALLY {
            }
        }
        END_TRY;
    }
    app_exit();
    return 0;
}

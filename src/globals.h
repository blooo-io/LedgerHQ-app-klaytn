#include "os.h"
#include "ux.h"
#include "os_io_seproxyhal.h"

#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#define CLA 0xE0

// header offsets
#define OFFSET_CLA              0
#define OFFSET_INS              1
#define OFFSET_P1               2
#define OFFSET_P2               3
#define OFFSET_LC               4
#define OFFSET_CDATA            5
#define DEPRECATED_OFFSET_CDATA 6

#define P1_CONFIRM     0x01
#define P1_NON_CONFIRM 0x00

#define P1_FIRST 0x00
#define P1_MORE  0x80

#define P2_NO_CHAINCODE 0x00
#define P2_CHAINCODE    0x01

#define P2_EXTEND 0x01
#define P2_MORE   0x02

#define ROUND_TO_NEXT(x, next) (((x) == 0) ? 0 : ((((x - 1) / (next)) + 1) * (next)))

/* See constant by same name in sdk/src/packet.rs */
#define PACKET_DATA_SIZE (1280 - 40 - 8)

#define MAX_BIP32_PATH_LENGTH             5
#define MAX_DERIVATION_PATH_BUFFER_LENGTH (1 + MAX_BIP32_PATH_LENGTH * 4)
#define TOTAL_SIGN_MESSAGE_BUFFER_LENGTH  (PACKET_DATA_SIZE + MAX_DERIVATION_PATH_BUFFER_LENGTH)

#define MAX_MESSAGE_LENGTH ROUND_TO_NEXT(TOTAL_SIGN_MESSAGE_BUFFER_LENGTH, USB_SEGMENT_SIZE)
#define SIGNATURE_LENGTH   64
#define HASH_LENGTH        32
#define PUBKEY_LENGTH      32
#define PRIVATEKEY_LENGTH  32
#define CHAINCODE_LENGTH   32
#define ETH_PUBKEY_LENGTH  40

#define MAX_OFFCHAIN_MESSAGE_LENGTH    (MAX_MESSAGE_LENGTH - 1 > 1212 ? 1212 : MAX_MESSAGE_LENGTH - 1)
#define OFFCHAIN_MESSAGE_HEADER_LENGTH 20

typedef enum InstructionCode {
    InsGetAppConfiguration = 0x01,
    InsGetPubkey = 0x02,
    InsSignLegacyTransaction = 0x04,
    InsSignValueTransfer = 0x08,
    InsSignValueTransferMemo = 0x10,
    InsSignSmartContractDeploy = 0x28,
    // DEPRECATED - Use non "16" suffixed variants below
    InsDeprecatedGetAppConfiguration = 0xF1,
    InsDeprecatedGetPubkey = 0xF2,
    InsDeprecatedSignMessage = 0xF3,
    // END DEPRECATED
    // InsGetAppConfiguration = 0xF4,
    // InsGetPubkey = 0xF5,
    InsSignMessage = 0xF6,
    InsSignOffchainMessage = 0xF7
} InstructionCode;

// display stepped screens
extern unsigned int ux_step;
extern unsigned int ux_step_count;

enum BlindSign {
    BlindSignDisabled = 0,
    BlindSignEnabled = 1,
};

enum PubkeyDisplay {
    PubkeyDisplayLong = 0,
    PubkeyDisplayShort = 1,
};

enum DisplayMode {
    DisplayModeUser = 0,
    DisplayModeExpert = 1,
};

typedef struct AppSettings {
    uint8_t allow_blind_sign;
    uint8_t pubkey_display;
    uint8_t display_mode;
} AppSettings;

typedef struct internalStorage_t {
    AppSettings settings;
    uint8_t initialized;
} internalStorage_t;

extern const internalStorage_t N_storage_real;
#define N_storage (*(volatile internalStorage_t *) PIC(&N_storage_real))
#endif
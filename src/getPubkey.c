#include "apdu.h"
#include "getPubkey.h"
#include "os.h"
#include "ux.h"
#include "utils.h"
#include "printer.h"

publicKeyContext_t G_publicKey;

void reset_getpubkey_globals(void) {
    MEMCLEAR(G_publicKey);
}

//////////////////////////////////////////////////////////////////////
#ifdef HAVE_BAGL

UX_STEP_NOCB(ux_display_public_flow_5_step,
             bnnn_paging,
             {
                 .title = "Pubkey",
                 .text = G_publicKey.address,
             });
UX_STEP_CB(ux_display_public_flow_6_step,
           pb,
           sendResponse(set_result_get_publicKey(&G_publicKey), true),
           {
               &C_icon_validate_14,
               "Approve",
           });
UX_STEP_CB(ux_display_public_flow_7_step,
           pb,
           sendResponse(0, false),
           {
               &C_icon_crossmark,
               "Reject",
           });

UX_FLOW(ux_display_public_flow,
        &ux_display_public_flow_5_step,
        &ux_display_public_flow_6_step,
        &ux_display_public_flow_7_step);

#endif  // HAVE_BAGL

void handle_get_pubkey(volatile unsigned int *flags, volatile unsigned int *tx) {
    if (!flags || !tx ||
        (G_command.instruction != InsDeprecatedGetPubkey &&
         G_command.instruction != InsGetPubkey) ||
        G_command.state != ApduStatePayloadComplete) {
        THROW(ApduReplySdkInvalidParameter);
    }

    get_public_key(&G_publicKey, G_command.derivation_path, G_command.derivation_path_length);

    if (G_command.non_confirm) {
        *tx = set_result_get_publicKey(&G_publicKey);
        THROW(ApduReplySuccess);
    } else {
#ifdef HAVE_BAGL
        ux_flow_init(0, ux_display_public_flow, NULL);
#endif  // HAVE_BAGL
        *flags |= IO_ASYNCH_REPLY;
    }
}

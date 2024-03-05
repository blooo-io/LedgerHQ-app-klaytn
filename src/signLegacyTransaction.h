#include "os.h"
#include "cx.h"
#include "globals.h"

#ifndef _SIGN_LEGACY_H_
#define _SIGN_LEGACY_H_

void handle_sign_legacy_transaction(volatile unsigned int *tx, tmpContent_t *tmpContent);
void handle_sign_legacy_transaction_ui(volatile unsigned int *flags);

#endif

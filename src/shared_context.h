#ifndef _SHARED_CONTEXT_H_
#define _SHARED_CONTEXT_H_

#include <stdbool.h>
#include <stdint.h>
#include "ethUstream.h"
#include "apdu.h"

extern ApduCommand G_command;
extern txInt256_t chainID;
extern tmpContent_t tmpContent;

#endif  // _SHARED_CONTEXT_H_

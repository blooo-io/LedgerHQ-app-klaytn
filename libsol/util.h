#pragma once
#include <string.h>
#include <stdio.h>

#define ARRAY_LEN(a) (sizeof(a) / sizeof((a)[0]))
#define BAIL_IF(x)           \
    do {                     \
        int err = x;         \
        if (err) return err; \
    } while (0)
#define MIN(a, b) ((a) < (b) ? (a) : (b));

#define assert_string_equal(actual, expected) assert(strcmp(actual, expected) == 0)

#define assert_pubkey_equal(actual, expected) assert(memcmp(actual, expected, 32) == 0)

#ifndef UNUSED
#define UNUSED(x) (void) x
#endif

static inline void printf_hex_array(const char *title __attribute__((unused)),
                                    size_t len __attribute__((unused)),
                                    const uint8_t *data __attribute__((unused))) {
    printf("%s", title);
    for (size_t i = 0; i < len; ++i) {
        printf("%02x", data[i]);
    };
    printf("\n");
}

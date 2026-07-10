#include "jwt.h"
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

static char *base64url_encode(const unsigned char *input, int len) {
    int b64_len = 4 * ((len + 2) / 3);
    char *b64 = malloc(b64_len + 1);
    if (!b64) return NULL;

    EVP_EncodeBlock((unsigned char *)b64, input, len);

    for (int i = 0; i < b64_len; i++) {
        if (b64[i] == '+') {
            b64[i] = '-';
        } else if (b64[i] == '/') {
            b64[i] = '_';
        } else if (b64[i] == '=') {
            b64[i] = '\0';
            break;
        }
    }

    return b64;
}

static unsigned char *hmac_sign(const char *key, int key_len,
                                 const unsigned char *data, int data_len,
                                 unsigned int *out_len) {
    unsigned char *result = malloc(EVP_MAX_MD_SIZE);
    if (!result) return NULL;

    HMAC(EVP_sha256(), key, key_len, data, data_len, result, out_len);
    return result;
}

char *jwt_create(int64_t user_id, const char *username, const char *secret) {
    const char *header = "{\"alg\":\"HS256\",\"typ\":\"JWT\"}";
    char *header_b64 = base64url_encode((const unsigned char *)header, strlen(header));
    if (!header_b64) return NULL;

    time_t now = time(NULL);
    time_t exp = now + 86400 * 7;

    char payload[256];
    int payload_len = snprintf(payload, sizeof(payload),
        "{\"user_id\":%ld,\"username\":\"%s\",\"exp\":%ld,\"iat\":%ld}",
        (long)user_id, username, (long)exp, (long)now);

    char *payload_b64 = base64url_encode((const unsigned char *)payload, payload_len);
    if (!payload_b64) {
        free(header_b64);
        return NULL;
    }

    char signing_input[512];
    int si_len = snprintf(signing_input, sizeof(signing_input),
        "%s.%s", header_b64, payload_b64);

    unsigned int sig_len;
    unsigned char *sig = hmac_sign(secret, strlen(secret),
                                    (const unsigned char *)signing_input, si_len,
                                    &sig_len);
    if (!sig) {
        free(header_b64);
        free(payload_b64);
        return NULL;
    }

    char *sig_b64 = base64url_encode(sig, sig_len);
    free(sig);

    if (!sig_b64) {
        free(header_b64);
        free(payload_b64);
        return NULL;
    }

    char *token = malloc(si_len + 1 + strlen(sig_b64) + 1);
    if (!token) {
        free(header_b64);
        free(payload_b64);
        free(sig_b64);
        return NULL;
    }

    sprintf(token, "%s.%s", signing_input, sig_b64);

    free(header_b64);
    free(payload_b64);
    free(sig_b64);

    return token;
}

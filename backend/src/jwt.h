#ifndef JWT_H
#define JWT_H

#include <stdint.h>

char *jwt_create(int64_t user_id, const char *username, const char *secret);
int jwt_validate(const char *token, const char *secret, int64_t *user_id);

#endif

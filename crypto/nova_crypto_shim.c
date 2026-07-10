// nova_crypto_shim.c — тонкая обёртка над OpenSSL (libcrypto), даёт Nova-у
// простой строка-в-строку-из API вместо возни с байтовыми буферами напрямую.
//
// Собрать: gcc -shared -o nova_crypto_shim.dll nova_crypto_shim.c
//          -I <ucrt64>/include -L <ucrt64>/lib -lcrypto -Wl,--out-implib,libnova_crypto_shim.a

#include <openssl/md5.h>
#include <openssl/sha.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char* to_hex(const unsigned char* digest, int len) {
    char* out = malloc(len * 2 + 1);
    for (int i = 0; i < len; i++) {
        sprintf(out + i * 2, "%02x", digest[i]);
    }
    out[len * 2] = 0;
    return out;
}

__declspec(dllexport) const char* nova_md5_hex(const char* input) {
    unsigned char digest[MD5_DIGEST_LENGTH];
    MD5((const unsigned char*)input, strlen(input), digest);
    return to_hex(digest, MD5_DIGEST_LENGTH);
}

__declspec(dllexport) const char* nova_sha1_hex(const char* input) {
    unsigned char digest[SHA_DIGEST_LENGTH];
    SHA1((const unsigned char*)input, strlen(input), digest);
    return to_hex(digest, SHA_DIGEST_LENGTH);
}

__declspec(dllexport) const char* nova_sha256_hex(const char* input) {
    unsigned char digest[SHA256_DIGEST_LENGTH];
    SHA256((const unsigned char*)input, strlen(input), digest);
    return to_hex(digest, SHA256_DIGEST_LENGTH);
}

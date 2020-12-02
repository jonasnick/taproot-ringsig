#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include <assert.h>

#include <secp256k1.h>
#include <secp256k1_rangeproof.h>

#define MIN_AMOUNT 5000

uint32_t to_uint32(unsigned char *bytes) {
    return (bytes[0] << 24) + (bytes[1] << 16) + (bytes[2] << 8) + bytes[3];
}

int read_utxo_metadata(uint64_t *coins_count, FILE *utxo_dump) {
    unsigned char buffer[256];
    size_t ret;
    size_t size;

    size = 32;
    ret = fread(buffer, 1, size, utxo_dump);
    if (ret != size) {
        return 0;
    }
    size = 8;
    ret = fread(buffer, 1, size, utxo_dump);
    if (ret != size) {
        return 0;
    }
    memcpy(coins_count, buffer, 8);
    /* Skip nChainTx */
    size = 4;
    ret = fread(buffer, 1, size, utxo_dump);
    if (ret != size) {
        return 0;
    }
    return 1;
}

uint64_t read_varint(FILE* utxo_dump)
{
    uint64_t n = 0;
    while(1) {
        unsigned char chData;
        assert(fread(&chData, 1, 1, utxo_dump) == 1);
        assert(n <= (ULONG_MAX >> 7));
        n = (n << 7) | (chData & 0x7F);
        if (chData & 0x80) {
            assert(n < ULONG_MAX);
            n++;
        } else {
            return n;
        }
    }
}

void print_coutpoint(unsigned char *buffer) {
    uint32_t n;
    for (int i = 31; i >= 0; i--) {
        printf("%02X", buffer[i]);
    }
    printf(":");
    memcpy(&n, &buffer[32], 4);
    printf("%d\n", n);
}

unsigned int GetSpecialScriptSize(unsigned int nSize)
{
    if (nSize == 0 || nSize == 1)
        return 20;
    if (nSize == 2 || nSize == 3 || nSize == 4 || nSize == 5)
        return 32;
    return 0;
}

uint64_t DecompressAmount(uint64_t x)
{
    // x = 0  OR  x = 1+10*(9*n + d - 1) + e  OR  x = 1+10*(n - 1) + 9
    if (x == 0)
        return 0;
    x--;
    // x = 10*(9*n + d - 1) + e
    int e = x % 10;
    x /= 10;
    uint64_t n = 0;
    if (e < 9) {
        // x = 9*n + d - 1
        int d = (x % 9) + 1;
        x /= 9;
        // x = n
        n = x*10 + d;
    } else {
        n = x+1;
    }
    while (e) {
        n *= 10;
        e--;
    }
    return n;
}


void help(char** argv) {
    printf("%s keygen\n", argv[0]);
    printf("%s prove <dumputxo_file> <seckey>\n", argv[0]);
}

/* TODO: */
static const unsigned char seckey[32] = { 1, 2, 3, 0 };

int keygen() {
    secp256k1_pubkey pubkey;
    unsigned char pubkey_ser[33];
    size_t output_len = sizeof(pubkey_ser);
    secp256k1_context *ctx;

    ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN);

    if (!secp256k1_ec_pubkey_create(ctx, &pubkey, seckey)) {
        secp256k1_context_destroy(ctx);
        return 0;
    }

    assert(secp256k1_ec_pubkey_serialize(ctx, pubkey_ser, &output_len, &pubkey, SECP256K1_EC_COMPRESSED));

    printf("pubkey: ");
    for (size_t i = 0; i < output_len; i++) {
        printf("%02X", pubkey_ser[i]);
    }
    printf("\n");
    secp256k1_context_destroy(ctx);
    return 1;
}

#define MAX_KEYS 100
int read_utxos(secp256k1_context *ctx, secp256k1_pubkey *taproot_keys, size_t *n_taproot_keys, FILE* utxo_dump, uint64_t coins_count) {
    unsigned char buffer[1024];
    *n_taproot_keys = 0;

    for (uint64_t i = 0; i < coins_count; i++) {
        /* skip outpoint */
        size_t size = 32 + 4;
        size_t ret = fread(buffer, 1, size, utxo_dump);
        if (ret != size) {
            return 0;
        }
        /* print_coutpoint(buffer); */

        uint32_t code = (uint32_t)read_varint(utxo_dump);
        uint64_t amount = DecompressAmount(read_varint(utxo_dump));
        uint64_t nSize = read_varint(utxo_dump);
        static const unsigned int nSpecialScripts = 6;

        if (nSize < nSpecialScripts) {
            size = GetSpecialScriptSize(nSize);
        } else {
            size = nSize - nSpecialScripts;
        }

        /* printf("height: %d, amount %lu, size %lu, ", code/2, amount, size); */
        assert(size < sizeof(buffer));
        ret = fread(buffer, 1, size, utxo_dump);
        if (ret != size) {
            return 0;
        }
        if (size == 34 && buffer[0] == 0x51) {
            buffer[1] = 0x02;
            printf("pubkey: ");
            for (size_t j = 0; j < size; j++) {
                printf("%02X", buffer[j]);
            }
            printf("\n");
            assert(*n_taproot_keys < MAX_KEYS);
            assert(secp256k1_ec_pubkey_parse(ctx, &taproot_keys[*n_taproot_keys], &buffer[1], 33));
            (*n_taproot_keys)++;
        }
    }
    return 1;
}

int get_taproot_keys(secp256k1_context *ctx, secp256k1_pubkey *pubkeys, size_t *n_pubkeys, const char *filename) {
    FILE *utxo_dump;
    uint64_t coins_count;

    utxo_dump = fopen(filename, "rb");
    if (utxo_dump == NULL) {
        fprintf(stderr, "couldn't open file\n");
        return 0;
    }

    if (!read_utxo_metadata(&coins_count, utxo_dump)) {
        fprintf(stderr, "error reading metadata\n");
        return 0;
    }
    if (!read_utxos(ctx, pubkeys, n_pubkeys, utxo_dump, coins_count)) {
        fprintf(stderr, "error reading utxos\n");
        return 0;
    }
    fclose(utxo_dump);
    return 1;
}

int prove(int argc, char **argv) {
    secp256k1_pubkey pubkeys[MAX_KEYS];
    size_t n_pubkeys;
    secp256k1_pubkey my_pubkey;
    secp256k1_context *ctx;

    if (argc < 3) {
        help(argv);
        return 0;
    }

    ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    if (!get_taproot_keys(ctx, pubkeys, &n_pubkeys, argv[2])) {
        return 0;
    }

    /* Find position of own pubkey */
    if (!secp256k1_ec_pubkey_create(ctx, &my_pubkey, seckey)) {
        secp256k1_context_destroy(ctx);
        return 0;
    }
    size_t pos = SIZE_MAX;
    for (int i = 0; i < n_pubkeys; i++) {
        if (memcmp(&my_pubkey, &pubkeys[i], sizeof(my_pubkey)) == 0) {
            pos = i;
        }
    }
    if (pos == SIZE_MAX) {
        fprintf(stderr, "couldn't find own pubkey\n");
        secp256k1_context_destroy(ctx);
        return 0;
    }
    printf("pos %lu\n", pos);

    secp256k1_scratch_space *scratch = secp256k1_scratch_space_create(ctx, 8 * 1024);
    unsigned char sig[1024];
    size_t sig_len = sizeof(sig);
    unsigned char msg[32] = { 0 };
    if (!secp256k1_ring_sign(ctx, scratch, sig, &sig_len, NULL, pubkeys, n_pubkeys, seckey, pos, NULL, msg)) {
        fprintf(stderr, "signing failed\n");
        secp256k1_scratch_space_destroy(ctx, scratch);
        secp256k1_context_destroy(ctx);
        return 0;
    }
    for (size_t i = 0; i < sig_len; i++) {
        printf("%02X", sig[i]);
    }
    printf("\n");

    secp256k1_scratch_space_destroy(ctx, scratch);
    secp256k1_context_destroy(ctx);
    return 1;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        help(argv);
        return 1;
    }

    if (strcmp(argv[1], "keygen") == 0) {
        return !keygen();
    } else if (strcmp(argv[1], "prove") == 0){
        return !prove(argc, argv);
    } else {
        help(argv);
        return 1;
    }
    return 0;
}

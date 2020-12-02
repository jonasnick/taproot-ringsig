#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include <assert.h>

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
        fread(&chData, 1, 1, utxo_dump);
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

int read_utxos(FILE* utxo_dump, uint64_t coins_count) {
    unsigned char buffer[1024];
    for (uint64_t i = 0; i < coins_count; i++) {
        /* skip outpoint */
        size_t size = 32 + 4;
        size_t ret = fread(buffer, 1, size, utxo_dump);
        if (ret != size) {
            return 0;
        }
        print_coutpoint(buffer);

        uint32_t code = (uint32_t)read_varint(utxo_dump);
        uint64_t amount = DecompressAmount(read_varint(utxo_dump));
        uint64_t nSize = read_varint(utxo_dump);
        static const unsigned int nSpecialScripts = 6;

        if (nSize < nSpecialScripts) {
            size = GetSpecialScriptSize(nSize);
        } else {
            size = nSize - nSpecialScripts;
        }

        printf("height: %d, amount %lu, size %lu, ", code/2, amount, size);
        assert(size < sizeof(buffer));
        ret = fread(buffer, 1, size, utxo_dump);
        if (ret != size) {
            return 0;
        }
        printf("pubkey: ");
        for (size_t j = 0; j < size; j++) {
            printf("%02X", buffer[j]);
        }
        printf("\n");
    }
    return 1;
}

void help(char** argv) {
    printf("%s keygen\n", argv[0]);
    printf("%s prove <dumputxo_file> <seckey>\n", argv[0]);
}

int keygen() {
    return 1;
}

int prove(int argc, char **argv) {
    FILE *utxo_dump;
    uint64_t coins_count;

    if (argc < 3) {
        help(argv);
        return 0;
    }

    utxo_dump = fopen(argv[1], "rb");
    if (utxo_dump == NULL) {
        fprintf(stderr, "couldn't open file\n");
        return 0;
    }

    if (!read_utxo_metadata(&coins_count, utxo_dump)) {
        fprintf(stderr, "error reading metadata\n");
        return 0;
    }
    if (!read_utxos(utxo_dump, coins_count)) {
        fprintf(stderr, "error reading utxos\n");
        return 0;
    }
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

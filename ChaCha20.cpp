/*
ChaCha20.c
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/* ---------- ChaCha20 core ---------- */

#define ROTL32(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

static void quarter_round(uint32_t *a, uint32_t *b, uint32_t *c, uint32_t *d) {
    *a += *b; *d ^= *a; *d = ROTL32(*d, 16);
    *c += *d; *b ^= *c; *b = ROTL32(*b, 12);
    *a += *b; *d ^= *a; *d = ROTL32(*d, 8);
    *c += *d; *b ^= *c; *b = ROTL32(*b, 7);
}

/* Generates one 64-byte keystream block for the given 256-bit key,
 * 32-bit block counter, and 96-bit nonce. */
static void chacha20_block(const uint32_t key[8], uint32_t counter,
                            const uint32_t nonce[3], uint8_t out[64]) {
    static const uint32_t constants[4] = {
        0x61707865, 0x3320646e, 0x79622d32, 0x6b206574 /* "expand 32-byte k" */
    };

    uint32_t state[16];
    state[0] = constants[0];
    state[1] = constants[1];
    state[2] = constants[2];
    state[3] = constants[3];
    for (int i = 0; i < 8; i++) state[4 + i] = key[i];
    state[12] = counter;
    state[13] = nonce[0];
    state[14] = nonce[1];
    state[15] = nonce[2];

    uint32_t working[16];
    memcpy(working, state, sizeof(working));

    /* 20 rounds = 10 iterations of (column round + diagonal round) */
    for (int i = 0; i < 10; i++) {
        /* Column rounds */
        quarter_round(&working[0], &working[4], &working[8],  &working[12]);
        quarter_round(&working[1], &working[5], &working[9],  &working[13]);
        quarter_round(&working[2], &working[6], &working[10], &working[14]);
        quarter_round(&working[3], &working[7], &working[11], &working[15]);
        /* Diagonal rounds */
        quarter_round(&working[0], &working[5], &working[10], &working[15]);
        quarter_round(&working[1], &working[6], &working[11], &working[12]);
        quarter_round(&working[2], &working[7], &working[8],  &working[13]);
        quarter_round(&working[3], &working[4], &working[9],  &working[14]);
    }

    for (int i = 0; i < 16; i++) {
        working[i] += state[i];
        out[4 * i + 0] = (uint8_t)(working[i] >> 0);
        out[4 * i + 1] = (uint8_t)(working[i] >> 8);
        out[4 * i + 2] = (uint8_t)(working[i] >> 16);
        out[4 * i + 3] = (uint8_t)(working[i] >> 24);
    }
}

/* Encrypts (or decrypts, since XOR is its own inverse) `len` bytes from
 * `in` into `out` using the given key, initial counter, and nonce. */
void chacha20_xor(const uint8_t key_bytes[32], uint32_t counter,
                   const uint8_t nonce_bytes[12],
                   const uint8_t *in, uint8_t *out, size_t len) {
    uint32_t key[8], nonce[3];
    for (int i = 0; i < 8; i++)
        key[i] = (uint32_t)key_bytes[4*i]       |
                 ((uint32_t)key_bytes[4*i+1] << 8)  |
                 ((uint32_t)key_bytes[4*i+2] << 16) |
                 ((uint32_t)key_bytes[4*i+3] << 24);
    for (int i = 0; i < 3; i++)
        nonce[i] = (uint32_t)nonce_bytes[4*i]       |
                   ((uint32_t)nonce_bytes[4*i+1] << 8)  |
                   ((uint32_t)nonce_bytes[4*i+2] << 16) |
                   ((uint32_t)nonce_bytes[4*i+3] << 24);

    uint8_t block[64];
    size_t offset = 0;
    while (offset < len) {
        chacha20_block(key, counter, nonce, block);
        counter++;

        size_t chunk = (len - offset < 64) ? (len - offset) : 64;
        for (size_t i = 0; i < chunk; i++)
            out[offset + i] = in[offset + i] ^ block[i];

        offset += chunk;
    }
}

/* ---------- Cycle-accurate timing ---------- */

#if defined(__x86_64__) || defined(__i386__)
static inline uint64_t rdtsc(void) {
    uint32_t lo, hi;
    __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
}
#define HAVE_RDTSC 1
#else
#define HAVE_RDTSC 0
#endif

/* ---------- Test vector from RFC 8439, Section 2.4.2 ---------- */

static void print_hex(const char *label, const uint8_t *data, size_t len) {
    printf("%s: ", label);
    for (size_t i = 0; i < len; i++) printf("%02x", data[i]);
    printf("\n");
}

int main(void) {
    /* RFC 8439 test vector */
    uint8_t key[32] = {
        0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
        0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
        0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
        0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f
    };
    uint8_t nonce[12] = {
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x4a,
        0x00,0x00,0x00,0x00
    };
    uint32_t counter = 1;

    const char *plaintext =
        "Ladies and Gentlemen of the class of '99: "
        "If I could offer you only one tip for the future, "
        "sunscreen would be it.";
    size_t len = strlen(plaintext);

    uint8_t ciphertext[256];
    uint8_t decrypted[256];

    chacha20_xor(key, counter, nonce, (const uint8_t *)plaintext, ciphertext, len);
    chacha20_xor(key, counter, nonce, ciphertext, decrypted, len);

    printf("Plaintext : %s\n", plaintext);
    print_hex("Ciphertext", ciphertext, len);
    printf("Decrypted : %.*s\n", (int)len, decrypted);
    printf("Round-trip check: %s\n\n",
           memcmp(plaintext, decrypted, len) == 0 ? "PASSED" : "FAILED");

    /* ---------- Benchmark ---------- */

    #define BUF_SIZE (1 << 20)   /* 1 MiB */
    #define ITERATIONS 100

    static uint8_t buf_in[BUF_SIZE];
    static uint8_t buf_out[BUF_SIZE];
    for (size_t i = 0; i < BUF_SIZE; i++) buf_in[i] = (uint8_t)i;

#if HAVE_RDTSC
    /* Warm-up */
    chacha20_xor(key, 0, nonce, buf_in, buf_out, BUF_SIZE);

    uint64_t best = UINT64_MAX;
    for (int iter = 0; iter < ITERATIONS; iter++) {
        uint64_t start = rdtsc();
        chacha20_xor(key, 0, nonce, buf_in, buf_out, BUF_SIZE);
        uint64_t end = rdtsc();
        uint64_t cycles = end - start;
        if (cycles < best) best = cycles;
    }

    printf("Benchmark: encrypted %d bytes, best of %d runs (RDTSC)\n",
           BUF_SIZE, ITERATIONS);
    printf("Total cycles      : %llu\n", (unsigned long long)best);
    printf("Cycles per byte   : %.3f\n", (double)best / BUF_SIZE);
#else
    clock_t start = clock();
    for (int iter = 0; iter < ITERATIONS; iter++)
        chacha20_xor(key, 0, nonce, buf_in, buf_out, BUF_SIZE);
    clock_t end = clock();

    double seconds = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Benchmark: encrypted %d bytes x %d iterations (clock())\n",
           BUF_SIZE, ITERATIONS);
    printf("Total time        : %.6f s\n", seconds);
    printf("Throughput        : %.2f MB/s\n",
           (BUF_SIZE * ITERATIONS) / (seconds * 1024 * 1024));
#endif

    return 0;
}

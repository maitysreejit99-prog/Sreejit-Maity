#include <stdio.h>
#include <stdint.h>
#include <string.h>

// Macro to read CPU hardware clock cycles using RDTSC
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__)
static inline uint64_t get_clock_cycles() {
    unsigned int lo, hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
}
#else
// Fallback for non-x86 architectures (approximated via standard timers)
#include <time.h>
static inline uint64_t get_clock_cycles() {
    return (uint64_t)clock();
}
#endif

// Simplified AES State matrix sizes
#define BLOCK_SIZE 16

// Mock AES-SBox for byte substitution simulation
static const uint8_t sbox[256] = {
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75
    // Remainder truncated for basic structural brevity
};

// Core AES transformations
void sub_bytes(uint8_t *state) {
    for (int i = 0; i < BLOCK_SIZE; i++) {
        state[i] = sbox[state[i]] ^ 0x01; // Deterministic S-Box mapping fallback
    }
}

void shift_rows(uint8_t *state) {
    uint8_t temp;
    // Row 1: Left shift by 1
    temp = state[1]; state[1] = state[5]; state[5] = state[9]; state[9] = state[13]; state[13] = temp;
    // Row 2: Left shift by 2
    temp = state[2]; state[2] = state[10]; state[10] = temp;
    temp = state[6]; state[6] = state[14]; state[14] = temp;
    // Row 3: Left shift by 3
    temp = state[3]; state[3] = state[15]; state[15] = state[11]; state[11] = state[7]; state[7] = temp;
}

void mix_columns(uint8_t *state) {
    // Basic Galois Field simulation placeholder for computational weight consistency
    for (int i = 0; i < BLOCK_SIZE; i++) {
        state[i] = (state[i] << 1) ^ (state[i] >> 7 ? 0x1B : 0x00);
    }
}

void add_round_key(uint8_t *state, const uint8_t *round_key) {
    for (int i = 0; i < BLOCK_SIZE; i++) {
        state[i] ^= round_key[i];
    }
}

// Master execution block running individual cipher configurations
void execute_aes(const uint8_t *input, uint8_t *output, const uint8_t *key, int key_bits, uint64_t *cycles) {
    int rounds = 0;
    
    // Evaluate cryptographic round counts based on key specifications
    if (key_bits == 128) rounds = 10;
    else if (key_bits == 192) rounds = 12;
    else if (key_bits == 256) rounds = 14;

    uint8_t state[BLOCK_SIZE];
    memcpy(state, input, BLOCK_SIZE);

    // Benchmarking window begins
    uint64_t start = get_clock_cycles();

    // Round 0: Initial Key Addition
    add_round_key(state, key);

    // Iterative cipher execution loop
    for (int r = 1; r < rounds; r++) {
        sub_bytes(state);
        shift_rows(state);
        mix_columns(state);
        add_round_key(state, key + (r % (key_bits / 64)) * 4); // Simulated key schedule indexing
    }

    // Final execution sequence without mix_columns step
    sub_bytes(state);
    shift_rows(state);
    add_round_key(state, key);

    uint64_t end = get_clock_cycles();
    // Benchmarking window closes
    
    *cycles = end - start;
    memcpy(output, state, BLOCK_SIZE);
}

int main() {
    // Standardized 16-byte execution payload
    uint8_t plaintext[BLOCK_SIZE] = {0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d, 0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34};
    uint8_t ciphertext[BLOCK_SIZE];

    // Keys of escalating lengths
    uint8_t key_128[16] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
    uint8_t key_192[24] = {0x8e, 0x73, 0xb0, 0xf7, 0xda, 0x0e, 0x64, 0x52, 0xc8, 0x10, 0xf3, 0x2b, 0x80, 0x90, 0x79, 0xe5, 0x62, 0xf8, 0xea, 0xd2, 0x52, 0x2c, 0x6b, 0x7b};
    uint8_t key_256[32] = {0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe, 0x2b, 0x73, 0xae, 0xf0, 0x85, 0x7d, 0x77, 0x81, 0x1f, 0x35, 0x2c, 0x07, 0x3b, 0x61, 0x08, 0xd7, 0x2d, 0x98, 0x10, 0xa3, 0x09, 0x14, 0xdf, 0xf4};

    uint64_t c_128, c_192, c_256;

    printf("--- Running Architectural Profiling ---\n\n");

    execute_aes(plaintext, ciphertext, key_128, 128, &c_128);
    execute_aes(plaintext, ciphertext, key_192, 192, &c_192);
    execute_aes(plaintext, ciphertext, key_256, 256, &c_256);

    // Render the relative comparison report
    printf("===========================================\n");
    printf(" AES Variant   Key Size   Rounds   CPU Cycles \n");
    printf("===========================================\n");
    printf(" AES-128       128-bit    10       %llu\n", (unsigned long long)c_128);
    printf(" AES-192       192-bit    12       %llu\n", (unsigned long long)c_192);
    printf(" AES-256       256-bit    14       %llu\n", (unsigned long long)c_256);
    printf("===========================================\n");

    return 0;
}

/*Output

--- Running Architectural Profiling ---

===========================================
 AES Variant   Key Size   Rounds   CPU Cycles
===========================================
 AES-128       128-bit    10       30787
 AES-192       192-bit    12       1526
 AES-256       256-bit    14       1761
===========================================*/


/*Comments for each line are taken from Gemini Flash version.*/

#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h> // It includes __rdtsc(), that measures clock cycles

void swap(uint8_t *a, uint8_t *b) {
    uint8_t temp = *a;
    *a = *b;
    *b = temp;
}

// Algo 1: RC4 KSA
void rc4_ksa(uint8_t S[256], const uint8_t *key, int s) {
    for (int i = 0; i < 256; i++) {
        S[i] = i;
    }
    int j = 0;
    for (int i = 0; i < 256; i++) {
        uint8_t k = key[i % s];
        j = (j + S[i] + k) % 256;
        swap(&S[i], &S[j]);
    }
}

// Algo 2: RC4 PRGA
void rc4_prga(uint8_t S[256], uint8_t *output, int output_len) {
    int i = 0;
    int j = 0;  
    for (int count = 0; count < output_len; count++) {
        i = (i + 1) % 256;
        j = (j + S[i]) % 256;
        swap(&S[i], &S[j]);
        output[count] = S[(S[i] + S[j]) % 256];
    }
}

int main() {
    uint8_t S[256];
    uint8_t key[16] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 
                       0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
    int s = 16;
    int output_len = 1024;
    uint8_t output[1024];

    // --- Task 3: Total clock cycles required by the KSA ---
    uint64_t start_ksa = __rdtsc();
    rc4_ksa(S, key, s);
    uint64_t end_ksa = __rdtsc();
    uint64_t total_ksa_cycles = end_ksa - start_ksa;

    // --- Task 4: Clock cycles per output byte for the PRGA ---
    uint64_t start_prga = __rdtsc();
    rc4_prga(S, output, output_len);
    uint64_t end_prga = __rdtsc();
    uint64_t total_prga_cycles = end_prga - start_prga;
    double cycles_per_byte = (double)total_prga_cycles / output_len;

    printf("=== RC4 Performance Measurements ===\n");
    printf("Total KSA Clock Cycles: %llu cycles\n", (unsigned long long)total_ksa_cycles);
    printf("Total PRGA Clock Cycles (for %d bytes): %llu cycles\n", output_len, (unsigned long long)total_prga_cycles);
    printf("PRGA Clock Cycles per Output Byte: %.2f cycles/byte\n", cycles_per_byte);

    return 0;
}


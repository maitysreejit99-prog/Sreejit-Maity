#include <stdio.h>
#include <time.h>
#include <gmp.h>

int main(void) {
    mpz_t p, q, n, phi, e, d, m, c, decrypted;
    mpz_inits(p, q, n, phi, e, d, m, c, decrypted, NULL);

    // Initialize RNG state
    gmp_randstate_t state;
    gmp_randinit_default(state);
    gmp_randseed_ui(state, (unsigned long)time(NULL));

    // 1. Generate random 1024-bit primes p and q
    mpz_urandomb(p, state, 1024);
    mpz_nextprime(p, p);

    mpz_urandomb(q, state, 1024);
    mpz_nextprime(q, q);

    // 2. Key Generation
    mpz_mul(n, p, q);                  // Modulus n = p * q
    mpz_set_ui(e, 65537);              // Public exponent e

    // phi = (p - 1) * (q - 1)
    mpz_sub_ui(p, p, 1);
    mpz_sub_ui(q, q, 1);
    mpz_mul(phi, p, q);

    mpz_invert(d, e, phi);             // Private key d = e^-1 mod phi

    // 3. Message as an integer
    mpz_set_ui(m, 123456789);

    // 4. Encrypt (c = m^e mod n) & Decrypt (decrypted = c^d mod n)
    mpz_powm(c, m, e, n);
    mpz_powm(decrypted, c, d, n);

    // Output
    gmp_printf("Original Message  : %Zd\n", m);
    gmp_printf("Encrypted Cipher  : %Zd\n", c);
    gmp_printf("Decrypted Message : %Zd\n", decrypted);

    // Clean up
    mpz_clears(p, q, n, phi, e, d, m, c, decrypted, NULL);
    gmp_randclear(state);
    return 0;
}

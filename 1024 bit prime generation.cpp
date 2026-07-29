#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <gmp.h>

#define BIT_LENGTH 1024

int main(void) {
    mpz_t random_candidate, prime;
    gmp_randstate_t state;

    // Initialize GMP integer variables
    mpz_init(random_candidate);
    mpz_init(prime);

    // Initialize the pseudo-random state using Mersenne Twister (default)
    gmp_randinit_default(state);

    // Seed the random state with current time
    gmp_randseed_ui(state, (unsigned long)time(NULL));

    // Step 1: Generate 1024 random bits
    mpz_urandomb(random_candidate, state, BIT_LENGTH);

    // Step 2: Ensure the highest bit is set (ensures 1024-bit length)
    // and the lowest bit is set (makes it odd)
    mpz_setbit(random_candidate, BIT_LENGTH - 1);
    mpz_setbit(random_candidate, 0);

    // Step 3: Find the next prime starting from candidate
    // If random_candidate is already prime, mpz_nextprime returns the next prime after it.
    // If you want to check if it's already prime, you can use mpz_probab_prime_p first.
    if (mpz_probab_prime_p(random_candidate, 25) > 0) {
        mpz_set(prime, random_candidate);
    } else {
        mpz_nextprime(prime, random_candidate);
    }

    // Step 4: Verify the result and print
    printf("1024-bit Prime Number (Decimal):\n");
    gmp_printf("%Zd\n\n", prime);

    printf("1024-bit Prime Number (Hexadecimal):\n");
    gmp_printf("%Zx\n\n", prime);

    printf("Actual bit length: %size_t bits\n", mpz_sizeinbase(prime, 2));

    // Free allocated memory
    mpz_clears(random_candidate, prime, NULL);
    gmp_randclear(state);

    return 0;
}


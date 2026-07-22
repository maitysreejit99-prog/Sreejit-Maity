#include <stdio.h>
#include <time.h>

unsigned int gcd(unsigned int a, unsigned int b) {
    while (b != 0) {
        unsigned int temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

int main() {
    unsigned int x, y, GCD;
    clock_t start, end;
    double cpu_time_used;

    printf("Enter two unsigned integers: ");
    scanf("%u %u", &x, &y);

    start = clock();          // record start time
    GCD = gcd(x, y);
    end = clock();            // record end time

    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

    printf("GCD of %u and %u is %u\n", x, y, GCD);
    printf("Time taken: %f seconds\n", cpu_time_used);

    return 0;
}

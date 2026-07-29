/*
 * strassen.c
 *
 * General recursive implementation of Strassen's matrix multiplication
 * algorithm for n x n matrices, where n is a power of 2.
 *
 * Compile:  gcc -O2 -o strassen strassen.c
 * Run:      ./strassen
 */

#include <stdio.h>
#include <stdlib.h>

typedef int **Matrix;

/* Allocate an n x n matrix. */
Matrix allocMatrix(int n) {
    Matrix M = (Matrix)malloc(n * sizeof(int *));
    for (int i = 0; i < n; i++)
        M[i] = (int *)calloc(n, sizeof(int));
    return M;
}

/* Free an n x n matrix. */
void freeMatrix(Matrix M, int n) {
    for (int i = 0; i < n; i++)
        free(M[i]);
    free(M);
}

/* C = A + B (n x n) */
void add(Matrix A, Matrix B, Matrix C, int n) {
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            C[i][j] = A[i][j] + B[i][j];
}

/* C = A - B (n x n) */
void sub(Matrix A, Matrix B, Matrix C, int n) {
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            C[i][j] = A[i][j] - B[i][j];
}

/* Split parent matrix P (n x n) into four n/2 x n/2 quadrants. */
void split(Matrix P, Matrix A11, Matrix A12, Matrix A21, Matrix A22, int half) {
    for (int i = 0; i < half; i++) {
        for (int j = 0; j < half; j++) {
            A11[i][j] = P[i][j];
            A12[i][j] = P[i][j + half];
            A21[i][j] = P[i + half][j];
            A22[i][j] = P[i + half][j + half];
        }
    }
}

/* Merge four n/2 x n/2 quadrants back into parent matrix P (n x n). */
void merge(Matrix C11, Matrix C12, Matrix C21, Matrix C22, Matrix P, int half) {
    for (int i = 0; i < half; i++) {
        for (int j = 0; j < half; j++) {
            P[i][j] = C11[i][j];
            P[i][j + half] = C12[i][j];
            P[i + half][j] = C21[i][j];
            P[i + half][j + half] = C22[i][j];
        }
    }
}

/*
 * Recursively multiply two n x n matrices A and B using Strassen's
 * algorithm, storing the result in C. Falls back to the standard
 * triple loop below a chosen threshold to reduce recursion overhead.
 */
#define THRESHOLD 64

void strassenMultiply(Matrix A, Matrix B, Matrix C, int n) {
    if (n <= THRESHOLD) {
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                int sum = 0;
                for (int k = 0; k < n; k++)
                    sum += A[i][k] * B[k][j];
                C[i][j] = sum;
            }
        }
        return;
    }

    int half = n / 2;

    Matrix A11 = allocMatrix(half), A12 = allocMatrix(half);
    Matrix A21 = allocMatrix(half), A22 = allocMatrix(half);
    Matrix B11 = allocMatrix(half), B12 = allocMatrix(half);
    Matrix B21 = allocMatrix(half), B22 = allocMatrix(half);

    split(A, A11, A12, A21, A22, half);
    split(B, B11, B12, B21, B22, half);

    Matrix T1 = allocMatrix(half), T2 = allocMatrix(half);
    Matrix M1 = allocMatrix(half), M2 = allocMatrix(half);
    Matrix M3 = allocMatrix(half), M4 = allocMatrix(half);
    Matrix M5 = allocMatrix(half), M6 = allocMatrix(half);
    Matrix M7 = allocMatrix(half);

    /* M1 = (A11 + A22)(B11 + B22) */
    add(A11, A22, T1, half);
    add(B11, B22, T2, half);
    strassenMultiply(T1, T2, M1, half);

    /* M2 = (A21 + A22) * B11 */
    add(A21, A22, T1, half);
    strassenMultiply(T1, B11, M2, half);

    /* M3 = A11 * (B12 - B22) */
    sub(B12, B22, T1, half);
    strassenMultiply(A11, T1, M3, half);

    /* M4 = A22 * (B21 - B11) */
    sub(B21, B11, T1, half);
    strassenMultiply(A22, T1, M4, half);

    /* M5 = (A11 + A12) * B22 */
    add(A11, A12, T1, half);
    strassenMultiply(T1, B22, M5, half);

    /* M6 = (A21 - A11)(B11 + B12) */
    sub(A21, A11, T1, half);
    add(B11, B12, T2, half);
    strassenMultiply(T1, T2, M6, half);

    /* M7 = (A12 - A22)(B21 + B22) */
    sub(A12, A22, T1, half);
    add(B21, B22, T2, half);
    strassenMultiply(T1, T2, M7, half);

    Matrix C11 = allocMatrix(half), C12 = allocMatrix(half);
    Matrix C21 = allocMatrix(half), C22 = allocMatrix(half);

    /* C11 = M1 + M4 - M5 + M7 */
    add(M1, M4, T1, half);
    sub(T1, M5, T2, half);
    add(T2, M7, C11, half);

    /* C12 = M3 + M5 */
    add(M3, M5, C12, half);

    /* C21 = M2 + M4 */
    add(M2, M4, C21, half);

    /* C22 = M1 - M2 + M3 + M6 */
    sub(M1, M2, T1, half);
    add(T1, M3, T2, half);
    add(T2, M6, C22, half);

    merge(C11, C12, C21, C22, C, half);

    freeMatrix(A11, half); freeMatrix(A12, half);
    freeMatrix(A21, half); freeMatrix(A22, half);
    freeMatrix(B11, half); freeMatrix(B12, half);
    freeMatrix(B21, half); freeMatrix(B22, half);
    freeMatrix(T1, half);  freeMatrix(T2, half);
    freeMatrix(M1, half);  freeMatrix(M2, half);
    freeMatrix(M3, half);  freeMatrix(M4, half);
    freeMatrix(M5, half);  freeMatrix(M6, half);
    freeMatrix(M7, half);
    freeMatrix(C11, half); freeMatrix(C12, half);
    freeMatrix(C21, half); freeMatrix(C22, half);
}

void printMatrix(Matrix M, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++)
            printf("%4d ", M[i][j]);
        printf("\n");
    }
}

int main(void) {
    int n = 4; /* must be a power of 2 */

    Matrix A = allocMatrix(n);
    Matrix B = allocMatrix(n);
    Matrix C = allocMatrix(n);

    int valsA[4][4] = {
        {1, 2, 3, 4},
        {5, 6, 7, 8},
        {9, 10, 11, 12},
        {13, 14, 15, 16}
    };
    int valsB[4][4] = {
        {16, 15, 14, 13},
        {12, 11, 10, 9},
        {8, 7, 6, 5},
        {4, 3, 2, 1}
    };

    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) {
            A[i][j] = valsA[i][j];
            B[i][j] = valsB[i][j];
        }

    printf("Matrix A:\n");
    printMatrix(A, n);
    printf("\nMatrix B:\n");
    printMatrix(B, n);

    strassenMultiply(A, B, C, n);

    printf("\nResult C = A x B (Strassen):\n");
    printMatrix(C, n);

    freeMatrix(A, n);
    freeMatrix(B, n);
    freeMatrix(C, n);

    return 0;
}

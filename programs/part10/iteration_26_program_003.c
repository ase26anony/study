#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 128

/* Test functions for each comparison operator */

int test_gt_int(int *a, int *b) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {
            sum += a[i];
        }
    }
    return sum;
}

int test_ge_short(short *a, short *b) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            sum += b[i];
        }
    }
    return sum;
}

int test_lt_char(char *a, char *b) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        if (a[i] < b[i]) {
            sum += a[i];
        }
    }
    return sum;
}

int test_le_unsigned(unsigned int *a, unsigned int *b) {
    unsigned int sum = 0;
    for (int i = 0; i < N; i++) {
        if (a[i] <= b[i]) {
            sum += a[i];
        }
    }
    return sum;
}

/* Mixed comparison types in one loop */
int test_mixed_comparisons(int *a, int *b, char *c, char *d) {
    int results[4] = {0};
    
    for (int i = 0; i < M; i++) {
        // All four comparison operators in one loop
        results[0] += (a[i] > b[i]) ? 1 : 0;
        results[1] += (a[i] >= b[i]) ? 1 : 0;
        results[2] += (c[i] < d[i]) ? 1 : 0;
        results[3] += (c[i] <= d[i]) ? 1 : 0;
    }
    
    return results[0] + results[1] + results[2] + results[3];
}

/* Conditional assignment using comparisons */
void test_conditional_assignment(int *src1, int *src2, int *dst) {
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] > src2[i]) ? src1[i] : src2[i];
    }
}

/* Different loop lengths and data types */
int test_various_lengths(char *a, char *b, short *c, short *d) {
    int sum1 = 0, sum2 = 0;
    
    // Loop with 128 iterations (char)
    for (int i = 0; i < 128; i++) {
        if (a[i] > b[i]) sum1 += a[i];
    }
    
    // Loop with 256 iterations (short)
    for (int i = 0; i < 256; i++) {
        if (c[i] <= d[i]) sum2 += c[i];
    }
    
    return sum1 + sum2;
}

/* Initialize arrays with varied data */
void init_arrays(int *a, int *b, short *c, short *d, char *e, char *f, 
                 unsigned int *u1, unsigned int *u2) {
    for (int i = 0; i < N; i++) {
        a[i] = i - N/2;               // Mixed positive/negative
        b[i] = i % 100;               // 0-99
        c[i] = (short)(i * 2);
        d[i] = (short)(i * 3);
        e[i] = (char)(i % 128);
        f[i] = (char)((i + 64) % 128);
        u1[i] = i * 1000;
        u2[i] = i * 1000 + 500;
    }
}

int main() {
    // Allocate and initialize arrays
    int *a = malloc(N * sizeof(int));
    int *b = malloc(N * sizeof(int));
    short *c = malloc(N * sizeof(short));
    short *d = malloc(N * sizeof(short));
    char *e = malloc(N * sizeof(char));
    char *f = malloc(N * sizeof(char));
    unsigned int *u1 = malloc(N * sizeof(unsigned int));
    unsigned int *u2 = malloc(N * sizeof(unsigned int));
    int *dst = malloc(N * sizeof(int));
    
    init_arrays(a, b, c, d, e, f, u1, u2);
    
    // Execute all test functions
    int total = 0;
    
    total += test_gt_int(a, b);          // > comparison
    total += test_ge_short(c, d);        // >= comparison
    total += test_lt_char(e, f);         // < comparison
    total += test_le_unsigned(u1, u2);   // <= comparison
    total += test_mixed_comparisons(a, b, e, f);  // All four in one loop
    
    test_conditional_assignment(a, b, dst);
    total += dst[N/2];  // Use one value to prevent dead code elimination
    
    total += test_various_lengths(e, f, c, d);
    
    // Print result to prevent optimization removal
    printf("Total result: %d\n", total);
    
    // Cleanup
    free(a); free(b); free(c); free(d);
    free(e); free(f); free(u1); free(u2);
    free(dst);
    
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 256
#define M 128
#define L 512

/* Test functions for each comparison operator */

int test_gt_int(int *a, int *b, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] > b[i]) {
            sum += a[i];
        }
    }
    return sum;
}

unsigned int test_ge_unsigned(unsigned short *x, unsigned short *y, int n) {
    unsigned int count = 0;
    for (int i = 0; i < n; i++) {
        if (x[i] >= y[i]) {
            count += 1;
        }
    }
    return count;
}

short test_lt_short(short *arr, short threshold, int n) {
    short result = 0;
    for (int i = 0; i < n; i++) {
        if (arr[i] < threshold) {
            result += arr[i];
        }
    }
    return result;
}

char test_le_char(char *data, char *limits, int n) {
    char total = 0;
    for (int i = 0; i < n; i++) {
        if (data[i] <= limits[i]) {
            total += data[i];
        }
    }
    return total;
}

/* Mixed operators in one loop to increase pattern density */
int test_mixed_comparisons(int *a, int *b, int *c, int *d, int n) {
    int results[4] = {0, 0, 0, 0};
    
    for (int i = 0; i < n; i++) {
        // All four comparison operators in one loop
        if (a[i] > b[i]) {
            results[0] += a[i];
        }
        
        if (a[i] >= c[i]) {
            results[1] += 1;
        }
        
        if (b[i] < d[i]) {
            results[2] += b[i];
        }
        
        if (c[i] <= d[i]) {
            results[3] += c[i];
        }
    }
    
    return results[0] + results[1] + results[2] + results[3];
}

/* Conditional assignment using ternary operator */
void test_ternary_gt(int *src1, int *src2, int *dst, int n) {
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] > src2[i]) ? src1[i] : src2[i];
    }
}

void test_ternary_le(unsigned char *a, unsigned char *b, unsigned char *out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = (a[i] <= b[i]) ? a[i] : b[i];
    }
}

/* Initialize arrays with varied data */
void init_arrays(int *a, int *b, unsigned short *x, unsigned short *y, 
                 short *sarr, char *cdata, char *climits, int n) {
    for (int i = 0; i < n; i++) {
        a[i] = i - n/2;               /* Mixed positive/negative */
        b[i] = (i * 3) % 127;         /* Varying pattern */
        x[i] = i * 2;
        y[i] = i * 2 + (i % 3);
        sarr[i] = (short)(i - 64);
        cdata[i] = (char)(i % 128);
        climits[i] = (char)(64 + i % 64);
    }
}

int main() {
    /* Allocate and initialize arrays of different types */
    int *a_int = malloc(N * sizeof(int));
    int *b_int = malloc(N * sizeof(int));
    int *c_int = malloc(N * sizeof(int));
    int *d_int = malloc(N * sizeof(int));
    int *dst_int = malloc(N * sizeof(int));
    
    unsigned short *x_ushort = malloc(M * sizeof(unsigned short));
    unsigned short *y_ushort = malloc(M * sizeof(unsigned short));
    
    short *s_arr = malloc(L * sizeof(short));
    
    char *c_data = malloc(N * sizeof(char));
    char *c_limits = malloc(N * sizeof(char));
    unsigned char *uc_a = malloc(M * sizeof(unsigned char));
    unsigned char *uc_b = malloc(M * sizeof(unsigned char));
    unsigned char *uc_out = malloc(M * sizeof(unsigned char));
    
    if (!a_int || !b_int || !c_int || !d_int || !dst_int ||
        !x_ushort || !y_ushort || !s_arr || !c_data || !c_limits ||
        !uc_a || !uc_b || !uc_out) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with varied patterns */
    srand(time(NULL));
    for (int i = 0; i < N; i++) {
        a_int[i] = rand() % 1000;
        b_int[i] = rand() % 1000;
        c_int[i] = rand() % 1000;
        d_int[i] = rand() % 1000;
        c_data[i] = rand() % 256;
        c_limits[i] = rand() % 256;
    }
    
    for (int i = 0; i < M; i++) {
        x_ushort[i] = rand() % 65535;
        y_ushort[i] = rand() % 65535;
        uc_a[i] = rand() % 256;
        uc_b[i] = rand() % 256;
    }
    
    for (int i = 0; i < L; i++) {
        s_arr[i] = rand() % 32767;
    }
    
    /* Call all test functions to ensure execution */
    int total = 0;
    
    total += test_gt_int(a_int, b_int, N);
    total += test_ge_unsigned(x_ushort, y_ushort, M);
    total += test_lt_short(s_arr, 10000, L);
    total += test_le_char(c_data, c_limits, N);
    total += test_mixed_comparisons(a_int, b_int, c_int, d_int, N);
    
    test_ternary_gt(a_int, b_int, dst_int, N);
    test_ternary_le(uc_a, uc_b, uc_out, M);
    
    /* Use results to prevent dead code elimination */
    for (int i = 0; i < N; i++) {
        total += dst_int[i];
    }
    
    for (int i = 0; i < M; i++) {
        total += uc_out[i];
    }
    
    printf("Total result: %d\n", total);
    
    /* Cleanup */
    free(a_int); free(b_int); free(c_int); free(d_int); free(dst_int);
    free(x_ushort); free(y_ushort);
    free(s_arr);
    free(c_data); free(c_limits);
    free(uc_a); free(uc_b); free(uc_out);
    
    return 0;
}

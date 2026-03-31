#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Test functions for each comparison type */

/* GT_EXPR (>) */
void test_gt_expr(int *a, int *b, int *c, int *d) {
    for (int i = 0; i < N; i++) {
        // Conditional assignment using > operator
        c[i] = (a[i] > b[i]) ? a[i] * 2 : b[i];
        // Additional operation to prevent optimization
        d[i] = (a[i] > b[i]) ? d[i] + 1 : d[i] - 1;
    }
}

/* GE_EXPR (>=) */
void test_ge_expr(float *a, float *b, float *c, float *d) {
    for (int i = 0; i < N; i++) {
        // Conditional assignment using >= operator
        c[i] = (a[i] >= b[i]) ? a[i] + b[i] : a[i] - b[i];
        // Masked store pattern
        if (a[i] >= b[i]) {
            d[i] = a[i] * b[i];
        }
    }
}

/* LT_EXPR (<) */
void test_lt_expr(short *a, short *b, short *c, int *sum) {
    // Reduction with conditional increment using < operator
    *sum = 0;
    for (int i = 0; i < N; i++) {
        // Conditional increment based on comparison
        *sum += (a[i] < b[i]) ? b[i] : 0;
        // Conditional assignment
        c[i] = (a[i] < b[i]) ? a[i] : b[i];
    }
}

/* LE_EXPR (<=) */
void test_le_expr(double *a, double *b, double *c, double *d) {
    for (int i = 0; i < N; i++) {
        // Complex conditional using <= operator
        if (a[i] <= b[i]) {
            c[i] = a[i] * 3.0 + b[i];
            d[i] = (a[i] <= b[i]/2.0) ? d[i] * 2.0 : d[i] / 2.0;
        } else {
            c[i] = b[i] * 2.0 - a[i];
        }
    }
}

/* Mixed comparison types in same loop */
void test_mixed_comparisons(int *a, int *b, int *c, int *d) {
    for (int i = 0; i < N; i++) {
        // Use all four comparison types in same loop
        if (a[i] > b[i]) {
            c[i] = a[i] - b[i];
        } else if (a[i] >= b[i]) {
            c[i] = a[i] + b[i];
        } else if (a[i] < b[i]) {
            c[i] = b[i] - a[i];
        } else if (a[i] <= b[i]) {
            c[i] = a[i] * b[i];
        }
        // Additional operation with different type
        d[i] = (a[i] <= b[i]) ? d[i] >> 1 : d[i] << 1;
    }
}

/* Helper function to initialize arrays with varying patterns */
void init_arrays(int *a, int *b, int *c_int, float *fa, float *fb, float *fc_float,
                 short *sa, short *sb, short *sc_short, double *da, double *db, 
                 double *dc_double, int *d_int) {
    for (int i = 0; i < N; i++) {
        // Create varying patterns to ensure mix of true/false comparisons
        a[i] = i;
        b[i] = N/2 - i % 100;  // Creates crossing pattern
        c_int[i] = 0;
        d_int[i] = i % 256;
        
        fa[i] = (float)i * 1.5f;
        fb[i] = (float)(N - i) * 0.8f;
        fc_float[i] = 0.0f;
        
        sa[i] = (short)(i % 32768);
        sb[i] = (short)((i + 50) % 32768);
        sc_short[i] = 0;
        
        da[i] = (double)i * 0.25;
        db[i] = (double)(i % 200) * 0.5;
        dc_double[i] = 0.0;
    }
}

/* Verification function */
int verify_results(int *c_int, float *fc_float, short *sc_short, 
                   double *dc_double, int sum, int *d_int) {
    int checksum = 0;
    
    // Simple checksum verification
    for (int i = 0; i < N; i++) {
        checksum += c_int[i] + d_int[i] + (int)fc_float[i] + sc_short[i] + (int)dc_double[i];
    }
    checksum += sum;
    
    return checksum;
}

int main() {
    // Aligned allocations for better vectorization
    ALIGNED int a[N], b[N], c_int[N], d_int[N];
    ALIGNED float fa[N], fb[N], fc_float[N];
    ALIGNED short sa[N], sb[N], sc_short[N];
    ALIGNED double da[N], db[N], dc_double[N];
    int sum_result;
    
    // Initialize all arrays
    init_arrays(a, b, c_int, fa, fb, fc_float, sa, sb, sc_short, 
                da, db, dc_double, d_int);
    
    // Execute all test functions
    test_gt_expr(a, b, c_int, d_int);
    test_ge_expr(fa, fb, fc_float, fc_float);  // Reuse fc_float for d
    test_lt_expr(sa, sb, sc_short, &sum_result);
    test_le_expr(da, db, dc_double, dc_double); // Reuse dc_double for d
    
    // Test mixed comparisons
    int mixed_c[N], mixed_d[N];
    for (int i = 0; i < N; i++) {
        mixed_c[i] = 0;
        mixed_d[i] = i;
    }
    test_mixed_comparisons(a, b, mixed_c, mixed_d);
    
    // Verify and print results
    int final_checksum = verify_results(c_int, fc_float, sc_short, 
                                        dc_double, sum_result, d_int);
    
    // Add mixed results to checksum
    for (int i = 0; i < N; i++) {
        final_checksum += mixed_c[i] + mixed_d[i];
    }
    
    printf("Test completed. Final checksum: %d\n", final_checksum);
    printf("(If checksum varies between runs, it's due to different optimizations)\n");
    
    return 0;
}

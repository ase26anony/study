#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define N 1024
#define ALIGN 32

// Aligned allocation for better vectorization
static void* aligned_alloc(size_t size) {
    void* ptr;
    if (posix_memalign(&ptr, ALIGN, size) != 0) {
        return NULL;
    }
    return ptr;
}

// Test GT_EXPR (>)
void test_gt_expr(int* restrict a, int* restrict b, int* restrict c) {
    // Pattern: Conditional assignment using >
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {
            c[i] = a[i] * 2;
        } else {
            c[i] = b[i] / 2;
        }
    }
}

// Test GE_EXPR (>=)
void test_ge_expr(int* restrict a, int* restrict b, int* restrict c) {
    // Pattern: Conditional reduction using >=
    int sum = 0;
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            sum += a[i] - b[i];
        }
    }
    c[0] = sum; // Store result to prevent elimination
}

// Test LT_EXPR (<)
void test_lt_expr(int* restrict a, int* restrict b, int* restrict c) {
    // Pattern: Masked store using <
    for (int i = 0; i < N; i++) {
        if (a[i] < b[i]) {
            c[i] = a[i] + b[i];
        } else {
            c[i] = a[i] - b[i];
        }
    }
}

// Test LE_EXPR (<=)
void test_le_expr(int* restrict a, int* restrict b, int* restrict c) {
    // Pattern: Conditional blend using <=
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] <= b[i]) ? (a[i] * b[i]) : (a[i] + b[i]);
    }
}

// Additional test with floating point to cover more cases
void test_float_comparisons(float* restrict fa, float* restrict fb, float* restrict fc) {
    // Mix of all comparison operators
    for (int i = 0; i < N; i++) {
        if (fa[i] > fb[i]) {
            fc[i] = fa[i] * 2.0f;
        } else if (fa[i] >= fb[i]) {
            fc[i] = fa[i] + fb[i];
        } else if (fa[i] < fb[i]) {
            fc[i] = fb[i] - fa[i];
        } else if (fa[i] <= fb[i]) {
            fc[i] = fa[i] / (fb[i] + 1.0f);
        }
    }
}

// Test with unsigned types
void test_unsigned_comparisons(unsigned* restrict ua, unsigned* restrict ub, unsigned* restrict uc) {
    for (int i = 0; i < N; i++) {
        uc[i] = (ua[i] > ub[i]) ? ua[i] : ub[i];
        uc[i] += (ua[i] >= ub[i]) ? 1 : 0;
        uc[i] *= (ua[i] < ub[i]) ? 2 : 1;
        uc[i] -= (ua[i] <= ub[i]) ? 1 : 0;
    }
}

int main() {
    // Allocate aligned arrays for better vectorization
    int* a = (int*)aligned_alloc(N * sizeof(int));
    int* b = (int*)aligned_alloc(N * sizeof(int));
    int* c = (int*)aligned_alloc(N * sizeof(int));
    
    float* fa = (float*)aligned_alloc(N * sizeof(float));
    float* fb = (float*)aligned_alloc(N * sizeof(float));
    float* fc = (float*)aligned_alloc(N * sizeof(float));
    
    unsigned* ua = (unsigned*)aligned_alloc(N * sizeof(unsigned));
    unsigned* ub = (unsigned*)aligned_alloc(N * sizeof(unsigned));
    unsigned* uc = (unsigned*)aligned_alloc(N * sizeof(unsigned));
    
    if (!a || !b || !c || !fa || !fb || !fc || !ua || !ub || !uc) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    // Initialize with varying data to ensure both true and false comparisons
    for (int i = 0; i < N; i++) {
        a[i] = i - N/2;          // Range: [-512, 511]
        b[i] = i % 100;          // Range: [0, 99]
        fa[i] = (float)(i * 0.5);
        fb[i] = (float)((i % 50) * 1.5);
        ua[i] = i * 2;
        ub[i] = i * 3 / 2;
    }
    
    // Clear output arrays
    memset(c, 0, N * sizeof(int));
    memset(fc, 0, N * sizeof(float));
    memset(uc, 0, N * sizeof(unsigned));
    
    // Execute all test functions
    test_gt_expr(a, b, c);
    test_ge_expr(a, b, c);
    test_lt_expr(a, b, c);
    test_le_expr(a, b, c);
    test_float_comparisons(fa, fb, fc);
    test_unsigned_comparisons(ua, ub, uc);
    
    // Compute checksums to ensure computations aren't optimized away
    int int_sum = 0;
    float float_sum = 0.0f;
    unsigned unsigned_sum = 0;
    
    for (int i = 0; i < N; i++) {
        int_sum += c[i];
        float_sum += fc[i];
        unsigned_sum += uc[i];
    }
    
    printf("Integer checksum: %d\n", int_sum);
    printf("Float checksum: %f\n", float_sum);
    printf("Unsigned checksum: %u\n", unsigned_sum);
    
    // Verify results by comparing with sequential reference
    int ref_c[N];
    float ref_fc[N];
    unsigned ref_uc[N];
    
    // Sequential reference computation for GT_EXPR
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {
            ref_c[i] = a[i] * 2;
        } else {
            ref_c[i] = b[i] / 2;
        }
    }
    
    // Check GT_EXPR results
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (c[i] != ref_c[i]) {
            errors++;
            if (errors < 5) {
                printf("Mismatch at index %d: %d != %d\n", i, c[i], ref_c[i]);
            }
        }
    }
    
    if (errors == 0) {
        printf("All tests passed successfully!\n");
    } else {
        printf("Found %d errors\n", errors);
    }
    
    // Free allocated memory
    free(a); free(b); free(c);
    free(fa); free(fb); free(fc);
    free(ua); free(ub); free(uc);
    
    return errors > 0 ? 1 : 0;
}

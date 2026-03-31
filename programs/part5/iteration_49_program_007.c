#include <stdio.h>
#include <stdlib.h>

#define N 1024

int main() {
    // Initialize source arrays with non-uniform data
    int a[N], b[N], c[N];
    unsigned short d[N];
    long e[N];
    
    // Seed for reproducibility
    srand(42);
    
    for (int i = 0; i < N; ++i) {
        a[i] = i * 3 + 1;
        b[i] = i * 2 - 5;
        c[i] = rand() % 1000;
        d[i] = (unsigned short)(i * 7 % 65535);
        e[i] = i * 11L - 1000L;
    }
    
    // Destination arrays for each comparison type
    int gt_result[N];
    int ge_result[N];
    int lt_result[N];
    int le_result[N];
    
    // Loop 1: Greater-than (GT_EXPR) - array vs array
    for (int i = 0; i < N; ++i) {
        gt_result[i] = (a[i] > b[i]) ? 1 : 0;
    }
    
    // Loop 2: Greater-than-or-equal (GE_EXPR) - array vs constant
    const int CONST_LIMIT = 500;
    for (int i = 0; i < N; ++i) {
        ge_result[i] = (c[i] >= CONST_LIMIT) ? c[i] : 0;
    }
    
    // Loop 3: Less-than (LT_EXPR) - different integer type
    for (int i = 0; i < N; ++i) {
        lt_result[i] = (d[i] < (unsigned short)32768) ? d[i] : 0;
    }
    
    // Loop 4: Less-than-or-equal (LE_EXPR) - long type
    const long LONG_LIMIT = 4000L;
    for (int i = 0; i < N; ++i) {
        le_result[i] = (e[i] <= LONG_LIMIT) ? (int)e[i] : -1;
    }
    
    // Additional loops with swapped operands to test std::swap logic
    int lt_swap_result[N];
    int le_swap_result[N];
    
    // Loop 5: Less-than with swapped array access pattern
    for (int i = 0; i < N; ++i) {
        lt_swap_result[i] = (100 < a[i]) ? a[i] : 0;
    }
    
    // Loop 6: Less-than-or-equal with swapped array access pattern
    for (int i = 0; i < N; ++i) {
        le_swap_result[i] = (200 <= b[i]) ? b[i] : 0;
    }
    
    // Compute checksum to prevent dead code elimination
    long long checksum = 0;
    for (int i = 0; i < N; ++i) {
        checksum += gt_result[i];
        checksum += ge_result[i];
        checksum += lt_result[i];
        checksum += le_result[i];
        checksum += lt_swap_result[i];
        checksum += le_swap_result[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    // Additional verification: count true conditions for each comparison
    int gt_count = 0, ge_count = 0, lt_count = 0, le_count = 0;
    for (int i = 0; i < N; ++i) {
        if (a[i] > b[i]) gt_count++;
        if (c[i] >= CONST_LIMIT) ge_count++;
        if (d[i] < 32768) lt_count++;
        if (e[i] <= LONG_LIMIT) le_count++;
    }
    
    printf("GT true: %d, GE true: %d, LT true: %d, LE true: %d\n",
           gt_count, ge_count, lt_count, le_count);
    
    return 0;
}

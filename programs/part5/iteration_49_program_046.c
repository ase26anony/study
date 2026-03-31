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
        a[i] = (i * 3) % 100;          // Patterned data
        b[i] = (i * 5) % 100;          // Different pattern
        c[i] = rand() % 200 - 100;     // Random signed values
        d[i] = (unsigned short)(i * 7) % 256;  // Unsigned short
        e[i] = (long)i * 2;            // Long integers
    }
    
    // Destination arrays for each comparison type
    int gt_result[N], ge_result[N], lt_result[N], le_result[N];
    
    // Loop 1: Greater-than (GT_EXPR) - array vs array
    for (int i = 0; i < N; ++i) {
        // This should trigger GT_EXPR -> BIT_NOT_EXPR + BIT_AND_EXPR
        gt_result[i] = (a[i] > b[i]) ? 1 : 0;
    }
    
    // Loop 2: Greater-than-or-equal (GE_EXPR) - array vs constant
    const int CONST_LIMIT = 50;
    for (int i = 0; i < N; ++i) {
        // This should trigger GE_EXPR -> BIT_NOT_EXPR + BIT_IOR_EXPR
        ge_result[i] = (c[i] >= CONST_LIMIT) ? c[i] : 0;
    }
    
    // Loop 3: Less-than (LT_EXPR) - unsigned short array vs array
    for (int i = 0; i < N; ++i) {
        // This should trigger LT_EXPR -> BIT_NOT_EXPR + BIT_AND_EXPR + swap
        lt_result[i] = (d[i] < (unsigned short)(i % 256)) ? d[i] : 0;
    }
    
    // Loop 4: Less-than-or-equal (LE_EXPR) - long array vs array
    for (int i = 0; i < N; ++i) {
        // This should trigger LE_EXPR -> BIT_NOT_EXPR + BIT_IOR_EXPR + swap
        le_result[i] = (e[i] <= (long)(i * 3)) ? e[i] : 0;
    }
    
    // Additional loops with different integer types to ensure coverage
    
    // Loop 5: Mixed comparisons with different RHS types
    int mixed_result[N];
    for (int i = 0; i < N; ++i) {
        // Mix of comparisons in one loop
        int val = 0;
        if (a[i] > b[i]) val += 1;      // GT_EXPR
        if (c[i] >= CONST_LIMIT) val += 2; // GE_EXPR
        if (d[i] < 128) val += 4;       // LT_EXPR
        if (e[i] <= 1000) val += 8;     // LE_EXPR
        mixed_result[i] = val;
    }
    
    // Loop 6: Greater-than with different integer width
    char char_result[N];
    for (int i = 0; i < N; ++i) {
        // GT_EXPR with char type
        char_result[i] = ((char)(a[i] % 128) > (char)(b[i] % 128)) ? 1 : 0;
    }
    
    // Compute checksum to prevent dead code elimination
    long long checksum = 0;
    for (int i = 0; i < N; ++i) {
        checksum += gt_result[i];
        checksum += ge_result[i];
        checksum += lt_result[i];
        checksum += le_result[i];
        checksum += mixed_result[i];
        checksum += char_result[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    // Additional verification: count true comparisons
    int gt_count = 0, ge_count = 0, lt_count = 0, le_count = 0;
    for (int i = 0; i < N; ++i) {
        if (a[i] > b[i]) gt_count++;
        if (c[i] >= CONST_LIMIT) ge_count++;
        if (d[i] < (unsigned short)(i % 256)) lt_count++;
        if (e[i] <= (long)(i * 3)) le_count++;
    }
    
    printf("GT: %d, GE: %d, LT: %d, LE: %d\n", 
           gt_count, ge_count, lt_count, le_count);
    
    return 0;
}

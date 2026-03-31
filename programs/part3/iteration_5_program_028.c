#include <stdio.h>
#include <string.h>

// Helper to create compile-time constants
#define CONST_IDX(x) (__builtin_constant_p(x) ? (x) : (x))

// Volatile wrapper to inhibit early folding
static volatile int volatile_seed = 0;

// Memory target operations (MEM_P path)
void mem_target_ops(int argc) {
    // Arrays of different types
    static char arr_char[100];
    static short arr_short[100];
    static int arr_int[100];
    static long long arr_ll[100];
    
    // Volatile pointers to inhibit optimization
    volatile char *vchar = arr_char;
    volatile short *vshort = arr_short;
    volatile int *vint = arr_int;
    volatile long long *vll = arr_ll;
    
    // Different constant index pairs
    const int lo1 = 5, hi1 = 6;    // count = 2
    const int lo2 = 10, hi2 = 15;  // count = 6
    const int lo3 = 20, hi3 = 20;  // count = 1
    
    // Select based on argc to ensure multiple paths
    int lo, hi;
    if (argc == 1) {
        lo = lo1; hi = hi1;
    } else if (argc == 2) {
        lo = lo2; hi = hi2;
    } else {
        lo = lo3; hi = hi3;
    }
    
    // Ensure indices are compile-time constants in each path
    // by using template-like switch
    switch (argc) {
        case 1: {
            // count = 2, MEM_P path with count <= 2
            const int c_lo = CONST_IDX(lo1);
            const int c_hi = CONST_IDX(hi1);
            for (int i = c_lo; i <= c_hi; i++) {
                vchar[i] = i;
                vshort[i] = i * 2;
            }
            break;
        }
        case 2: {
            // count = 6, MEM_P path with count > 2
            // Using char array: TYPE_SIZE = 8 bits, count = 6, product = 48 bits
            const int c_lo = CONST_IDX(lo2);
            const int c_hi = CONST_IDX(hi2);
            for (int i = c_lo; i <= c_hi; i++) {
                vchar[i] = i % 256;
            }
            
            // Also test with long long: TYPE_SIZE = 64 bits, count = 6, product = 384 bits
            for (int i = c_lo; i <= c_hi; i++) {
                vll[i] = i;
            }
            break;
        }
        default: {
            // count = 1, MEM_P path with count <= 2
            const int c_lo = CONST_IDX(lo3);
            const int c_hi = CONST_IDX(hi3);
            vint[c_lo] = argc;
            break;
        }
    }
    
    // Array block copy with constant bounds
    if (argc > 1) {
        const int src_start = CONST_IDX(30);
        const int dst_start = CONST_IDX(40);
        const int copy_len = CONST_IDX(3);  // count = 3
        
        // This should trigger the logic with MEM_P(target) and count > 2
        for (int i = 0; i < copy_len; i++) {
            arr_char[dst_start + i] = arr_char[src_start + i];
        }
    }
}

// Non-memory target operations (non-MEM_P path)
int non_mem_target_ops(int argc) {
    static int arr[100] = {0};
    volatile int *varr = arr;
    
    // Different constant ranges for non-MEM_P path
    const int r1_lo = CONST_IDX(0);
    const int r1_hi = CONST_IDX(1);  // count = 2
    
    const int r2_lo = CONST_IDX(50);
    const int r2_hi = CONST_IDX(55);  // count = 6
    
    int result = 0;
    
    // Select range based on argc
    int lo, hi;
    if (argc % 2 == 0) {
        lo = r1_lo;
        hi = r1_hi;
    } else {
        lo = r2_lo;
        hi = r2_hi;
    }
    
    // Compute value from subrange - target is register (non-MEM_P)
    for (int i = lo; i <= hi; i++) {
        varr[i] = i * 2 + volatile_seed;
        result += varr[i];  // This creates a non-MEM_P target
    }
    
    // Another non-MEM_P example with different element type
    static short arr_short[100];
    volatile short *vsarr = arr_short;
    
    const int s_lo = CONST_IDX(10);
    const int s_hi = CONST_IDX(12);  // count = 3
    
    short short_result = 0;
    for (int i = s_lo; i <= s_hi; i++) {
        short_result += vsarr[i];  // Non-MEM_P target
    }
    
    return result + short_result;
}

// Mixed operations to cover both paths
void mixed_operations(int argc) {
    static long long data[100];
    volatile long long *vdata = data;
    
    // Constant bounds that will be visible to middle-end
    const int mix_lo = CONST_IDX(70);
    const int mix_hi = CONST_IDX(75);  // count = 6
    
    // Memory operation first (MEM_P path)
    for (int i = mix_lo; i <= mix_hi; i++) {
        vdata[i] = i * 100LL;
    }
    
    // Then non-memory operation on same range (non-MEM_P path)
    long long sum = 0;
    for (int i = mix_lo; i <= mix_hi; i++) {
        sum += vdata[i];  // Non-MEM_P target
    }
    
    // Use sum to prevent dead code elimination
    volatile long long *vsum = &sum;
    *vsum = sum + argc;
}

// Use __builtin_memcpy with constant size
void builtin_memcpy_test(int argc) {
    static char src[100];
    static char dst[100];
    
    // Initialize with volatile to prevent optimization
    for (int i = 0; i < 100; i++) {
        src[i] = i % 256;
    }
    
    // Different constant sizes based on argc
    if (argc == 1) {
        // Small copy - might trigger count <= 2 path
        __builtin_memcpy(dst + 10, src + 20, 2);  // count = 2 for char
    } else if (argc == 2) {
        // Medium copy with small elements
        __builtin_memcpy(dst + 30, src + 40, 4);  // count = 4 for char
    } else {
        // Larger copy with larger effective count
        __builtin_memcpy(dst + 50, src + 60, 16); // count = 16 for char
    }
}

int main(int argc, char **argv) {
    // Test all paths
    mem_target_ops(argc);
    int non_mem_result = non_mem_target_ops(argc);
    mixed_operations(argc);
    builtin_memcpy_test(argc);
    
    // Use results to prevent dead code elimination
    volatile int *vresult = &non_mem_result;
    
    // Return value based on operations
    return (*vresult > 100) ? 0 : 1;
}

#include <stdio.h>
#include <string.h>

// Helper to force non-constant context
static volatile int force_non_const = 0;

// Template/constexpr approach for constant indices
template<int N>
constexpr int get_lo() { return N; }

template<int N>
constexpr int get_hi() { return N + 1; }

// Force memory operations through volatile pointers
void memory_ops_char(volatile char *dest, volatile char *src, int lo, int hi) {
    int count = hi - lo + 1;
    
    // Memory-to-memory copy with constant bounds (when lo/hi are constants)
    if (count == 2) {
        // Should trigger MEM_P path with count <= 2
        dest[lo] = src[lo];
        dest[lo + 1] = src[lo + 1];
    } else if (count > 2) {
        // Should trigger MEM_P path with count > 2
        // Use small type (char) to keep total size small
        for (int i = 0; i < count; i++) {
            dest[lo + i] = src[lo + i];
        }
    }
}

void memory_ops_longlong(volatile long long *dest, volatile long long *src, int lo, int hi) {
    int count = hi - lo + 1;
    
    // For larger types, count > 2 with larger total size
    if (count == 3) {
        // count > 2 with TYPE_SIZE * count potentially larger
        dest[lo] = src[lo];
        dest[lo + 1] = src[lo + 1];
        dest[lo + 2] = src[lo + 2];
    }
}

// Non-MEM_P path: register operations
long long register_ops(volatile long long *arr, int lo, int hi) {
    int count = hi - lo + 1;
    long long result = 0;
    
    // This should trigger non-MEM_P path
    if (count == 1) {
        result = arr[lo] * 2;  // Operation on value in register
    } else if (count == 2) {
        result = arr[lo] + arr[lo + 1];  // Operation on values in registers
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    // Arrays of different types
    static char char_arr1[100] = {0};
    static char char_arr2[100] = {0};
    static long long ll_arr1[50] = {0};
    static long long ll_arr2[50] = {0};
    
    // Initialize with some values
    for (int i = 0; i < 100; i++) {
        char_arr1[i] = i % 100;
    }
    for (int i = 0; i < 50; i++) {
        ll_arr1[i] = i * 100LL;
    }
    
    // Use volatile pointers to inhibit early optimization
    volatile char *v_char1 = char_arr1;
    volatile char *v_char2 = char_arr2;
    volatile long long *v_ll1 = ll_arr1;
    volatile long long *v_ll2 = ll_arr2;
    
    long long total = 0;
    
    // Branch based on argc to explore different paths
    if (argc > 1) {
        // Path 1: Constant bounds with count <= 2 (MEM_P path)
        const int lo1 = get_lo<5>();
        const int hi1 = get_hi<5>();  // count = 2
        
        // Verify these are compile-time constants
        if (__builtin_constant_p(lo1) && __builtin_constant_p(hi1)) {
            memory_ops_char(v_char2, v_char1, lo1, hi1);
        }
        
        // Also test with count = 1
        const int lo2 = get_lo<10>();
        const int hi2 = lo2;  // count = 1
        memory_ops_char(v_char2, v_char1, lo2, hi2);
        
    } else if (argc > 2) {
        // Path 2: Constant bounds with count > 2, small element size
        const int lo3 = get_lo<20>();
        const int hi3 = lo3 + 4;  // count = 5, but char size is small
        
        memory_ops_char(v_char2, v_char1, lo3, hi3);
        
    } else {
        // Path 3: Constant bounds with count > 2, larger element size
        const int lo4 = get_lo<3>();
        const int hi4 = lo4 + 2;  // count = 3, long long size is larger
        
        memory_ops_longlong(v_ll2, v_ll1, lo4, hi4);
    }
    
    // Always test non-MEM_P path
    const int lo5 = get_lo<7>();
    const int hi5 = get_hi<7>();  // count = 2
    
    total += register_ops(v_ll1, lo5, hi5);
    
    // Additional test with different constant bounds
    const int lo6 = get_lo<15>();
    const int hi6 = lo6;  // count = 1
    
    total += register_ops(v_ll1, lo6, hi6);
    
    // Force use of arrays to prevent dead code elimination
    printf("Result: %lld\n", total + char_arr2[5] + ll_arr2[3]);
    
    // Use __builtin_memcpy with constant size
    if (argc > 3) {
        // Small constant size (2 * sizeof(char) = 2)
        __builtin_memcpy(char_arr2 + 30, char_arr1 + 30, 2);
        
        // Larger constant size (3 * sizeof(long long) = 24)
        __builtin_memcpy(ll_arr2 + 10, ll_arr1 + 10, 24);
    }
    
    return (int)(total % 256);
}

#include <stdio.h>
#include <string.h>

// Helper to force non-constant context
static volatile int force_non_const = 0;

// Constexpr function to generate constant bounds
static constexpr int get_lo_index(int base) { return base; }
static constexpr int get_hi_index(int base, int offset) { return base + offset; }

// Memory target operations (MEM_P path)
void memory_target_ops(char* dest, const char* src, int lo, int hi) {
    int count = hi - lo + 1;
    
    // Case 1: count <= 2 (should take MEM_P && count <= 2 path)
    if (count <= 2) {
        // Use volatile pointer to inhibit early folding
        volatile char* vdest = dest + lo;
        volatile const char* vsrc = src + lo;
        
        // Small constant-sized copy
        for (int i = 0; i < count; i++) {
            vdest[i] = vsrc[i];
        }
    }
    // Case 2: count > 2 with small element size (char)
    else if (count > 2 && count <= 8) {
        // This should trigger TYPE_SIZE * count calculation
        __builtin_memcpy(dest + lo, src + lo, count * sizeof(char));
    }
}

// Non-memory target operations (non-MEM_P path)
int non_memory_target_ops(const int* arr, int lo, int hi) {
    int result = 0;
    int count = hi - lo + 1;
    
    // Operation where target is register (non-MEM_P)
    if (count == 1) {
        // Single element access
        result = arr[lo];
    } else if (count == 2) {
        // Two elements combined
        result = arr[lo] + arr[hi];
    } else if (count > 2 && count <= 4) {
        // Multiple elements, still register target
        for (int i = lo; i <= hi; i++) {
            result += arr[i];
        }
    }
    
    return result;
}

// Test different element sizes
void test_different_sizes(int argc) {
    // Arrays of different types
    char char_arr[100] = {0};
    int int_arr[100] = {0};
    long long ll_arr[100] = {0};
    
    // Initialize with pattern
    for (int i = 0; i < 100; i++) {
        char_arr[i] = i % 256;
        int_arr[i] = i;
        ll_arr[i] = i * 100LL;
    }
    
    // Use volatile pointers to inhibit optimization
    volatile char* vchar = char_arr;
    volatile int* vint = int_arr;
    volatile long long* vll = ll_arr;
    
    // Different constant bounds based on argc
    int lo, hi;
    
    if (argc == 1) {
        // Case 1: count = 1
        lo = get_lo_index(10);
        hi = get_hi_index(10, 0);  // count = 1
    } else if (argc == 2) {
        // Case 2: count = 2
        lo = get_lo_index(20);
        hi = get_hi_index(20, 1);  // count = 2
    } else if (argc == 3) {
        // Case 3: count = 3 (small type, small total size)
        lo = get_lo_index(30);
        hi = get_hi_index(30, 2);  // count = 3
    } else {
        // Case 4: count = 4 (larger type, larger total size)
        lo = get_lo_index(40);
        hi = get_hi_index(40, 3);  // count = 4
    }
    
    // Force bounds to be visible as constants
    if (__builtin_constant_p(lo) && __builtin_constant_p(hi)) {
        // This assertion helps ensure constant propagation
        asm volatile("" : : "r"(lo), "r"(hi));
    }
    
    // Test MEM_P path with char array (small element size)
    memory_target_ops(char_arr, (const char*)char_arr + 50, lo, hi);
    
    // Test MEM_P path with long long array (larger element size)
    if (hi < 90) {  // Ensure bounds are valid
        volatile long long* dest = ll_arr;
        volatile const long long* src = ll_arr + 50;
        
        // Constant-sized copy that might trigger the condition
        int count = hi - lo + 1;
        if (count > 2) {
            // This may trigger TYPE_SIZE * count check
            for (int i = 0; i < count; i++) {
                dest[lo + i] = src[lo + i];
            }
        }
    }
    
    // Test non-MEM_P path
    int reg_result = non_memory_target_ops(int_arr, lo, hi);
    
    // Use results to prevent dead code elimination
    vchar[0] = (char)reg_result;
    vint[0] = reg_result;
    
    // Create observable side effect
    printf("Result: %d, Char[0]=%d, Int[0]=%d\n", 
           reg_result, char_arr[0], int_arr[0]);
}

// Additional test with template (C++ mode) for better constant propagation
#ifdef __cplusplus
template<int LO, int HI>
struct ConstantBounds {
    static void copy_range(char* dest, const char* src) {
        constexpr int count = HI - LO + 1;
        // This should generate constant bounds
        for (int i = 0; i < count; i++) {
            dest[LO + i] = src[LO + i];
        }
    }
};
#endif

int main(int argc, char** argv) {
    // Test with different argc values to explore multiple paths
    test_different_sizes(argc);
    
    // Additional constant-bound tests
    char buffer1[100], buffer2[100];
    
    // Initialize buffers
    for (int i = 0; i < 100; i++) {
        buffer1[i] = i;
        buffer2[i] = 100 - i;
    }
    
    // Test with explicit constant bounds
    const int c_lo = 5;
    const int c_hi = 7;  // count = 3
    
    // Use volatile to inhibit early constant folding
    volatile int v_idx = c_lo;
    
    // This should still have constant bounds visible to middle-end
    int count = c_hi - c_lo + 1;
    for (int i = 0; i < count; i++) {
        buffer2[c_lo + i] = buffer1[c_lo + i];
    }
    
    // Test with builtin memcpy and constant size
    if (argc > 1) {
        // Constant size that's > 2
        __builtin_memcpy(buffer1 + 10, buffer2 + 10, 3 * sizeof(char));
        
        // Larger constant size with small element type
        __builtin_memcpy(buffer1 + 20, buffer2 + 20, 16 * sizeof(char));
    }
    
    // Return value based on array contents to ensure code isn't dead
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        sum += buffer1[i] + buffer2[i];
    }
    
    return sum % 256;
}

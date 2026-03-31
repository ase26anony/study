#include <stddef.h>
#include <stdio.h>
#include <string.h>

// Helper to create compile-time constant indices
#define CONST_IDX(n) (__builtin_constant_p(n) ? (n) : (n))

// Volatile wrapper to inhibit early constant folding
static volatile int volatile_seed = 0;

// Memory target operations (MEM_P path)
void mem_target_ops(char *dest, const char *src, int lo, int hi) {
    int count = hi - lo + 1;
    
    // Path 1: count <= 2 for MEM_P target
    if (count <= 2) {
        // This should trigger MEM_P path with count <= 2
        for (int i = lo; i <= hi; ++i) {
            dest[i] = src[i];
        }
    }
    // Path 2: count > 2 but small total size
    else if (count == 3) {
        // Small element type (char) * count = 3 bytes
        __builtin_memcpy(dest + lo, src + lo, 3);
    }
    // Path 3: count > 2 with larger total size
    else if (count == 4) {
        // Larger element type (long long) * count = 32 bytes
        long long *ldest = (long long *)dest;
        const long long *lsrc = (const long long *)src;
        for (int i = lo; i <= hi; ++i) {
            ldest[i] = lsrc[i];
        }
    }
}

// Non-memory target operations (non-MEM_P path)
int non_mem_target_ops(const int *arr, int lo, int hi) {
    int result = 0;
    
    // This should trigger non-MEM_P path
    for (int i = lo; i <= hi; ++i) {
        result += arr[i];  // Target is register, not memory
    }
    
    return result;
}

// Template-like function to generate constant bounds
static inline int get_constant_index(int idx) {
    // Use volatile to prevent constant folding too early
    return idx + (volatile_seed * 0);  // volatile_seed*0 = 0, but volatile access
}

int main(int argc, char *argv[]) {
    // Arrays of different types
    char char_arr1[100] = {0};
    char char_arr2[100] = {0};
    int int_arr[100] = {0};
    long long ll_arr[100] = {0};
    
    // Initialize arrays
    for (int i = 0; i < 100; ++i) {
        char_arr1[i] = i % 26 + 'a';
        int_arr[i] = i * 2;
        ll_arr[i] = i * 1000LL;
    }
    
    // Use argc to select different paths
    int test_case = argc > 1 ? argv[1][0] - '0' : 0;
    
    // Constant bounds using different methods
    constexpr int const_lo = 5;
    constexpr int const_hi = 6;  // count = 2
    
    // More constant bounds
    const int const_lo2 = 10;
    const int const_hi2 = 12;    // count = 3
    
    // Volatile-qualified pointers to inhibit optimization
    volatile char *v_char1 = char_arr1;
    volatile char *v_char2 = char_arr2;
    volatile int *v_int = int_arr;
    volatile long long *v_ll = ll_arr;
    
    int result = 0;
    
    switch (test_case) {
        case 0:
            // Test 1: MEM_P path with count = 2 (char array)
            // Use constant bounds directly
            mem_target_ops(char_arr2, char_arr1, CONST_IDX(5), CONST_IDX(6));
            result = char_arr2[5] + char_arr2[6];
            break;
            
        case 1:
            // Test 2: MEM_P path with count = 1
            mem_target_ops(char_arr2, char_arr1, CONST_IDX(7), CONST_IDX(7));
            result = char_arr2[7];
            break;
            
        case 2:
            // Test 3: MEM_P path with count = 3 (small total size)
            mem_target_ops(char_arr2, char_arr1, 
                          get_constant_index(10), 
                          get_constant_index(12));
            result = char_arr2[10] + char_arr2[11] + char_arr2[12];
            break;
            
        case 3:
            // Test 4: MEM_P path with count = 4 (larger total size)
            // Cast to different type to change element size
            mem_target_ops((char *)ll_arr, (const char *)char_arr1,
                          CONST_IDX(20), CONST_IDX(23));
            result = (int)ll_arr[20];
            break;
            
        case 4:
            // Test 5: Non-MEM_P path (register target)
            result = non_mem_target_ops(int_arr, CONST_IDX(30), CONST_IDX(35));
            break;
            
        case 5:
            // Test 6: Mixed operations
            // First do memory operation
            mem_target_ops(char_arr2, char_arr1, const_lo, const_hi);
            // Then non-memory operation
            result = non_mem_target_ops(int_arr, const_lo2, const_hi2);
            result += char_arr2[const_lo];
            break;
            
        default:
            // Test 7: Complex case with multiple conditions
            for (int i = 0; i < 4; ++i) {
                int lo = i * 10;
                int hi = lo + (i % 3);  // Varying count: 1, 2, or 3
                
                if (i % 2 == 0) {
                    // MEM_P path
                    mem_target_ops(char_arr2 + lo, char_arr1 + lo, 0, hi - lo);
                } else {
                    // Non-MEM_P path
                    result += non_mem_target_ops(int_arr + lo, 0, hi - lo);
                }
            }
            break;
    }
    
    // Force side effects to prevent dead code elimination
    __asm__ volatile ("" : : "r"(char_arr2), "r"(int_arr), "r"(ll_arr));
    
    // Use result to ensure code has observable effect
    printf("Result: %d\n", result);
    
    return result != 0;
}

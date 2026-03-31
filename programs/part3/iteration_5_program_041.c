#include <stdio.h>
#include <string.h>

// Helper to generate constant indices via templates
template<int N>
struct ConstIndex {
    static constexpr int value = N;
};

// Volatile wrapper to inhibit early constant folding
volatile int volatile_seed = 0;

// Memory target operations (MEM_P path)
void memory_target_operations(char* dest, const char* src, int lo, int hi) {
    int count = hi - lo + 1;
    
    // Case 1: count <= 2 (should take MEM_P && count <= 2 path)
    if (count <= 2) {
        // Small constant-sized copy
        for (int i = lo; i <= hi; ++i) {
            dest[i] = src[i];
        }
    }
    // Case 2: count > 2 but small total size
    else if (count == 3) {
        // Use builtin memcpy with constant size
        __builtin_memcpy(&dest[lo], &src[lo], 3 * sizeof(char));
    }
}

// Non-memory target operations (non-MEM_P path)
int non_memory_target_operations(const int* arr, int lo, int hi) {
    int result = 0;
    
    // This creates a non-MEM_P target (register accumulation)
    for (int i = lo; i <= hi; ++i) {
        result += arr[i];
    }
    
    return result;
}

int main(int argc, char* argv[]) {
    // Arrays of different types
    char char_arr[100] = {0};
    int int_arr[100] = {0};
    long long ll_arr[100] = {0};
    
    // Initialize with some values
    for (int i = 0; i < 100; ++i) {
        char_arr[i] = i % 26 + 'a';
        int_arr[i] = i * 2;
        ll_arr[i] = i * 1000LL;
    }
    
    // Use volatile pointers to inhibit optimization
    volatile char* v_char_arr = char_arr;
    volatile int* v_int_arr = int_arr;
    volatile long long* v_ll_arr = ll_arr;
    
    // Result accumulator to ensure code isn't dead
    int total_result = 0;
    
    // Different constant index pairs based on argc
    int lo_index, hi_index;
    
    if (argc == 1) {
        // Case 1: count = 1 (single element)
        lo_index = ConstIndex<5>::value;
        hi_index = ConstIndex<5>::value;
    } else if (argc == 2) {
        // Case 2: count = 2
        lo_index = ConstIndex<10>::value;
        hi_index = ConstIndex<11>::value;
    } else if (argc == 3) {
        // Case 3: count = 3 with char (small total size)
        lo_index = ConstIndex<20>::value;
        hi_index = ConstIndex<22>::value;
    } else {
        // Case 4: count = 4 with long long (larger total size)
        lo_index = ConstIndex<30>::value;
        hi_index = ConstIndex<33>::value;
    }
    
    // Add volatile to prevent constant propagation too early
    lo_index += volatile_seed - volatile_seed;
    hi_index += volatile_seed - volatile_seed;
    
    // Verify indices are compile-time constants
    if (__builtin_constant_p(lo_index) && __builtin_constant_p(hi_index)) {
        // Force the compiler to consider this path
        asm volatile("" : : "r"(lo_index), "r"(hi_index));
    }
    
    // Memory target operations (MEM_P path)
    char char_dest[100] = {0};
    memory_target_operations(char_dest, char_arr, lo_index, hi_index);
    
    // Non-memory target operations (non-MEM_P path)
    total_result += non_memory_target_operations(int_arr, lo_index, hi_index);
    
    // Additional test with different element sizes
    if (argc > 2) {
        // Test with short array (2 bytes per element)
        short short_arr[100];
        for (int i = 0; i < 100; ++i) short_arr[i] = i;
        
        // This should trigger count > 2 path with TYPE_SIZE consideration
        short short_dest[100];
        int count = hi_index - lo_index + 1;
        if (count > 2) {
            // Force array slice copy
            for (int i = lo_index; i <= hi_index; ++i) {
                short_dest[i] = short_arr[i];
            }
            total_result += short_dest[lo_index];
        }
    }
    
    // Use conditional to ensure multiple paths are considered
    if (argc % 2 == 0) {
        // Alternative constant indices
        constexpr int alt_lo = 40;
        constexpr int alt_hi = 42;
        
        // Array initialization with constant bounds
        int temp[3];
        for (int i = alt_lo; i <= alt_hi; ++i) {
            temp[i - alt_lo] = int_arr[i] * 2;
        }
        total_result += temp[0];
    }
    
    // Loop with constant bounds that might be unrolled
    constexpr int loop_start = 50;
    constexpr int loop_end = 53;
    
    int loop_sum = 0;
    for (int i = loop_start; i <= loop_end; ++i) {
        loop_sum += int_arr[i];
    }
    total_result += loop_sum;
    
    // Ensure all operations have observable effects
    printf("Result: %d\n", total_result);
    
    // Return value based on modified arrays
    return char_dest[lo_index] + total_result % 256;
}

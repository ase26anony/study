#include <stdio.h>
#include <string.h>

// Helper function to force constant indices through templates
template<int N>
struct Constant {
    static constexpr int value = N;
};

// Volatile wrapper to prevent early constant folding
volatile int volatile_seed = 0;

// Memory target operations (MEM_P path)
void memory_target_operations(char* dest, const char* src, int lo, int hi) {
    int count = hi - lo + 1;
    
    // Case 1: count <= 2 (should take first branch)
    if (count <= 2) {
        // Use volatile to prevent optimization
        volatile int vlo = lo;
        volatile int vhi = hi;
        
        // Memory copy of small range
        for (int i = vlo; i <= vhi; i++) {
            dest[i] = src[i];
        }
    }
    // Case 2: count > 2 but small total size
    else if (count <= 4) {
        // Small type (char) * count <= something
        __builtin_memcpy(dest + lo, src + lo, count * sizeof(char));
    }
}

// Non-memory target operations (non-MEM_P path)
int non_memory_target_operations(const int* arr, int lo, int hi) {
    int result = 0;
    
    // This creates a register target (non-MEM_P)
    for (int i = lo; i <= hi; i++) {
        result += arr[i];  // Result goes to register
    }
    
    return result;
}

int main(int argc, char* argv[]) {
    // Arrays of different types
    char char_arr1[100] = {0};
    char char_arr2[100] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int int_arr[100] = {0};
    long long ll_arr[100] = {0};
    
    // Initialize with pattern
    for (int i = 0; i < 100; i++) {
        int_arr[i] = i;
        ll_arr[i] = i * 100LL;
    }
    
    // Use argc to create different control flow paths
    int path = argc > 1 ? (argv[1][0] - '0') : 0;
    
    // Different constant index pairs based on path
    int lo_index, hi_index;
    
    switch (path % 4) {
        case 0:
            // count = 1
            lo_index = Constant<5>::value;
            hi_index = Constant<5>::value;
            break;
        case 1:
            // count = 2
            lo_index = Constant<10>::value;
            hi_index = Constant<11>::value;
            break;
        case 2:
            // count = 3, small type (char)
            lo_index = Constant<20>::value;
            hi_index = Constant<22>::value;
            break;
        case 3:
            // count = 4, larger type (int)
            lo_index = Constant<30>::value;
            hi_index = Constant<33>::value;
            break;
    }
    
    // Add volatile offset to prevent complete constant folding
    volatile int volatile_offset = volatile_seed;
    lo_index += volatile_offset - volatile_offset;  // Net zero, but not const-folded early
    hi_index += volatile_offset - volatile_offset;
    
    // Verify indices are constant at compile time (for debugging)
    if (__builtin_constant_p(lo_index) && __builtin_constant_p(hi_index)) {
        // This should be true for our constant indices
    }
    
    // Test MEM_P path with different counts
    memory_target_operations(char_arr1, char_arr2, lo_index, hi_index);
    
    // Test non-MEM_P path
    int sum = non_memory_target_operations(int_arr, lo_index, hi_index);
    
    // Also test with larger type and count > 2
    if (path == 3) {
        // This should test the TYPE_SIZE * count path
        long long ll_sum = 0;
        for (int i = lo_index; i <= hi_index; i++) {
            ll_sum += ll_arr[i];
        }
        
        // Force memory operation with larger type
        long long ll_temp[4];
        __builtin_memcpy(ll_temp, &ll_arr[lo_index], 4 * sizeof(long long));
    }
    
    // Create observable side effects
    int result = 0;
    for (int i = 0; i < 10; i++) {
        result += char_arr1[i] + int_arr[i];
    }
    
    // Use result to prevent dead code elimination
    printf("Result: %d (sum: %d)\n", result, sum);
    
    return result > 100 ? 0 : 1;
}

#include <stdio.h>
#include <string.h>

// Helper function to create compile-time constant indices
static inline constexpr int const_idx(int base) {
    return base * 2;
}

// Template to generate constant bounds
template<int N>
struct ConstantBounds {
    static constexpr int lo = N;
    static constexpr int hi = N + 3;  // count = 4 > 2
};

// Different scenarios for testing
enum TestCase {
    MEM_SMALL_COUNT,      // MEM_P && count <= 2
    MEM_LARGE_COUNT_SMALL, // MEM_P && count > 2 && small total size
    MEM_LARGE_COUNT_LARGE, // MEM_P && count > 2 && large total size
    NON_MEM_TARGET,       // !MEM_P
    MIXED_SCENARIOS       // Combination of above
};

// Force memory operations through volatile pointers
static char char_array[100] = {0};
static int int_array[100] = {0};
static long long ll_array[100] = {0};

// Volatile pointers to inhibit early optimization
volatile char* v_char_ptr = char_array;
volatile int* v_int_ptr = int_array;
volatile long long* v_ll_ptr = ll_array;

// Test memory target with count <= 2 (path: MEM_P && count <= 2)
void test_mem_small_count(int lo, int hi) {
    // Access small subrange (count = 2)
    for (int i = lo; i <= hi; i++) {
        char_array[i] = char_array[i + 10];
    }
}

// Test memory target with count > 2 but small element size
void test_mem_large_count_small(int lo, int hi) {
    // char type: TYPE_SIZE = 8 bits, count = 4, total = 32 bits
    // This should trigger the TYPE_SIZE * count calculation
    for (int i = lo; i <= hi; i++) {
        char_array[i] = 'A' + (i - lo);
    }
}

// Test memory target with count > 2 and large element size
void test_mem_large_count_large(int lo, int hi) {
    // long long type: TYPE_SIZE = 64 bits, count = 4, total = 256 bits
    for (int i = lo; i <= hi; i++) {
        ll_array[i] = ll_array[i + 20] + 1;
    }
}

// Test non-memory target (!MEM_P)
int test_non_mem_target(int lo, int hi) {
    // Compute value from subrange - target is register, not memory
    int sum = 0;
    for (int i = lo; i <= hi; i++) {
        sum += int_array[i];  // This loads but target is sum (register)
    }
    return sum;
}

// Use __builtin_memcpy with constant size
void test_builtin_memcpy(int lo, int hi) {
    int count = hi - lo + 1;
    if (count == 2) {
        // Small count
        __builtin_memcpy(&char_array[lo], &char_array[lo + 30], 2);
    } else if (count == 4) {
        // Larger count but with char type
        __builtin_memcpy(&char_array[lo], &char_array[lo + 40], 4);
    }
}

int main(int argc, char* argv[]) {
    // Initialize arrays
    for (int i = 0; i < 100; i++) {
        char_array[i] = i % 26 + 'A';
        int_array[i] = i * 2;
        ll_array[i] = i * 100LL;
    }
    
    // Use argc to select different test cases
    TestCase test_case = (TestCase)(argc % 5);
    
    // Generate constant indices using different methods
    int lo_index, hi_index;
    
    switch (test_case) {
        case MEM_SMALL_COUNT:
            // Constant bounds: lo=5, hi=6, count=2
            lo_index = const_idx(2) + 1;  // Compile-time constant: 5
            hi_index = 6;                 // Direct constant
            test_mem_small_count(lo_index, hi_index);
            break;
            
        case MEM_LARGE_COUNT_SMALL:
            // Constant bounds: lo=10, hi=13, count=4
            lo_index = ConstantBounds<10>::lo;  // Template constant
            hi_index = ConstantBounds<10>::hi;
            test_mem_large_count_small(lo_index, hi_index);
            break;
            
        case MEM_LARGE_COUNT_LARGE:
            // Constant bounds: lo=20, hi=23, count=4
            lo_index = 20;
            hi_index = 23;
            test_mem_large_count_large(lo_index, hi_index);
            break;
            
        case NON_MEM_TARGET:
            // Constant bounds: lo=30, hi=33, count=4
            lo_index = 30;
            hi_index = 33;
            {
                int result = test_non_mem_target(lo_index, hi_index);
                // Use result to prevent dead code elimination
                char_array[0] = result & 0xFF;
            }
            break;
            
        case MIXED_SCENARIOS:
            // Test multiple scenarios in one execution
            test_mem_small_count(40, 41);
            test_mem_large_count_small(42, 45);
            test_builtin_memcpy(46, 49);
            {
                int result = test_non_mem_target(50, 53);
                char_array[1] = result & 0xFF;
            }
            break;
    }
    
    // Additional test with volatile pointers to inhibit optimization
    volatile int* volatile_ptr = int_array;
    int volatile_idx = const_idx(3);  // Should be constant 6
    
    // This should generate tree nodes with constant bounds
    // but through volatile access
    for (int i = volatile_idx; i <= volatile_idx + 1; i++) {
        // Volatile write - forces memory operations
        *(volatile_ptr + i) = *(volatile_ptr + i + 10) + 1;
    }
    
    // Use __builtin_constant_p to verify constant propagation
    if (__builtin_constant_p(const_idx(5))) {
        char_array[2] = 'C';  // Mark as constant
    }
    
    // Ensure all code has observable effects
    int checksum = 0;
    for (int i = 0; i < 10; i++) {
        checksum += char_array[i] + int_array[i] + (int)(ll_array[i] & 0xFF);
    }
    
    printf("Checksum: %d\n", checksum);
    return checksum != 0 ? 0 : 1;
}

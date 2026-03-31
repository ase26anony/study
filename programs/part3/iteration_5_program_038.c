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
    static constexpr int hi = N + 3;  // count = 4
};

// Different scenarios to test various paths
enum TestCase {
    CASE_MEM_COUNT_LE_2,
    CASE_MEM_COUNT_GT_2_SMALL,
    CASE_MEM_COUNT_GT_2_LARGE,
    CASE_NON_MEM
};

// Use volatile to prevent early constant folding
volatile int g_selector = 0;

int main(int argc, char *argv[]) {
    // Arrays of different types to test different TYPE_SIZE scenarios
    static char arr_char[100];
    static short arr_short[100];
    static int arr_int[100];
    static long long arr_llong[100];
    
    // Volatile pointers to inhibit optimization
    volatile char *v_char = arr_char;
    volatile short *v_short = arr_short;
    volatile int *v_int = arr_int;
    volatile long long *v_llong = arr_llong;
    
    // Initialize arrays
    for (int i = 0; i < 100; i++) {
        arr_char[i] = i % 128;
        arr_short[i] = i * 2;
        arr_int[i] = i * 3;
        arr_llong[i] = i * 5LL;
    }
    
    // Use argc to select different test cases
    TestCase test_case = (TestCase)(argc % 4);
    
    // Result accumulator to ensure code isn't dead
    int result = 0;
    
    // Test different scenarios
    switch (test_case) {
        case CASE_MEM_COUNT_LE_2: {
            // Memory target with count <= 2
            // Use constexpr function to generate constant bounds
            const int lo = const_idx(5);      // lo = 10
            const int hi = const_idx(5) + 1;  // hi = 11, count = 2
            
            // Memory-to-memory copy of 2 elements
            // This should trigger MEM_P(target) && count <= 2 path
            for (int i = lo; i <= hi; i++) {
                v_char[i] = v_char[i + 10];
            }
            
            // Also test with different types
            v_short[lo] = v_short[lo + 5];
            v_short[hi] = v_short[hi + 5];
            
            result = v_char[lo] + v_short[hi];
            break;
        }
        
        case CASE_MEM_COUNT_GT_2_SMALL: {
            // Memory target with count > 2 but small total size
            // Using char array: TYPE_SIZE = 8 bits, count = 4, total = 32 bits
            const int lo = 20;
            const int hi = 23;  // count = 4
            
            // This should go into the MEM_P(target) && count > 2 path
            // with TYPE_SIZE * count being checked
            for (int i = lo; i <= hi; i++) {
                v_char[i] = v_char[i + 20];
            }
            
            // Use template to ensure compile-time constants
            constexpr int t_lo = ConstantBounds<30>::lo;
            constexpr int t_hi = ConstantBounds<30>::hi;
            
            // Another memory copy with template constants
            for (int i = t_lo; i <= t_hi; i++) {
                v_char[i] = v_char[i + 30];
            }
            
            result = v_char[lo] + v_char[t_hi];
            break;
        }
        
        case CASE_MEM_COUNT_GT_2_LARGE: {
            // Memory target with count > 2 and larger total size
            // Using long long array: TYPE_SIZE = 64 bits, count = 4, total = 256 bits
            const int lo = 40;
            const int hi = 43;  // count = 4
            
            // This tests the MEM_P(target) && count > 2 path with larger type
            for (int i = lo; i <= hi; i++) {
                v_llong[i] = v_llong[i + 10];
            }
            
            // Mix with builtin memcpy with constant size
            // Size = 4 * sizeof(long long) = 32 bytes
            if (__builtin_constant_p(hi - lo + 1)) {
                __builtin_memcpy(&arr_llong[50], &arr_llong[lo], 
                                (hi - lo + 1) * sizeof(long long));
            }
            
            result = (int)v_llong[lo] + (int)v_llong[50];
            break;
        }
        
        case CASE_NON_MEM: {
            // Non-memory target (register operations)
            const int lo = 60;
            const int hi = 63;  // count = 4
            
            // Compute values into registers (non-MEM_P target)
            int sum = 0;
            for (int i = lo; i <= hi; i++) {
                sum += v_int[i];  // This loads into register
            }
            
            // Another non-memory operation
            long long product = 1;
            for (int i = lo; i <= hi; i++) {
                product *= v_llong[i];  // Register operations
            }
            
            // Use the results
            v_int[0] = sum;
            v_llong[1] = product;
            
            result = sum + (int)product;
            break;
        }
    }
    
    // Additional test: conditional based on volatile to force path analysis
    volatile int cond = g_selector;
    if (cond > 0) {
        // Single element access (count = 1)
        const int idx = const_idx(8);  // idx = 16
        v_int[idx] = v_int[idx + 1] + v_int[idx + 2];
        result += v_int[idx];
    } else {
        // Two element access (count = 2)
        const int lo2 = 70;
        const int hi2 = 71;  // count = 2
        for (int i = lo2; i <= hi2; i++) {
            v_int[i] = v_int[i + 10] * 2;
        }
        result += v_int[lo2] + v_int[hi2];
    }
    
    // Use __builtin_constant_p to verify constant propagation
    const int verify_lo = 80;
    const int verify_hi = 83;
    if (__builtin_constant_p(verify_hi - verify_lo + 1)) {
        // This block should be reachable
        for (int i = verify_lo; i <= verify_hi; i++) {
            arr_char[i] = i;
        }
        result += arr_char[verify_lo];
    }
    
    // Ensure all operations have observable effects
    printf("Result: %d\n", result);
    
    // Return value based on array contents to prevent dead code elimination
    return (arr_char[10] + arr_int[20] + (int)arr_llong[30]) % 256;
}

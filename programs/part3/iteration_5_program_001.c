#include <stdio.h>
#include <string.h>

// Helper to create compile-time constant indices
template<int N>
constexpr int const_index() { return N; }

// Volatile wrapper to inhibit early constant folding
static volatile int volatile_seed = 0;

int main(int argc, char *argv[]) {
    // Arrays of different types and sizes
    static char char_arr[100] = {0};
    static int int_arr[100] = {0};
    static long long ll_arr[100] = {0};
    
    // Volatile pointers to prevent optimization
    volatile char *v_char = char_arr;
    volatile int *v_int = int_arr;
    volatile long long *v_ll = ll_arr;
    
    // Initialize with some values
    for (int i = 0; i < 100; i++) {
        char_arr[i] = i;
        int_arr[i] = i * 2;
        ll_arr[i] = i * 3LL;
    }
    
    // Result accumulator to prevent dead code elimination
    int result = 0;
    
    // Use argc to select different paths
    if (argc > 1) {
        // Path 1: MEM_P(target) with count <= 2
        // Access subrange with constant bounds
        const int lo1 = const_index<5>();
        const int hi1 = const_index<6>();  // count = 2
        
        // Memory-to-memory copy (triggers MEM_P path)
        for (int i = lo1; i <= hi1; i++) {
            v_char[i] = v_char[i + 10];
        }
        
        // Also test with int array
        const int lo2 = const_index<20>();
        const int hi2 = const_index<20>();  // count = 1
        
        int temp[1];
        for (int i = lo2; i <= hi2; i++) {
            temp[i - lo2] = v_int[i];
        }
        result += temp[0];
    }
    
    if (argc > 2) {
        // Path 2: MEM_P(target) with count > 2 but small total size
        // char array: TYPE_SIZE = 1, count = 3, total = 3
        const int lo3 = const_index<30>();
        const int hi3 = const_index<32>();  // count = 3
        
        // Use __builtin_memcpy with constant size
        char temp_buf[3];
        __builtin_memcpy(temp_buf, &char_arr[lo3], 3);
        
        // Memory-to-memory with volatile
        for (int i = lo3; i <= hi3; i++) {
            v_char[i + 20] = temp_buf[i - lo3];
        }
    }
    
    if (argc > 3) {
        // Path 3: MEM_P(target) with count > 2 and larger total size
        // long long array: TYPE_SIZE = 8, count = 4, total = 32
        const int lo4 = const_index<40>();
        const int hi4 = const_index<43>();  // count = 4
        
        // This should trigger the TYPE_SIZE * count check
        long long temp_ll[4];
        for (int i = lo4; i <= hi4; i++) {
            temp_ll[i - lo4] = v_ll[i];
        }
        
        // Copy back to different location
        for (int i = lo4; i <= hi4; i++) {
            v_ll[i + 10] = temp_ll[i - lo4];
        }
    }
    
    if (argc > 4) {
        // Path 4: Non-MEM_P(target) - register operations
        const int lo5 = const_index<50>();
        const int hi5 = const_index<53>();  // count = 4
        
        // Compute sum in registers (non-MEM_P target)
        int sum = 0;
        for (int i = lo5; i <= hi5; i++) {
            sum += v_int[i];
        }
        
        // Use the result
        result += sum;
        
        // Another non-MEM_P example with multiplication
        const int lo6 = const_index<60>();
        const int hi6 = const_index<61>();  // count = 2
        
        long long product = 1;
        for (int i = lo6; i <= hi6; i++) {
            product *= v_ll[i];
        }
        result += (int)(product % 1000);
    }
    
    // Mixed scenario with volatile index to force analysis
    if (argc > 5) {
        volatile int dynamic = volatile_seed;
        
        // Constant bounds but accessed through volatile pointer
        const int lo7 = const_index<70>();
        const int hi7 = const_index<73>();  // count = 4
        
        if (dynamic == 0) {
            // Use as memory target
            for (int i = lo7; i <= hi7; i++) {
                v_int[i] = v_int[i + 20];
            }
        } else {
            // Use as register target
            int acc = 0;
            for (int i = lo7; i <= hi7; i++) {
                acc ^= v_int[i];
            }
            result += acc;
        }
    }
    
    // Verify constant propagation with __builtin_constant_p
    const int test_lo = const_index<80>();
    const int test_hi = const_index<82>();
    
    if (__builtin_constant_p(test_lo) && __builtin_constant_p(test_hi)) {
        // This should always be true
        result += (test_hi - test_lo + 1);  // Add count
    }
    
    // Ensure all operations have observable effects
    for (int i = 0; i < 10; i++) {
        result += char_arr[i] + int_arr[i] + (int)ll_arr[i];
    }
    
    printf("Result: %d\n", result);
    return result != 0;
}

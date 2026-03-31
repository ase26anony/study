#include <stdio.h>
#include <string.h>

// Helper function to create compile-time constant indices
static inline int const_idx(int x) {
    return __builtin_constant_p(x) ? x : x;
}

// Template-like function using _Generic to select different operations
#define SELECT_OPERATION(type, arr, lo, hi, target_type) \
    _Generic((target_type), \
        int:   copy_to_memory_##type(arr, lo, hi), \
        void*: copy_to_register_##type(arr, lo, hi) \
    )

// Memory target operations (MEM_P(target) should be true)
static void copy_to_memory_char(volatile char* arr, int lo, int hi) {
    volatile char dest[10];
    // count = hi - lo + 1
    // For char: TYPE_SIZE = 8 bits, count <= 2 triggers one path
    // For count > 2 but small total size triggers another
    for (int i = lo; i <= hi; i++) {
        dest[i - lo] = arr[i];
    }
    // Use volatile to prevent optimization
    asm volatile("" : : "r"(dest) : "memory");
}

static void copy_to_memory_int(volatile int* arr, int lo, int hi) {
    volatile int dest[10];
    // For int (likely 32-bit), TYPE_SIZE * count might be larger
    for (int i = lo; i <= hi; i++) {
        dest[i - lo] = arr[i];
    }
    asm volatile("" : : "r"(dest) : "memory");
}

// Non-memory target operations (!MEM_P(target) should be true)
static int copy_to_register_char(volatile char* arr, int lo, int hi) {
    int result = 0;
    // This creates a non-memory target (register)
    for (int i = lo; i <= hi; i++) {
        result += arr[i];
    }
    return result;
}

static long long copy_to_register_longlong(volatile long long* arr, int lo, int hi) {
    long long result = 0;
    for (int i = lo; i <= hi; i++) {
        result ^= arr[i];  // Use XOR to prevent simple optimization
    }
    return result;
}

// Force constant evaluation through templates/constexpr-like macros
#define CONST_EVAL(expr) (__builtin_constant_p(expr) ? (expr) : (expr))

// Main test function
int main(int argc, char* argv[]) {
    // Declare arrays of different types
    volatile char char_arr[100];
    volatile int int_arr[100];
    volatile long long ll_arr[100];
    
    // Initialize arrays with non-constant values to prevent pre-computation
    for (int i = 0; i < 100; i++) {
        char_arr[i] = (i * 13) & 0xFF;
        int_arr[i] = i * 17;
        ll_arr[i] = (long long)i * 23;
    }
    
    int result = 0;
    
    // Use argc to select different paths
    switch (argc) {
        case 1: {
            // Test 1: count = 1 (single element, MEM_P path)
            // lo_index = 5, hi_index = 5, count = 1
            const int lo = CONST_EVAL(5);
            const int hi = CONST_EVAL(5);
            copy_to_memory_char(char_arr, lo, hi);
            result += 1;
            break;
        }
        
        case 2: {
            // Test 2: count = 2 (two elements, MEM_P path)
            // lo_index = 10, hi_index = 11, count = 2
            const int lo = CONST_EVAL(10);
            const int hi = CONST_EVAL(11);
            copy_to_memory_int(int_arr, lo, hi);
            result += 2;
            break;
        }
        
        case 3: {
            // Test 3: count > 2 with small element size (char)
            // lo_index = 20, hi_index = 25, count = 6
            // TYPE_SIZE(char) * count = 8 * 6 = 48 bits = 6 bytes
            const int lo = CONST_EVAL(20);
            const int hi = CONST_EVAL(25);
            copy_to_memory_char(char_arr, lo, hi);
            result += 3;
            break;
        }
        
        case 4: {
            // Test 4: count > 2 with large element size (long long)
            // lo_index = 30, hi_index = 35, count = 6
            // TYPE_SIZE(long long) * count = 64 * 6 = 384 bits = 48 bytes
            const int lo = CONST_EVAL(30);
            const int hi = CONST_EVAL(35);
            volatile long long dest[6];
            for (int i = lo; i <= hi; i++) {
                dest[i - lo] = ll_arr[i];
            }
            asm volatile("" : : "r"(dest) : "memory");
            result += 4;
            break;
        }
        
        case 5: {
            // Test 5: Non-MEM_P target with count = 1
            const int lo = CONST_EVAL(40);
            const int hi = CONST_EVAL(40);
            result += copy_to_register_char(char_arr, lo, hi);
            break;
        }
        
        case 6: {
            // Test 6: Non-MEM_P target with count > 2
            const int lo = CONST_EVAL(50);
            const int hi = CONST_EVAL(55);
            result += (int)copy_to_register_longlong(ll_arr, lo, hi);
            break;
        }
        
        default: {
            // Test all conditions mixed
            // Mix of MEM_P and non-MEM_P targets
            const int lo1 = CONST_EVAL(60);
            const int hi1 = CONST_EVAL(60);  // count = 1
            copy_to_memory_char(char_arr, lo1, hi1);
            
            const int lo2 = CONST_EVAL(70);
            const int hi2 = CONST_EVAL(71);  // count = 2
            result += copy_to_register_char(char_arr, lo2, hi2);
            
            const int lo3 = CONST_EVAL(80);
            const int hi3 = CONST_EVAL(85);  // count = 6
            copy_to_memory_int(int_arr, lo3, hi3);
            
            const int lo4 = CONST_EVAL(90);
            const int hi4 = CONST_EVAL(95);  // count = 6
            result += (int)copy_to_register_longlong(ll_arr, lo4, hi4);
            break;
        }
    }
    
    // Use __builtin_memcpy with constant size for additional coverage
    if (argc > 1) {
        volatile char src[10] = {1,2,3,4,5,6,7,8,9,10};
        volatile char dst[10];
        
        // Different constant sizes to trigger different paths
        if (argc % 2 == 0) {
            // Small copy (2 elements)
            __builtin_memcpy((void*)dst, (const void*)src, 2 * sizeof(char));
        } else {
            // Larger copy (6 elements)
            __builtin_memcpy((void*)dst, (const void*)src, 6 * sizeof(char));
        }
        asm volatile("" : : "r"(dst) : "memory");
    }
    
    // Ensure result is used
    printf("Result: %d\n", result);
    
    return result & 0xFF;  // Return non-zero to ensure all code paths matter
}

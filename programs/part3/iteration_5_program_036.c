#include <stdio.h>
#include <string.h>

// Helper to create compile-time constants
#define CONST_LO 5
#define CONST_HI 6
#define CONST_LO2 10
#define CONST_HI2 15

// Volatile pointers to inhibit early constant folding
volatile int *volatile_ptr_int;
volatile char *volatile_ptr_char;
volatile long long *volatile_ptr_llong;

// Memory target operations (MEM_P(target) path)
void memory_target_operations(int argc) {
    static char arr_char[100];
    static int arr_int[100];
    static long long arr_llong[100];
    
    volatile_ptr_char = arr_char;
    volatile_ptr_int = arr_int;
    volatile_ptr_llong = arr_llong;
    
    // Scenario 1: count <= 2 (count = 2)
    if (argc > 1) {
        // This should trigger count <= 2 path
        for (int i = CONST_LO; i <= CONST_HI; i++) {
            volatile_ptr_char[i] = (char)(i * 2);
        }
        
        // Verify constants are compile-time
        if (__builtin_constant_p(CONST_HI - CONST_LO + 1)) {
            // Force memory operation
            volatile_ptr_int[CONST_LO] = volatile_ptr_int[CONST_HI];
        }
    }
    
    // Scenario 2: count > 2 with small element size (char)
    if (argc > 2) {
        // count = 6, element size = 1 byte, total = 6 bytes
        for (int i = CONST_LO2; i <= CONST_HI2; i++) {
            volatile_ptr_char[i] = (char)(i % 256);
        }
        
        // Block copy with constant bounds
        char temp[6];
        for (int i = 0; i <= CONST_HI2 - CONST_LO2; i++) {
            temp[i] = volatile_ptr_char[CONST_LO2 + i];
        }
        
        // Copy back with different indices
        for (int i = 0; i <= CONST_HI2 - CONST_HI; i++) {
            volatile_ptr_char[CONST_HI + i] = temp[i];
        }
    }
    
    // Scenario 3: count > 2 with larger element size (long long)
    if (argc > 3) {
        // count = 6, element size = 8 bytes, total = 48 bytes
        for (int i = CONST_LO2; i <= CONST_HI2; i++) {
            volatile_ptr_llong[i] = (long long)i * 1000LL;
        }
        
        // Use __builtin_memcpy with constant size
        long long buffer[6];
        __builtin_memcpy(buffer, &volatile_ptr_llong[CONST_LO2], 
                         (CONST_HI2 - CONST_LO2 + 1) * sizeof(long long));
        
        // Modify and copy back
        buffer[0] = 9999LL;
        __builtin_memcpy(&volatile_ptr_llong[CONST_HI], buffer, 
                         (CONST_HI2 - CONST_LO2) * sizeof(long long));
    }
}

// Non-memory target operations (!MEM_P(target) path)
int non_memory_target_operations(int argc) {
    static int arr[100] = {0};
    volatile_ptr_int = arr;
    
    int result = 0;
    
    // Register target: arithmetic expression result
    if (argc > 4) {
        // Single element access (count = 1)
        int val1 = volatile_ptr_int[CONST_LO];
        int val2 = volatile_ptr_int[CONST_HI];
        
        // This creates a non-MEM_P target
        result = val1 * val2 + (CONST_HI - CONST_LOW + 1);
        
        // Multiple elements in expression (count = 2 conceptually)
        if (__builtin_constant_p(CONST_HI - CONST_LO)) {
            result += volatile_ptr_int[CONST_LO] + volatile_ptr_int[CONST_HI];
        }
    }
    
    // Larger count in expression context
    if (argc > 5) {
        // Sum of range (count = 6)
        int sum = 0;
        for (int i = CONST_LO2; i <= CONST_HI2; i++) {
            sum += volatile_ptr_int[i];
        }
        result += sum;
    }
    
    return result;
}

// Template to force constant bounds (C++ version available)
#ifdef __cplusplus
template<int LO, int HI>
struct ConstantBounds {
    static constexpr int lo = LO;
    static constexpr int hi = HI;
    static constexpr int count = hi - lo + 1;
    
    template<typename T>
    static void copy_range(volatile T* dest, volatile T* src) {
        for (int i = lo; i <= hi; i++) {
            dest[i] = src[i];
        }
    }
};
#endif

int main(int argc, char *argv[]) {
    int ret_val = 0;
    
    // Execute both paths based on argc
    memory_target_operations(argc);
    ret_val = non_memory_target_operations(argc);
    
    // Force all code paths to be considered
    volatile int dummy = 0;
    
    // Use different constant bounds based on argc
    if (argc > 6) {
        // Very small range (count = 1)
        static short arr_short[100];
        volatile short *volatile_ptr_short = arr_short;
        
        volatile_ptr_short[7] = volatile_ptr_short[7];  // Self-assignment
        volatile_ptr_short[8] = 42;
        
        // Check constant propagation
        if (__builtin_constant_p(8 - 7 + 1)) {
            dummy = 1;
        }
    }
    
    if (argc > 7) {
        // Medium range with int (count = 4, element size = 4, total = 16)
        static int arr_mid[100];
        volatile int *volatile_ptr_mid = arr_mid;
        
        for (int i = 20; i <= 23; i++) {
            volatile_ptr_mid[i] = i * 100;
        }
        
        // Array section copy
        int temp[4];
        for (int i = 0; i < 4; i++) {
            temp[i] = volatile_ptr_mid[20 + i];
            volatile_ptr_mid[30 + i] = temp[i];
        }
    }
    
    // Ensure side effects
    printf("Result: %d\n", ret_val + dummy);
    
    return ret_val + dummy;
}

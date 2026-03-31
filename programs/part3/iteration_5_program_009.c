#include <stdio.h>
#include <string.h>

// Helper to generate compile-time constants
#define CONST_LO 5
#define CONST_HI 6
#define CONST_LO2 10
#define CONST_HI2 15

// Volatile pointers to inhibit constant folding
volatile int *volatile p_int;
volatile char *volatile p_char;
volatile long long *volatile p_llong;

// Memory target operations (MEM_P path)
void mem_target_operations(int argc) {
    static int src_arr[100] = {0};
    static int dst_arr[100] = {0};
    static char char_arr[100] = {0};
    static long long ll_arr[100] = {0};
    
    // Use volatile pointers to prevent early optimization
    volatile int *volatile vsrc = src_arr;
    volatile int *volatile vdst = dst_arr;
    volatile char *volatile vchar = char_arr;
    volatile long long *volatile vll = ll_arr;
    
    // Initialize with some values
    for (int i = 0; i < 100; i++) {
        src_arr[i] = i;
        char_arr[i] = i;
        ll_arr[i] = i;
    }
    
    // Path 1: count <= 2 (MEM_P path)
    if (argc > 1) {
        // Single element: count = 1
        vdst[CONST_LO] = vsrc[CONST_LO];
        
        // Two elements: count = 2
        vdst[CONST_LO] = vsrc[CONST_LO];
        vdst[CONST_HI] = vsrc[CONST_HI];
        
        // Using memcpy with small constant size (count = 2, element size = 4)
        __builtin_memcpy(&vdst[CONST_LO], &vsrc[CONST_LO], 2 * sizeof(int));
    }
    
    // Path 2: count > 2 but small total size (char array)
    if (argc > 2) {
        // count = 6, element size = 1, total size = 6
        for (int i = CONST_LO2; i <= CONST_HI2; i++) {
            vchar[i] = i;
        }
        
        // Use memcpy with constant bounds
        __builtin_memcpy(&vchar[CONST_LO2], &vchar[0], 
                        (CONST_HI2 - CONST_LO2 + 1) * sizeof(char));
    }
    
    // Path 3: count > 2 with larger type
    if (argc > 3) {
        // count = 6, element size = 8, total size = 48
        for (int i = CONST_LO2; i <= CONST_HI2; i++) {
            vll[i] = vll[i - CONST_LO2];
        }
    }
}

// Non-memory target operations (non-MEM_P path)
int non_mem_target_operations(int argc) {
    static int arr[100] = {0};
    volatile int *volatile varr = arr;
    
    // Initialize
    for (int i = 0; i < 100; i++) {
        arr[i] = i * 2;
    }
    
    int result = 0;
    
    // Non-MEM_P path: result goes to register
    if (argc > 4) {
        // Single element access
        result = varr[CONST_LO];
        
        // Multiple elements combined (count = 2)
        result = varr[CONST_LO] + varr[CONST_HI];
        
        // Larger count but still register target
        int sum = 0;
        for (int i = CONST_LO2; i <= CONST_HI2; i++) {
            sum += varr[i];
        }
        result += sum;
    }
    
    return result;
}

// Template to force constant bounds (C11 doesn't have constexpr, so use static)
static const int template_lo = 20;
static const int template_hi = 25;

// Function with constant bounds checking
void constant_bounds_check(int argc) {
    static int data[100];
    volatile int *volatile vdata = data;
    
    // Different constant bounds based on argc
    const int lo = (argc % 2 == 0) ? template_lo : CONST_LO;
    const int hi = (argc % 2 == 0) ? template_hi : CONST_HI;
    
    // This should trigger const_bounds_p with compile-time constants
    if (__builtin_constant_p(lo) && __builtin_constant_p(hi)) {
        // Force generation of tree nodes
        for (int i = lo; i <= hi; i++) {
            vdata[i] = i * 3;
        }
    }
    
    // Array slice copy with constant bounds
    int temp[10];
    if (hi - lo + 1 <= 10) {
        __builtin_memcpy(temp, &vdata[lo], (hi - lo + 1) * sizeof(int));
    }
}

// Main function with multiple paths
int main(int argc, char **argv) {
    int ret = 0;
    
    // Test MEM_P path with different conditions
    mem_target_operations(argc);
    
    // Test non-MEM_P path
    ret += non_mem_target_operations(argc);
    
    // Test constant bounds detection
    constant_bounds_check(argc);
    
    // Create observable side effects
    static volatile int sink = 0;
    sink = ret;
    
    // Use __builtin_constant_p to verify constant propagation
    if (__builtin_constant_p(CONST_HI - CONST_LO + 1)) {
        ret += 1;
    }
    
    return ret % 256;
}

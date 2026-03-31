#include <stdio.h>
#include <string.h>

// Helper to make indices compile-time constants
static inline int const_idx(int idx) {
    return idx;
}

// Template-like function using static inline to create constants
static inline int get_lo(int choice) {
    switch(choice) {
        case 0: return 5;
        case 1: return 10;
        case 2: return 0;
        case 3: return 100;
        default: return 0;
    }
}

static inline int get_hi(int choice) {
    switch(choice) {
        case 0: return 6;   // count = 2
        case 1: return 12;  // count = 3
        case 2: return 7;   // count = 8
        case 3: return 103; // count = 4
        default: return 0;
    }
}

int main(int argc, char *argv[]) {
    // Different array types to test TYPE_SIZE variations
    static char arr_char[256];
    static short arr_short[256];
    static int arr_int[256];
    static long long arr_ll[256];
    
    // Volatile pointers to inhibit early optimization
    volatile char *vchar = arr_char;
    volatile short *vshort = arr_short;
    volatile int *vint = arr_int;
    volatile long long *vll = arr_ll;
    
    // Initialize arrays
    for (int i = 0; i < 256; i++) {
        arr_char[i] = i % 128;
        arr_short[i] = i;
        arr_int[i] = i * 2;
        arr_ll[i] = i * 3LL;
    }
    
    int choice = argc > 1 ? argv[1][0] % 4 : 0;
    
    // Test 1: Memory target with count <= 2 (MEM_P path)
    if (choice == 0) {
        // char array: small element size
        const int lo = get_lo(0);  // 5
        const int hi = get_hi(0);  // 6, count = 2
        
        // Memory-to-memory copy of subrange
        char temp[2];
        for (int i = lo; i <= hi; i++) {
            temp[i - lo] = vchar[i];
        }
        // Copy back to different location
        for (int i = 0; i <= hi - lo; i++) {
            vchar[i + 20] = temp[i];
        }
        
        // Also test with builtin memcpy
        __builtin_memcpy(&arr_char[30], &arr_char[lo], (hi - lo + 1) * sizeof(char));
    }
    
    // Test 2: Memory target with count > 2 but small total size
    else if (choice == 1) {
        // char array: count = 3, total size = 3 bytes
        const int lo = get_lo(1);  // 10
        const int hi = get_hi(1);  // 12, count = 3
        
        char temp[3];
        for (int i = lo; i <= hi; i++) {
            temp[i - lo] = vchar[i];
        }
        for (int i = 0; i <= hi - lo; i++) {
            vchar[i + 40] = temp[i];
        }
    }
    
    // Test 3: Memory target with count > 2 and larger element size
    else if (choice == 2) {
        // long long array: count = 8, total size = 64 bytes
        const int lo = get_lo(2);  // 0
        const int hi = get_hi(2);  // 7, count = 8
        
        // Use volatile to prevent optimization
        volatile long long temp[8];
        for (int i = lo; i <= hi; i++) {
            temp[i - lo] = vll[i];
        }
        for (int i = 0; i <= hi - lo; i++) {
            vll[i + 50] = temp[i];
        }
    }
    
    // Test 4: Non-MEM_P target (register result)
    else if (choice == 3) {
        // int array: compute sum of subrange into register
        const int lo = get_lo(3);  // 100
        const int hi = get_hi(3);  // 103, count = 4
        
        int sum = 0;
        for (int i = lo; i <= hi; i++) {
            sum += vint[i];  // Target is register, not MEM_P
        }
        
        // Use the result to prevent dead code elimination
        vint[0] = sum;
        
        // Also test with direct computation
        int prod = vint[lo] * vint[lo + 1] * vint[lo + 2] * vint[hi];
        vint[1] = prod;
    }
    
    // Additional test: Mixed scenarios with conditional bounds
    if (argc > 2) {
        const int alt_lo = (argv[2][0] % 64);
        const int alt_hi = alt_lo + (argv[2][1] % 4);  // count 1-4
        
        // Test with short array
        short result = 0;
        for (int i = alt_lo; i <= alt_hi; i++) {
            result += vshort[i];
        }
        vshort[alt_lo] = result;
        
        // Verify constants are visible to compiler
        if (__builtin_constant_p(alt_hi - alt_lo + 1)) {
            vshort[alt_hi] = 1;
        }
    }
    
    // Create observable side effects
    int checksum = 0;
    for (int i = 0; i < 64; i++) {
        checksum += arr_char[i] + arr_short[i] + arr_int[i] + (int)arr_ll[i];
    }
    
    printf("Result: %d\n", checksum);
    return checksum % 256;
}

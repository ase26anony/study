#include <stdio.h>
#include <stdint.h>
#include <math.h>

#define SIZE 1024
#define DUMMY_INIT 0x12345678

// Global variables to prevent optimization
volatile int dummy = DUMMY_INIT;
double array1[SIZE];
double array2[SIZE];

// Initialize arrays with mixed values including NaNs
void __attribute__((noinline)) init_arrays() {
    for (int i = 0; i < SIZE; i++) {
        array1[i] = (double)(i + 1) * 1.5;
        // Every 4th element is NaN, others are normal numbers
        if (i % 4 == 0) {
            array2[i] = __builtin_nan("");
        } else {
            array2[i] = (double)(i % 10) * 0.7;
        }
    }
}

// Test UNORDERED condition code (unord)
void __attribute__((noinline)) test_unordered() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        // Generate UNORDERED condition code
        int cmp_result = __builtin_isunordered(a, b);
        
        // Force condition code output through inline assembly
        asm volatile("" 
                     : "+r" (dummy) 
                     : "g" (cmp_result)
                     : "cc");
    }
}

// Test ORDERED condition code (ord)
void __attribute__((noinline)) test_ordered() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        // Generate ORDERED condition code
        int cmp_result = !__builtin_isunordered(a, b);
        
        // Force condition code output
        asm volatile("" 
                     : "+r" (dummy) 
                     : "g" (cmp_result)
                     : "cc");
    }
}

// Test UNEQ condition code (ueq)
void __attribute__((noinline)) test_uneq() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        // Generate UNEQ condition code: unordered OR equal
        int cmp_result = __builtin_isunordered(a, b) || (a == b);
        
        // Force condition code output
        asm volatile("" 
                     : "+r" (dummy) 
                     : "g" (cmp_result)
                     : "cc");
    }
}

// Test UNGE condition code (nlt)
void __attribute__((noinline)) test_unge() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        // Generate UNGE condition code: unordered OR greater-or-equal
        int cmp_result = __builtin_isunordered(a, b) || (a >= b);
        
        // Force condition code output
        asm volatile("" 
                     : "+r" (dummy) 
                     : "g" (cmp_result)
                     : "cc");
    }
}

// Test UNGT condition code (nle)
void __attribute__((noinline)) test_ungt() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        // Generate UNGT condition code: unordered OR greater-than
        int cmp_result = __builtin_isunordered(a, b) || (a > b);
        
        // Force condition code output
        asm volatile("" 
                     : "+r" (dummy) 
                     : "g" (cmp_result)
                     : "cc");
    }
}

// Test UNLE condition code (ule)
void __attribute__((noinline)) test_unle() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        // Generate UNLE condition code: unordered OR less-or-equal
        int cmp_result = __builtin_isunordered(a, b) || (a <= b);
        
        // Force condition code output
        asm volatile("" 
                     : "+r" (dummy) 
                     : "g" (cmp_result)
                     : "cc");
    }
}

// Test UNLT condition code (ult)
void __attribute__((noinline)) test_unlt() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        // Generate UNLT condition code: unordered OR less-than
        int cmp_result = __builtin_isunordered(a, b) || (a < b);
        
        // Force condition code output
        asm volatile("" 
                     : "+r" (dummy) 
                     : "g" (cmp_result)
                     : "cc");
    }
}

// Test LTGT condition code (une)
void __attribute__((noinline)) test_ltgt() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        // Generate LTGT condition code using builtin
        int cmp_result = __builtin_islessgreater(a, b);
        
        // Alternative: (a < b) || (a > b) but not equal
        // This should also generate LTGT
        
        // Force condition code output
        asm volatile("" 
                     : "+r" (dummy) 
                     : "g" (cmp_result)
                     : "cc");
    }
}

// Additional test with direct comparisons to ensure coverage
void __attribute__((noinline)) test_mixed_comparisons() {
    for (int i = 0; i < SIZE; i++) {
        float fa = (float)array1[i];
        float fb = (float)array2[i];
        
        // Mix of different comparison types
        int cmp1 = (fa != fb);  // May generate UNEQ or LTGT
        int cmp2 = (fa < fb);   // May generate UNLT
        int cmp3 = (fa > fb);   // May generate UNGT
        int cmp4 = (fa <= fb);  // May generate UNLE
        int cmp5 = (fa >= fb);  // May generate UNGE
        
        // Force multiple condition codes
        asm volatile("" : "+r" (dummy) : "g" (cmp1) : "cc");
        asm volatile("" : "+r" (dummy) : "g" (cmp2) : "cc");
        asm volatile("" : "+r" (dummy) : "g" (cmp3) : "cc");
        asm volatile("" : "+r" (dummy) : "g" (cmp4) : "cc");
        asm volatile("" : "+r" (dummy) : "g" (cmp5) : "cc");
    }
}

int main() {
    // Initialize arrays with mixed values including NaNs
    init_arrays();
    
    // Call all test functions to generate various condition codes
    test_unordered();      // Should generate "unord"
    test_ordered();        // Should generate "ord"
    test_uneq();           // Should generate "ueq"
    test_unge();           // Should generate "nlt"
    test_ungt();           // Should generate "nle"
    test_unle();           // Should generate "ule"
    test_unlt();           // Should generate "ult"
    test_ltgt();           // Should generate "une"
    
    // Additional mixed comparisons
    test_mixed_comparisons();
    
    // Compute checksum to prevent dead code elimination
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += (int)(array1[i] + array2[i]);
    }
    
    // Use dummy to prevent optimization
    checksum += dummy;
    
    printf("Checksum: %d\n", checksum);
    return 0;
}

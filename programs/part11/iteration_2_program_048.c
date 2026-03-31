#include <stdio.h>
#include <stdint.h>
#include <math.h>

#define ARRAY_SIZE 1024
#define NAN_INTERVAL 4

/* Global dummy variable to prevent optimization */
volatile uint64_t dummy = 0;

/* Arrays with mixed normal values and NaNs */
double array1[ARRAY_SIZE];
double array2[ARRAY_SIZE];

/* Initialize arrays with pattern including NaNs */
__attribute__((constructor)) void init_arrays() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = (double)(i + 1) * 1.5;
        if (i % NAN_INTERVAL == 0) {
            array2[i] = __builtin_nan("");
        } else {
            array2[i] = (double)(i + 1) * 2.5;
        }
    }
}

/* Test functions for each condition code type */

__attribute__((noinline)) void test_unordered() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        int cmp = __builtin_isunordered(a, b);
        asm volatile("" : "+r"(dummy) : "g"(cmp));
    }
}

__attribute__((noinline)) void test_ordered() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        int cmp = !__builtin_isunordered(a, b);
        asm volatile("" : "+r"(dummy) : "g"(cmp));
    }
}

__attribute__((noinline)) void test_uneq() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        int cmp = (a == b) || __builtin_isunordered(a, b);
        asm volatile("" : "+r"(dummy) : "g"(cmp));
    }
}

__attribute__((noinline)) void test_unge() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        int cmp = !(a < b);
        asm volatile("" : "+r"(dummy) : "g"(cmp));
    }
}

__attribute__((noinline)) void test_ungt() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        int cmp = !(a <= b);
        asm volatile("" : "+r"(dummy) : "g"(cmp));
    }
}

__attribute__((noinline)) void test_unle() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        int cmp = (a <= b) || __builtin_isunordered(a, b);
        asm volatile("" : "+r"(dummy) : "g"(cmp));
    }
}

__attribute__((noinline)) void test_unlt() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        int cmp = (a < b) || __builtin_isunordered(a, b);
        asm volatile("" : "+r"(dummy) : "g"(cmp));
    }
}

__attribute__((noinline)) void test_ltgt() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        int cmp = __builtin_islessgreater(a, b);
        asm volatile("" : "+r"(dummy) : "g"(cmp));
    }
}

/* Additional test functions using direct comparisons */

__attribute__((noinline)) void test_direct_comparisons() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Generate various condition codes through direct comparisons */
        int cmp1 = (a != b);
        int cmp2 = (a >= b);
        int cmp3 = (a > b);
        int cmp4 = (a <= b);
        int cmp5 = (a < b);
        
        asm volatile("" : "+r"(dummy) : "g"(cmp1));
        asm volatile("" : "+r"(dummy) : "g"(cmp2));
        asm volatile("" : "+r"(dummy) : "g"(cmp3));
        asm volatile("" : "+r"(dummy) : "g"(cmp4));
        asm volatile("" : "+r"(dummy) : "g"(cmp5));
    }
}

int main() {
    /* Call all test functions to generate condition codes */
    test_unordered();
    test_ordered();
    test_uneq();
    test_unge();
    test_ungt();
    test_unle();
    test_unlt();
    test_ltgt();
    test_direct_comparisons();
    
    /* Use dummy variable to prevent dead code elimination */
    printf("Checksum: %lu\n", (unsigned long)dummy);
    
    return 0;
}

#include <stdio.h>
#include <stdint.h>
#include <math.h>

#define ARRAY_SIZE 1024
#define NAN_INTERVAL 4

/* Global variables to prevent optimization */
volatile int dummy = 0;
double array1[ARRAY_SIZE];
double array2[ARRAY_SIZE];

/* Initialize arrays with mixed NaN and normal values */
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
        int cmp_result = __builtin_isunordered(a, b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

__attribute__((noinline)) void test_ordered() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        int cmp_result = !__builtin_isunordered(a, b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

__attribute__((noinline)) void test_uneq() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        int cmp_result = (a == b) || __builtin_isunordered(a, b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

__attribute__((noinline)) void test_unge() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        int cmp_result = !(a < b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

__attribute__((noinline)) void test_ungt() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        int cmp_result = !(a <= b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

__attribute__((noinline)) void test_unle() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        int cmp_result = (a <= b) || __builtin_isunordered(a, b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

__attribute__((noinline)) void test_unlt() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        int cmp_result = (a < b) || __builtin_isunordered(a, b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

__attribute__((noinline)) void test_ltgt() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        int cmp_result = __builtin_islessgreater(a, b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Additional test with direct comparisons to ensure coverage */
__attribute__((noinline)) void test_mixed_comparisons() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Direct unordered comparison */
        int cmp1 = (a != a) || (b != b);
        asm volatile("" : "+r"(dummy) : "g"(cmp1));
        
        /* Direct ordered comparison */
        int cmp2 = (a == a) && (b == b);
        asm volatile("" : "+r"(dummy) : "g"(cmp2));
        
        /* Various comparison operators */
        int cmp3 = (a > b);
        asm volatile("" : "+r"(dummy) : "g"(cmp3));
        
        int cmp4 = (a < b);
        asm volatile("" : "+r"(dummy) : "g"(cmp4));
        
        int cmp5 = (a >= b);
        asm volatile("" : "+r"(dummy) : "g"(cmp5));
        
        int cmp6 = (a <= b);
        asm volatile("" : "+r"(dummy) : "g"(cmp6));
        
        int cmp7 = (a == b);
        asm volatile("" : "+r"(dummy) : "g"(cmp7));
        
        int cmp8 = (a != b);
        asm volatile("" : "+r"(dummy) : "g"(cmp8));
    }
}

int main() {
    /* Call all test functions */
    test_unordered();
    test_ordered();
    test_uneq();
    test_unge();
    test_ungt();
    test_unle();
    test_unlt();
    test_ltgt();
    test_mixed_comparisons();
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += (int)(array1[i] + array2[i]);
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Dummy: %d\n", dummy);
    
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Compiler barrier to prevent reordering
#define COMPILER_BARRIER() asm volatile("" : : : "memory")

// Test functions marked noinline to preserve boundaries
__attribute__((noinline))
int test_pointer_increment_sum(int *array, int n) {
    int sum = 0;
    int *ptr = array;
    int *end = array + n;
    
    // Simple pointer increment pattern
    while (ptr < end) {
        sum += *ptr;
        ptr++;  // This should trigger auto-inc-dec pattern
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
void test_pointer_copy(int *dst, int *src, int n) {
    int *d = dst;
    int *s = src;
    int *end = src + n;
    
    // Copy with pointer increment - classic pattern for auto-inc-dec
    while (s < end) {
        *d = *s;
        d++;
        s++;  // Two pointers incrementing
    }
    
    COMPILER_BARRIER();
}

__attribute__((noinline))
long long test_mixed_types(char *carr, short *sarr, int *iarr, long long *llarr, int n) {
    long long total = 0;
    char *cp = carr;
    short *sp = sarr;
    int *ip = iarr;
    long long *llp = llarr;
    
    // Mixed type accesses with pointer arithmetic
    for (int i = 0; i < n; i++) {
        total += *cp;
        cp++;  // char pointer increment
        
        total += *sp;
        sp++;  // short pointer increment
        
        total += *ip;
        ip++;  // int pointer increment
        
        total += *llp;
        llp++;  // long long pointer increment
    }
    
    COMPILER_BARRIER();
    return total;
}

__attribute__((noinline))
int test_complex_addressing(int *array, int n) {
    int sum = 0;
    volatile int *volatile_ptr = array;  // Volatile to prevent simplification
    
    // Complex addressing with offset
    for (int i = 0; i < n; i++) {
        // This creates XEXP(x, 0) pattern with offset
        sum += *(volatile_ptr + i);
        // The increment happens in the loop counter
    }
    
    // Now with explicit pointer arithmetic
    int *ptr = array;
    for (int i = 0; i < n; i++) {
        sum += *ptr;
        ptr += 1;  // Non-unity stride
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_structure_access(void *data, int n) {
    struct S {
        int a;
        int b;
        char c;
        short d;
    };
    
    struct S *sp = (struct S *)data;
    int total = 0;
    
    // Structure access with pointer walking
    for (int i = 0; i < n; i++) {
        total += sp->a;
        total += sp->b;
        total += sp->c;
        total += sp->d;
        sp++;  // Pointer increment after multiple member accesses
    }
    
    COMPILER_BARRIER();
    return total;
}

__attribute__((noinline))
int test_dual_pointer_arithmetic(int *arr1, int *arr2, int n) {
    int sum = 0;
    int *p1 = arr1;
    int *p2 = arr2;
    
    // Two pointers with arithmetic that could be seen as base-plus-index
    for (int i = 0; i < n; i++) {
        // Complex expression that might simplify to base register
        sum += *(p1 + (p2 - p2));  // Should simplify to *p1
        p1++;
        p2++;
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_array_index_and_pointer_mix(int *array, int n) {
    int sum = 0;
    int *ptr = array;
    
    // Mix of array indexing and pointer arithmetic
    for (int i = 0; i < n; i++) {
        sum += array[i];  // Array indexing
        sum += *ptr;      // Pointer dereference
        ptr++;            // Pointer increment
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
void test_fill_with_value(int *array, int n, int value) {
    int *ptr = array;
    int *end = array + n;
    
    // Fill array with pointer write and increment
    while (ptr < end) {
        *ptr = value;
        ptr++;  // Write with increment
    }
    
    COMPILER_BARRIER();
}

int main(int argc, char **argv) {
    // Use argc to make loop bounds non-constant
    int size = (argc > 1) ? atoi(argv[1]) : 100;
    if (size < 10) size = 100;
    
    // Allocate arrays of different types
    int *int_array1 = malloc(size * sizeof(int));
    int *int_array2 = malloc(size * sizeof(int));
    char *char_array = malloc(size * sizeof(char));
    short *short_array = malloc(size * sizeof(short));
    long long *ll_array = malloc(size * sizeof(long long));
    struct S {
        int a;
        int b;
        char c;
        short d;
    };
    struct S *struct_array = malloc(size * sizeof(struct S));
    
    // Initialize arrays
    for (int i = 0; i < size; i++) {
        int_array1[i] = i;
        int_array2[i] = size - i;
        char_array[i] = (char)(i % 256);
        short_array[i] = (short)(i * 2);
        ll_array[i] = (long long)i * 1000;
        struct_array[i].a = i;
        struct_array[i].b = i * 2;
        struct_array[i].c = (char)(i % 128);
        struct_array[i].d = (short)(i * 3);
    }
    
    int checksum = 0;
    
    // Run all test patterns
    checksum += test_pointer_increment_sum(int_array1, size);
    
    test_pointer_copy(int_array2, int_array1, size);
    checksum += int_array2[size/2];  // Sample value
    
    checksum += test_mixed_types(char_array, short_array, int_array1, ll_array, size);
    
    checksum += test_complex_addressing(int_array1, size);
    
    checksum += test_structure_access(struct_array, size);
    
    checksum += test_dual_pointer_arithmetic(int_array1, int_array2, size);
    
    checksum += test_array_index_and_pointer_mix(int_array1, size);
    
    test_fill_with_value(int_array2, size, 0xABCD);
    checksum += int_array2[size/3];  // Sample value
    
    // Final compiler barrier
    COMPILER_BARRIER();
    
    printf("Checksum: %d\n", checksum);
    
    // Cleanup
    free(int_array1);
    free(int_array2);
    free(char_array);
    free(short_array);
    free(ll_array);
    free(struct_array);
    
    return 0;
}

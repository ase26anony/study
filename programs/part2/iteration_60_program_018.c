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
        ptr++;  // This should trigger auto-inc pattern
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
void test_pointer_copy(int *dst, int *src, int n) {
    int *d = dst;
    int *s = src;
    int *end = src + n;
    
    // Copy with post-increment on both sides
    while (s < end) {
        *d = *s;
        d++;
        s++;  // Two auto-inc opportunities
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
int test_complex_addressing(int *array, int n, int offset) {
    int sum = 0;
    volatile int *volatile_ptr = array;  // Volatile to prevent simplification
    int *ptr = (int *)volatile_ptr;
    
    // Complex addressing: ptr[offset] with pointer increment
    for (int i = 0; i < n; i++) {
        // This creates XEXP(x, 0) pattern
        sum += ptr[offset];
        ptr++;  // Increment after access
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
        sp++;  // Structure pointer increment - multiple memory accesses before increment
    }
    
    COMPILER_BARRIER();
    return total;
}

__attribute__((noinline))
int test_dual_pointer_arithmetic(int *arr1, int *arr2, int n) {
    int sum = 0;
    int *p1 = arr1;
    int *p2 = arr2;
    
    // Dual pointer arithmetic that might create base-plus-index patterns
    for (int i = 0; i < n; i++) {
        // Complex address expression: *(p1 + (p2 - p2)) simplifies but requires analysis
        sum += *(p1 + (p2 - p2));
        p1++;
        
        // Another complex expression
        sum += p2[0];
        p2++;
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_pointer_with_constant_offset(int *array, int n) {
    int sum = 0;
    int *ptr = array;
    
    // Access with constant offset then increment
    for (int i = 0; i < n; i++) {
        // *(ptr + 4) creates non-trivial address expression
        sum += *(ptr + 4);
        ptr++;  // This should be recognized as auto-inc candidate
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
void test_write_pattern(int *array, int n, int value) {
    int *ptr = array;
    int *end = array + n;
    
    // Write pattern with post-increment
    while (ptr < end) {
        *ptr = value;
        ptr++;  // Auto-inc for write operations
    }
    
    COMPILER_BARRIER();
}

int main(int argc, char **argv) {
    // Use argc to make loop bounds non-constant at compile time
    int size = (argc > 1) ? atoi(argv[1]) : 100;
    if (size < 10) size = 100;
    
    // Allocate arrays of different types
    char *char_array = (char *)malloc(size * sizeof(char));
    short *short_array = (short *)malloc(size * sizeof(short));
    int *int_array1 = (int *)malloc(size * sizeof(int));
    int *int_array2 = (int *)malloc(size * sizeof(int));
    long long *ll_array = (long long *)malloc(size * sizeof(long long));
    struct S *struct_array = (struct S *)malloc(size * sizeof(struct S));
    
    // Initialize arrays
    for (int i = 0; i < size; i++) {
        char_array[i] = (char)(i % 256);
        short_array[i] = (short)(i * 2);
        int_array1[i] = i * 3;
        int_array2[i] = i * 4;
        ll_array[i] = (long long)i * 1000;
        struct_array[i].a = i;
        struct_array[i].b = i * 2;
        struct_array[i].c = (char)(i % 128);
        struct_array[i].d = (short)(i % 32768);
    }
    
    int checksum = 0;
    
    // Run all test patterns
    checksum += test_pointer_increment_sum(int_array1, size);
    
    test_pointer_copy(int_array2, int_array1, size);
    checksum += int_array2[size / 2];  // Sample check
    
    checksum += test_mixed_types(char_array, short_array, int_array1, ll_array, size / 4);
    
    checksum += test_complex_addressing(int_array1, size, 2);
    
    checksum += test_structure_access(struct_array, size / 2);
    
    checksum += test_dual_pointer_arithmetic(int_array1, int_array2, size / 2);
    
    checksum += test_pointer_with_constant_offset(int_array1, size / 2);
    
    test_write_pattern(int_array2, size, 0xABCD);
    checksum += int_array2[size / 3];  // Sample check
    
    // Final compiler barrier
    COMPILER_BARRIER();
    
    printf("Final checksum: %d\n", checksum);
    
    // Cleanup
    free(char_array);
    free(short_array);
    free(int_array1);
    free(int_array2);
    free(ll_array);
    free(struct_array);
    
    return 0;
}

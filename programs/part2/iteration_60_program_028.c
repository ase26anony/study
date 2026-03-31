#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Compiler barriers to prevent reordering
#define COMPILER_BARRIER() asm volatile("" : : : "memory")

// Test functions marked noinline to preserve boundaries
__attribute__((noinline))
int test_char_pointer_sum(volatile char *data, int n) {
    char *ptr = (char *)data;
    int sum = 0;
    
    // Simple pointer increment pattern
    for (int i = 0; i < n; i++) {
        sum += *ptr;
        ptr++;  // Adjacent increment - should trigger auto-inc pattern
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_short_pointer_copy(volatile short *src, volatile short *dst, int n) {
    short *s = (short *)src;
    short *d = (short *)dst;
    
    // Classic copy pattern with pointer increment
    for (int i = 0; i < n; i++) {
        *d = *s;
        s++;
        d++;  // Two pointers incrementing - complex addressing
    }
    
    COMPILER_BARRIER();
    return (int)*d;
}

__attribute__((noinline))
long test_int_pointer_stride(volatile int *data, int n, int stride) {
    int *ptr = (int *)data;
    long sum = 0;
    
    // Pointer with stride increment
    for (int i = 0; i < n; i++) {
        sum += *ptr;
        ptr += stride;  // Non-unit stride increment
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
long long test_mixed_index_pointer(volatile long long *data, int n) {
    long long *ptr = (long long *)data;
    long long sum = 0;
    
    // Mixed indexing and pointer arithmetic
    for (int i = 0; i < n; i++) {
        // Access with offset - creates complex address expression
        sum += ptr[0];
        // Also try *(ptr + 1) to create different pattern
        if (i < n - 1) {
            sum += *(ptr + 1);
        }
        ptr++;  // Increment after mixed accesses
    }
    
    COMPILER_BARRIER();
    return sum;
}

// Structure for pointer walking
struct MixedData {
    int a;
    char b;
    short c;
    long d;
};

__attribute__((noinline))
long test_struct_pointer_walk(volatile struct MixedData *data, int n) {
    struct MixedData *ptr = (struct MixedData *)data;
    long sum = 0;
    
    // Walk through array of structures
    for (int i = 0; i < n; i++) {
        // Access multiple members
        sum += ptr->a;
        sum += ptr->b;
        sum += ptr->c;
        sum += ptr->d;
        ptr++;  // Increment after last member access
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_complex_address_expr(volatile int *base1, volatile int *base2, int n) {
    int *p1 = (int *)base1;
    int *p2 = (int *)base2;
    int sum = 0;
    
    // Complex address expression that might simplify
    for (int i = 0; i < n; i++) {
        // This creates: *(p1 + (p2 - p2)) which should simplify to *p1
        // but requires analysis to reach the auto-inc pattern
        sum += p1[p2 - p2];
        p1++;  // Increment after complex access
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_pointer_arithmetic_chain(volatile int *data, int n) {
    int *ptr = (int *)data;
    int sum = 0;
    
    // Chain of pointer arithmetic operations
    for (int i = 0; i < n; i++) {
        // Multiple ways to access through pointer
        sum += *ptr;
        sum += *(ptr + 1);
        sum += ptr[2];
        
        // Increment with non-trivial expression
        ptr = ptr + 1;  // Different syntax for increment
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
void test_pointer_write_increment(volatile int *data, int value, int n) {
    int *ptr = (int *)data;
    
    // Write pattern with increment
    for (int i = 0; i < n; i++) {
        *ptr = value + i;
        ptr++;  // Write then increment
    }
    
    COMPILER_BARRIER();
}

int main(int argc, char **argv) {
    // Use argc to make loop bounds non-constant
    int n = (argc > 1) ? atoi(argv[1]) : 100;
    if (n < 10) n = 10;
    if (n > 1000) n = 1000;
    
    // Allocate arrays of different types
    char *char_array = (char *)malloc(n * sizeof(char));
    short *short_array1 = (short *)malloc(n * sizeof(short));
    short *short_array2 = (short *)malloc(n * sizeof(short));
    int *int_array1 = (int *)malloc(n * sizeof(int));
    int *int_array2 = (int *)malloc(n * sizeof(int));
    long long *ll_array = (long long *)malloc(n * sizeof(long long));
    struct MixedData *struct_array = (struct MixedData *)malloc(n * sizeof(struct MixedData));
    
    // Initialize with volatile to prevent compile-time simplification
    volatile int init_val = 7;
    
    for (int i = 0; i < n; i++) {
        char_array[i] = (char)(init_val + i);
        short_array1[i] = (short)(init_val + i);
        short_array2[i] = 0;
        int_array1[i] = init_val + i;
        int_array2[i] = 0;
        ll_array[i] = init_val + i;
        struct_array[i].a = init_val + i;
        struct_array[i].b = (char)(init_val + i);
        struct_array[i].c = (short)(init_val + i);
        struct_array[i].d = init_val + i;
    }
    
    long total_sum = 0;
    
    // Run all test patterns
    total_sum += test_char_pointer_sum((volatile char *)char_array, n);
    total_sum += test_short_pointer_copy((volatile short *)short_array1, 
                                        (volatile short *)short_array2, n);
    total_sum += test_int_pointer_stride((volatile int *)int_array1, n, 1);
    total_sum += test_mixed_index_pointer((volatile long long *)ll_array, n);
    total_sum += test_struct_pointer_walk((volatile struct MixedData *)struct_array, n);
    total_sum += test_complex_address_expr((volatile int *)int_array1, 
                                          (volatile int *)int_array2, n);
    total_sum += test_pointer_arithmetic_chain((volatile int *)int_array1, n);
    
    // Test write pattern
    test_pointer_write_increment((volatile int *)int_array2, init_val, n);
    
    // Verify writes
    for (int i = 0; i < n; i++) {
        total_sum += int_array2[i];
    }
    
    printf("Total checksum: %ld\n", total_sum);
    
    // Cleanup
    free(char_array);
    free(short_array1);
    free(short_array2);
    free(int_array1);
    free(int_array2);
    free(ll_array);
    free(struct_array);
    
    return 0;
}

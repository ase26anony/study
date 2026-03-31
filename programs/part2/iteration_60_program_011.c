#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Compiler barriers to prevent unwanted optimizations
#define COMPILER_BARRIER() asm volatile("" : : : "memory")

// Test functions marked noinline to preserve boundaries
__attribute__((noinline))
int test_char_sum(volatile char *data, int n) {
    char *ptr = (char *)data;
    int sum = 0;
    
    // Simple pointer increment pattern
    for (int i = 0; i < n; i++) {
        sum += *ptr;
        ptr++;  // Should trigger auto-inc pattern
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_short_sum(volatile short *data, int n) {
    short *ptr = (short *)data;
    int sum = 0;
    
    // Pointer with stride (could be 1 or more)
    for (int i = 0; i < n; i++) {
        sum += *ptr;
        ptr += 1;  // Different increment form
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_int_sum(volatile int *data, int n) {
    int *ptr = (int *)data;
    int sum = 0;
    
    // Mixed indexing and pointer arithmetic
    for (int i = 0; i < n; i++) {
        // Complex addressing: *(ptr + 0) which becomes XEXP(x, 0)
        sum += *(ptr + 0);
        ptr = ptr + 1;  // Another form of increment
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
long long test_longlong_sum(volatile long long *data, int n) {
    long long *ptr = (long long *)data;
    long long sum = 0;
    
    // Pointer increment with offset access
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  // Array indexing form
        ptr++;  // Post-increment
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
void test_copy(volatile int *src, volatile int *dst, int n) {
    int *s = (int *)src;
    int *d = (int *)dst;
    
    // Classic copy pattern with two moving pointers
    for (int i = 0; i < n; i++) {
        *d = *s;
        s++;
        d++;
    }
    
    COMPILER_BARRIER();
}

__attribute__((noinline))
void test_fill(volatile int *data, int value, int n) {
    int *ptr = (int *)data;
    
    // Fill pattern with write and increment
    for (int i = 0; i < n; i++) {
        *ptr = value;
        ptr++;
    }
    
    COMPILER_BARRIER();
}

// Structure to force complex addressing
struct MixedData {
    int a;
    short b;
    char c;
    long long d;
};

__attribute__((noinline))
long long test_struct_sum(volatile struct MixedData *data, int n) {
    struct MixedData *ptr = (struct MixedData *)data;
    long long sum = 0;
    
    // Access multiple structure members with pointer walking
    for (int i = 0; i < n; i++) {
        sum += ptr->a + ptr->b + ptr->c + ptr->d;
        ptr++;  // Increment after multiple accesses
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_complex_addressing(volatile int *base1, volatile int *base2, int n) {
    int *p1 = (int *)base1;
    int *p2 = (int *)base2;
    int sum = 0;
    
    // Complex addressing that might create base-plus-index
    for (int i = 0; i < n; i++) {
        // Expression that could be seen as (p1 + (p2 - p2))
        // Might simplify but requires analysis
        sum += *(p1 + (p2 - p2));
        p1++;
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_mixed_patterns(volatile int *data, int n) {
    int *ptr = (int *)data;
    int sum = 0;
    
    // Mix different access patterns in same loop
    for (int i = 0; i < n; i++) {
        // Multiple accesses with same pointer
        int val1 = *ptr;
        ptr++;
        int val2 = *ptr;  // ptr already incremented
        sum += val1 + val2;
        // Don't increment here - already done in middle
    }
    
    COMPILER_BARRIER();
    return sum;
}

int main(int argc, char **argv) {
    // Use argc to make loop bounds non-constant
    int size = (argc > 1) ? atoi(argv[1]) : 100;
    if (size < 10) size = 100;
    
    // Allocate arrays of different types
    char *char_array = malloc(size * sizeof(char));
    short *short_array = malloc(size * sizeof(short));
    int *int_array = malloc(size * sizeof(int));
    long long *ll_array = malloc(size * sizeof(long long));
    int *src_array = malloc(size * sizeof(int));
    int *dst_array = malloc(size * sizeof(int));
    struct MixedData *struct_array = malloc(size * sizeof(struct MixedData));
    
    // Initialize with pattern
    for (int i = 0; i < size; i++) {
        char_array[i] = (char)(i % 256);
        short_array[i] = (short)(i * 2);
        int_array[i] = i * 3;
        ll_array[i] = i * 1000LL;
        src_array[i] = i * 5;
        struct_array[i].a = i;
        struct_array[i].b = (short)(i * 2);
        struct_array[i].c = (char)(i % 128);
        struct_array[i].d = i * 100LL;
    }
    
    int total = 0;
    
    // Run all test patterns
    total += test_char_sum((volatile char *)char_array, size);
    total += test_short_sum((volatile short *)short_array, size);
    total += test_int_sum((volatile int *)int_array, size);
    total += test_longlong_sum((volatile long long *)ll_array, size);
    
    test_copy((volatile int *)src_array, (volatile int *)dst_array, size);
    
    // Verify copy
    for (int i = 0; i < size; i++) {
        total += dst_array[i];
    }
    
    test_fill((volatile int *)dst_array, 42, size);
    
    // Verify fill
    for (int i = 0; i < size; i++) {
        total += dst_array[i];
    }
    
    total += test_struct_sum((volatile struct MixedData *)struct_array, size);
    total += test_complex_addressing((volatile int *)int_array, 
                                     (volatile int *)src_array, size);
    total += test_mixed_patterns((volatile int *)int_array, size);
    
    printf("Total checksum: %d\n", total);
    
    // Free memory
    free(char_array);
    free(short_array);
    free(int_array);
    free(ll_array);
    free(src_array);
    free(dst_array);
    free(struct_array);
    
    return 0;
}

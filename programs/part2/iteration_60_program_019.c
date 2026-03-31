#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Compiler barriers to prevent unwanted optimizations
#define COMPILER_BARRIER() asm volatile("" : : : "memory")

// Non-inline functions to preserve boundaries
__attribute__((noinline)) 
int test_char_pointer_increment(volatile char *data, int n) {
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
int test_short_pointer_with_offset(volatile short *data, int n) {
    short *ptr = (short *)data;
    int sum = 0;
    
    // Pointer with offset access
    for (int i = 0; i < n; i++) {
        // Complex addressing: *(ptr + 0) but ptr is modified
        sum += *(ptr + 0);
        ptr += 1;  // Increment by sizeof(short)
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_int_mixed_index_pointer(volatile int *data, int n) {
    int *ptr = (int *)data;
    int sum = 0;
    
    // Mixed indexing and pointer arithmetic
    for (int i = 0; i < n; i++) {
        // Array indexing form
        sum += ptr[0];
        // Pointer increment - adjacent to access
        ptr = ptr + 1;
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
long long test_longlong_copy_pattern(volatile long long *src, 
                                     volatile long long *dst, 
                                     int n) {
    long long *s = (long long *)src;
    long long *d = (long long *)dst;
    long long checksum = 0;
    
    // Classic copy pattern with two moving pointers
    for (int i = 0; i < n; i++) {
        *d = *s;  // Memory access
        checksum += *d;
        d++;      // Adjacent increments
        s++;
    }
    
    COMPILER_BARRIER();
    return checksum;
}

// Structure for pointer walking
struct MixedData {
    int a;
    short b;
    char c;
    long long d;
};

__attribute__((noinline))
long long test_struct_pointer_walk(volatile struct MixedData *data, int n) {
    struct MixedData *ptr = (struct MixedData *)data;
    long long sum = 0;
    
    // Walk through array of structures
    for (int i = 0; i < n; i++) {
        // Access multiple members
        sum += ptr->a;
        sum += ptr->b;
        sum += ptr->c;
        sum += ptr->d;
        // Pointer increment after last access
        ptr++;
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_complex_address_expression(volatile int *data1, 
                                   volatile int *data2, 
                                   int n) {
    int *p1 = (int *)data1;
    int *p2 = (int *)data2;
    int sum = 0;
    
    // Complex addressing that might simplify to base + 0
    for (int i = 0; i < n; i++) {
        // Expression: *(p1 + (p2 - p2)) which simplifies to *p1
        // But requires analysis to reach XEXP(x, 0)
        sum += *(p1 + (p2 - p2));
        p1++;
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_pointer_arithmetic_chain(volatile int *data, int n) {
    int *ptr = (int *)data;
    int sum = 0;
    
    // Chain of pointer arithmetic
    for (int i = 0; i < n; i++) {
        int *temp = ptr;
        sum += *temp;  // Access through temporary
        ptr = temp + 1;  // Increment through chain
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
void test_fill_pattern(volatile int *data, int value, int n) {
    int *ptr = (int *)data;
    
    // Fill with write and increment
    for (int i = 0; i < n; i++) {
        *ptr = value;
        ptr++;
    }
    
    COMPILER_BARRIER();
}

int main(int argc, char **argv) {
    // Use argc to make loop bounds non-constant
    int base_size = (argc > 1) ? atoi(argv[1]) : 100;
    if (base_size < 10) base_size = 100;
    
    // Allocate arrays of different types
    char *char_array = malloc(base_size * sizeof(char));
    short *short_array = malloc(base_size * sizeof(short));
    int *int_array1 = malloc(base_size * sizeof(int));
    int *int_array2 = malloc(base_size * sizeof(int));
    long long *ll_array1 = malloc(base_size * sizeof(long long));
    long long *ll_array2 = malloc(base_size * sizeof(long long));
    struct MixedData *struct_array = malloc(base_size * sizeof(struct MixedData));
    
    // Initialize with pattern
    for (int i = 0; i < base_size; i++) {
        char_array[i] = (char)(i % 256);
        short_array[i] = (short)(i * 2);
        int_array1[i] = i * 3;
        int_array2[i] = i * 4;
        ll_array1[i] = i * 5LL;
        ll_array2[i] = i * 6LL;
        struct_array[i].a = i;
        struct_array[i].b = (short)(i * 2);
        struct_array[i].c = (char)(i % 128);
        struct_array[i].d = i * 7LL;
    }
    
    long long total_checksum = 0;
    
    // Run all test patterns
    total_checksum += test_char_pointer_increment((volatile char *)char_array, base_size);
    total_checksum += test_short_pointer_with_offset((volatile short *)short_array, base_size);
    total_checksum += test_int_mixed_index_pointer((volatile int *)int_array1, base_size);
    total_checksum += test_longlong_copy_pattern((volatile long long *)ll_array1, 
                                                (volatile long long *)ll_array2, 
                                                base_size);
    total_checksum += test_struct_pointer_walk((volatile struct MixedData *)struct_array, base_size);
    total_checksum += test_complex_address_expression((volatile int *)int_array1, 
                                                     (volatile int *)int_array2, 
                                                     base_size);
    total_checksum += test_pointer_arithmetic_chain((volatile int *)int_array1, base_size);
    
    // Fill pattern test
    test_fill_pattern((volatile int *)int_array2, 0xABCD, base_size);
    
    // Verify fill
    for (int i = 0; i < base_size; i++) {
        total_checksum += int_array2[i];
    }
    
    printf("Total checksum: %lld\n", total_checksum);
    
    // Cleanup
    free(char_array);
    free(short_array);
    free(int_array1);
    free(int_array2);
    free(ll_array1);
    free(ll_array2);
    free(struct_array);
    
    return 0;
}

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
        ptr++;  // Should trigger auto-inc pattern
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_short_pointer_offset(volatile short *data, int n) {
    short *ptr = (short *)data;
    int sum = 0;
    
    // Pointer with offset access
    for (int i = 0; i < n; i++) {
        // Complex addressing: *(ptr + 0) which becomes XEXP(x, 0)
        sum += *(ptr + 0);
        ptr += 1;  // Increment after access
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_int_pointer_copy(volatile int *src, volatile int *dst, int n) {
    int *s = (int *)src;
    int *d = (int *)dst;
    int sum = 0;
    
    // Classic copy pattern with two moving pointers
    for (int i = 0; i < n; i++) {
        *d = *s;
        sum += *d;
        s++;
        d++;
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
long long test_longlong_mixed_index(volatile long long *data, int n) {
    long long *ptr = (long long *)data;
    long long sum = 0;
    
    // Mixed indexing and pointer arithmetic
    for (int i = 0; i < n; i++) {
        // Array indexing that should convert to pointer form
        sum += ptr[0];
        // Pointer increment in same iteration
        ptr = ptr + 1;
    }
    
    COMPILER_BARRIER();
    return sum;
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
        // Increment pointer after last access
        ptr++;
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_double_pointer_arithmetic(volatile int *data1, volatile int *data2, int n) {
    int *p1 = (int *)data1;
    int *p2 = (int *)data2;
    int sum = 0;
    
    // Complex pointer arithmetic that might create interesting addressing modes
    for (int i = 0; i < n; i++) {
        // Expression that could be seen as base-plus-index: p1 + (p2 - p2)
        sum += *(p1 + (p2 - p2));
        p1++;
        // Keep p2 moving too to prevent complete optimization
        p2 = p2 + 1;
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_volatile_pointer_chain(volatile int *data, int n) {
    volatile int *ptr = data;
    int sum = 0;
    
    // Volatile pointer prevents some optimizations
    for (int i = 0; i < n; i++) {
        sum += *ptr;
        // The increment should still be recognizable
        ptr = ptr + 1;
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_nested_pointer_access(volatile int **data, int n) {
    int sum = 0;
    
    // Access through pointer to pointer
    for (int i = 0; i < n; i++) {
        volatile int *ptr = data[i];
        sum += *ptr;
        // Modify the pointer for next iteration
        data[i] = ptr + 1;
    }
    
    COMPILER_BARRIER();
    return sum;
}

int main(int argc, char **argv) {
    // Use argc to make loop bounds non-constant
    int n = (argc > 1) ? atoi(argv[1]) : 100;
    if (n < 10) n = 10;
    if (n > 1000) n = 1000;
    
    // Allocate and initialize arrays of different types
    char *char_array = (char *)malloc(n * sizeof(char));
    short *short_array = (short *)malloc(n * sizeof(short));
    int *int_array1 = (int *)malloc(n * sizeof(int));
    int *int_array2 = (int *)malloc(n * sizeof(int));
    long long *ll_array = (long long *)malloc(n * sizeof(long long));
    struct MixedData *struct_array = (struct MixedData *)malloc(n * sizeof(struct MixedData));
    int **ptr_array = (int **)malloc(n * sizeof(int *));
    
    // Initialize data
    for (int i = 0; i < n; i++) {
        char_array[i] = (char)(i % 256);
        short_array[i] = (short)(i * 2);
        int_array1[i] = i * 3;
        int_array2[i] = 0;
        ll_array[i] = (long long)i * i;
        
        struct_array[i].a = i;
        struct_array[i].b = (short)(i * 2);
        struct_array[i].c = (char)(i % 128);
        struct_array[i].d = (long long)i * i * i;
        
        ptr_array[i] = &int_array1[i];
    }
    
    int total_sum = 0;
    
    // Run all test patterns
    total_sum += test_char_pointer_sum((volatile char *)char_array, n);
    total_sum += test_short_pointer_offset((volatile short *)short_array, n);
    total_sum += test_int_pointer_copy((volatile int *)int_array1, 
                                      (volatile int *)int_array2, n);
    total_sum += test_longlong_mixed_index((volatile long long *)ll_array, n);
    total_sum += test_struct_pointer_walk((volatile struct MixedData *)struct_array, n);
    total_sum += test_double_pointer_arithmetic((volatile int *)int_array1,
                                               (volatile int *)int_array2, n);
    total_sum += test_volatile_pointer_chain((volatile int *)int_array1, n);
    total_sum += test_nested_pointer_access((volatile int **)ptr_array, n);
    
    // Verify some results
    int verify_sum = 0;
    for (int i = 0; i < n; i++) {
        verify_sum += int_array2[i];
    }
    
    printf("Total checksum: %d (verify: %d)\n", total_sum, verify_sum);
    
    // Cleanup
    free(char_array);
    free(short_array);
    free(int_array1);
    free(int_array2);
    free(ll_array);
    free(struct_array);
    free(ptr_array);
    
    return 0;
}

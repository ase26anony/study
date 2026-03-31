#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Compiler barriers to prevent reordering but keep pointer arithmetic intact
#define COMPILER_BARRIER() asm volatile("" : : : "memory")

// Test functions marked noinline to preserve function boundaries
__attribute__((noinline))
int test_char_sum(volatile char *data, int n) {
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
int test_short_pointer_arithmetic(volatile short *data, int n) {
    short *ptr = (short *)data;
    int sum = 0;
    
    // Mixed forms: array indexing and pointer arithmetic
    for (int i = 0; i < n; i++) {
        // Access via pointer with offset
        sum += *(ptr + 0);
        // Also use array notation to create complex addressing
        sum += ptr[0];
        ptr += 1;  // Increment after access
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_int_copy_and_increment(volatile int *src, volatile int *dst, int n) {
    int *s = (int *)src;
    int *d = (int *)dst;
    int sum = 0;
    
    // Classic copy with post-increment
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
long long test_longlong_complex_address(volatile long long *data, int n) {
    long long *ptr1 = (long long *)data;
    long long *ptr2 = ptr1;
    long long sum = 0;
    
    // Complex addressing expression that might simplify
    for (int i = 0; i < n; i++) {
        // This creates addressing like *(ptr1 + ptr2 - ptr2)
        // which should become just *ptr1 but requires analysis
        sum += *(ptr1 + (ptr2 - ptr2));
        ptr1++;  // Increment base pointer
    }
    
    COMPILER_BARRIER();
    return sum;
}

// Structure for pointer walking pattern
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
    
    // Walk through array of structures with pointer
    for (int i = 0; i < n; i++) {
        // Access multiple members
        sum += ptr->a;
        sum += ptr->b;
        sum += ptr->c;
        sum += ptr->d;
        ptr++;  // Increment after last access
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_mixed_sizes_and_types(volatile void *data, int n) {
    char *cptr = (char *)data;
    short *sptr = (short *)(cptr + 64);
    int *iptr = (int *)((char *)sptr + 128);
    long long *llptr = (long long *)((char *)iptr + 256);
    
    int sum = 0;
    
    // Access different types with pointer arithmetic
    for (int i = 0; i < n; i++) {
        sum += *cptr;
        cptr++;
        
        sum += *sptr;
        sptr++;
        
        sum += *iptr;
        iptr++;
        
        sum += (int)*llptr;
        llptr++;
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_pointer_with_constant_offset(volatile int *data, int n) {
    int *ptr = (int *)data;
    int sum = 0;
    
    // Use pointer with constant offset
    for (int i = 0; i < n; i++) {
        // Access via *(ptr + 0) - creates simple but non-trivial addressing
        sum += *(ptr + 0);
        // Also try ptr[0] which is equivalent
        sum += ptr[0];
        ptr += 1;  // Increment after access
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_double_pointer_arithmetic(volatile int *data1, volatile int *data2, int n) {
    int *p1 = (int *)data1;
    int *p2 = (int *)data2;
    int sum = 0;
    
    // Two pointers with independent arithmetic
    for (int i = 0; i < n; i++) {
        sum += *p1;
        p1++;
        
        sum += *p2;
        p2++;
    }
    
    COMPILER_BARRIER();
    return sum;
}

int main(int argc, char **argv) {
    // Use argc to make loop bounds non-constant at compile time
    int n = (argc > 1) ? atoi(argv[1]) : 100;
    if (n <= 0) n = 100;
    if (n > 1000) n = 1000;  // Limit for safety
    
    // Allocate and initialize arrays of different types
    char *char_array = malloc(n * sizeof(char));
    short *short_array = malloc(n * sizeof(short));
    int *int_array = malloc(n * sizeof(int));
    int *int_array2 = malloc(n * sizeof(int));
    long long *ll_array = malloc(n * sizeof(long long));
    struct MixedData *struct_array = malloc(n * sizeof(struct MixedData));
    
    // Initialize with pattern
    for (int i = 0; i < n; i++) {
        char_array[i] = (char)(i % 256);
        short_array[i] = (short)(i * 2);
        int_array[i] = i * 3;
        int_array2[i] = i * 4;
        ll_array[i] = (long long)i * 1000;
        struct_array[i].a = i;
        struct_array[i].b = (short)(i * 2);
        struct_array[i].c = (char)(i % 128);
        struct_array[i].d = (long long)i * 10000;
    }
    
    int total_sum = 0;
    
    // Call all test functions with volatile pointers to prevent optimization
    total_sum += test_char_sum((volatile char *)char_array, n);
    total_sum += test_short_pointer_arithmetic((volatile short *)short_array, n);
    total_sum += test_int_copy_and_increment((volatile int *)int_array, 
                                            (volatile int *)int_array2, n);
    total_sum += test_longlong_complex_address((volatile long long *)ll_array, n);
    total_sum += test_struct_pointer_walk((volatile struct MixedData *)struct_array, n);
    
    // Create mixed buffer for multi-type test
    void *mixed_buffer = malloc(1024);
    memset(mixed_buffer, 0, 1024);
    memcpy(mixed_buffer, char_array, n);
    memcpy((char *)mixed_buffer + 64, short_array, n * sizeof(short));
    memcpy((char *)mixed_buffer + 192, int_array, n * sizeof(int));
    memcpy((char *)mixed_buffer + 448, ll_array, n * sizeof(long long));
    
    total_sum += test_mixed_sizes_and_types((volatile void *)mixed_buffer, n / 4);
    total_sum += test_pointer_with_constant_offset((volatile int *)int_array, n);
    total_sum += test_double_pointer_arithmetic((volatile int *)int_array, 
                                               (volatile int *)int_array2, n);
    
    printf("Total checksum: %d\n", total_sum);
    
    // Free allocated memory
    free(char_array);
    free(short_array);
    free(int_array);
    free(int_array2);
    free(ll_array);
    free(struct_array);
    free(mixed_buffer);
    
    return 0;
}

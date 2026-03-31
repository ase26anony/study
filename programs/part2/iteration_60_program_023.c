#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Compiler barriers to prevent unwanted optimizations
#define COMPILER_BARRIER() asm volatile("" : : : "memory")

// Non-inline test functions to preserve boundaries
__attribute__((noinline))
static int test_char_sum(volatile char *data, int n) {
    char *ptr = (char *)data;
    int sum = 0;
    
    // Simple pointer increment pattern
    for (int i = 0; i < n; i++) {
        sum += *ptr;
        ptr++;  // Adjacent increment - should trigger auto-inc-dec
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
static int test_short_pointer_offset(volatile short *data, int n) {
    short *ptr = (short *)data;
    int sum = 0;
    
    // Pointer with offset access followed by increment
    for (int i = 0; i < n; i++) {
        // Complex addressing: *(ptr + 0) - forces XEXP(x, 0) extraction
        sum += *(ptr + 0);
        ptr += 1;  // Increment by element size
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
static int test_int_mixed_index_pointer(volatile int *data, int n) {
    int *ptr = (int *)data;
    int sum = 0;
    
    // Mixed indexing and pointer arithmetic
    for (int i = 0; i < n; i++) {
        // Array indexing form: ptr[0]
        sum += ptr[0];
        // Pointer arithmetic form: ptr = ptr + 1
        ptr = ptr + 1;
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
static long long test_longlong_copy(volatile long long *src, 
                                   volatile long long *dst, int n) {
    long long *s = (long long *)src;
    long long *d = (long long *)dst;
    long long checksum = 0;
    
    // Classic copy pattern with post-increment
    for (int i = 0; i < n; i++) {
        *d = *s;
        checksum += *d;
        s++;
        d++;
    }
    
    COMPILER_BARRIER();
    return checksum;
}

__attribute__((noinline))
static int test_double_pointer_arithmetic(volatile int *data1, 
                                         volatile int *data2, int n) {
    int *p1 = (int *)data1;
    int *p2 = (int *)data2;
    int sum = 0;
    
    // Complex pointer arithmetic that might create base-plus-index
    for (int i = 0; i < n; i++) {
        // Expression: *(p1 + (p2 - p2)) simplifies but requires analysis
        sum += *(p1 + (p2 - p2));
        p1++;
        // Keep p2 moving differently to prevent complete simplification
        p2 = (int *)((char *)p2 + 1);
    }
    
    COMPILER_BARRIER();
    return sum;
}

// Structure for pointer walking pattern
struct MixedData {
    char c;
    short s;
    int i;
    long long ll;
};

__attribute__((noinline))
static long long test_struct_pointer_walk(volatile struct MixedData *data, int n) {
    struct MixedData *ptr = (struct MixedData *)data;
    long long sum = 0;
    
    // Structure access with pointer increment
    for (int i = 0; i < n; i++) {
        // Access multiple members
        sum += ptr->c;
        sum += ptr->s;
        sum += ptr->i;
        sum += ptr->ll;
        
        // Pointer increment after last access
        ptr++;
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
static int test_volatile_pointer_chain(volatile int *data, int n) {
    volatile int * volatile vptr = data;
    int sum = 0;
    
    // Volatile pointer chain to prevent simplification
    for (int i = 0; i < n; i++) {
        // Access through volatile pointer
        sum += *vptr;
        // Increment volatile pointer
        vptr = vptr + 1;
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
static int test_nested_pointer_access(volatile int **data, int n) {
    int sum = 0;
    
    // Pointer to pointer access pattern
    for (int i = 0; i < n; i++) {
        volatile int *ptr = data[i];
        // Dereference and increment
        sum += *ptr;
        // Modify the pointer in the array
        data[i] = ptr + 1;
    }
    
    COMPILER_BARRIER();
    return sum;
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
    int **ptr_array = malloc(base_size * sizeof(int *));
    
    // Initialize data
    for (int i = 0; i < base_size; i++) {
        char_array[i] = (char)(i % 256);
        short_array[i] = (short)(i * 2);
        int_array1[i] = i * 3;
        int_array2[i] = i * 4;
        ll_array1[i] = i * 5LL;
        ll_array2[i] = 0;
        
        struct_array[i].c = (char)(i % 256);
        struct_array[i].s = (short)(i * 2);
        struct_array[i].i = i * 3;
        struct_array[i].ll = i * 5LL;
        
        ptr_array[i] = &int_array1[i];
    }
    
    long long total = 0;
    
    // Call test functions with volatile casts to prevent compile-time optimization
    total += test_char_sum((volatile char *)char_array, base_size);
    total += test_short_pointer_offset((volatile short *)short_array, base_size);
    total += test_int_mixed_index_pointer((volatile int *)int_array1, base_size);
    total += test_longlong_copy((volatile long long *)ll_array1,
                               (volatile long long *)ll_array2, base_size);
    total += test_double_pointer_arithmetic((volatile int *)int_array1,
                                           (volatile int *)int_array2, base_size);
    total += test_struct_pointer_walk((volatile struct MixedData *)struct_array, base_size);
    total += test_volatile_pointer_chain((volatile int *)int_array1, base_size);
    total += test_nested_pointer_access((volatile int **)ptr_array, base_size);
    
    printf("Total checksum: %lld\n", total);
    
    // Cleanup
    free(char_array);
    free(short_array);
    free(int_array1);
    free(int_array2);
    free(ll_array1);
    free(ll_array2);
    free(struct_array);
    free(ptr_array);
    
    return 0;
}

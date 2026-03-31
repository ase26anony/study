#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Compiler barrier to prevent reordering
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
int test_short_pointer_offset(volatile short *data, int n, int offset) {
    short *ptr = (short *)data + offset;
    int sum = 0;
    
    // Pointer with offset and increment
    for (int i = 0; i < n; i++) {
        sum += *ptr;
        ptr += 1;  // Increment by stride
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_int_pointer_complex(volatile int *data1, volatile int *data2, int n) {
    int *p1 = (int *)data1;
    int *p2 = (int *)data2;
    int sum = 0;
    
    // Mixed pointer arithmetic that might create complex addressing
    for (int i = 0; i < n; i++) {
        // Access via pointer with offset
        sum += *(p1 + 0);
        sum += p2[0];
        
        // Increment pointers
        p1++;
        p2++;
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
long long test_longlong_copy(volatile long long *src, volatile long long *dst, int n) {
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
int test_mixed_access_sizes(volatile char *cdata, volatile short *sdata, 
                           volatile int *idata, int n) {
    char *cp = (char *)cdata;
    short *sp = (short *)sdata;
    int *ip = (int *)idata;
    int sum = 0;
    
    // Mixed size accesses with pointer increments
    for (int i = 0; i < n; i++) {
        sum += *cp;
        cp++;
        
        sum += *sp;
        sp++;
        
        sum += *ip;
        ip++;
    }
    
    COMPILER_BARRIER();
    return sum;
}

// Structure for pointer walking
struct MixedData {
    char c;
    int i;
    short s;
    long long ll;
};

__attribute__((noinline))
long long test_struct_pointer_walk(volatile struct MixedData *data, int n) {
    struct MixedData *ptr = (struct MixedData *)data;
    long long sum = 0;
    
    // Walk through array of structures
    for (int i = 0; i < n; i++) {
        // Access multiple members
        sum += ptr->c;
        sum += ptr->i;
        sum += ptr->s;
        sum += ptr->ll;
        
        // Increment pointer after last access
        ptr++;
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_double_pointer_arithmetic(volatile int *base, int n) {
    int *ptr = (int *)base;
    int sum = 0;
    
    // Complex pointer arithmetic that might need analysis
    for (int i = 0; i < n; i++) {
        // Access with offset that should simplify
        sum += *(ptr + 0);
        
        // Another access with different offset
        sum += ptr[0];
        
        // Increment
        ptr += 1;
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_pointer_arithmetic_in_loop(volatile int *data, int n, int stride) {
    int *ptr = (int *)data;
    int sum = 0;
    
    // Variable stride increment
    for (int i = 0; i < n; i++) {
        sum += *ptr;
        ptr += stride;  // Non-unity stride
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
    long long *ll_array1 = (long long *)malloc(n * sizeof(long long));
    long long *ll_array2 = (long long *)malloc(n * sizeof(long long));
    struct MixedData *struct_array = (struct MixedData *)malloc(n * sizeof(struct MixedData));
    
    // Initialize with pattern
    for (int i = 0; i < n; i++) {
        char_array[i] = (char)(i % 256);
        short_array[i] = (short)(i * 2);
        int_array1[i] = i * 3;
        int_array2[i] = i * 5;
        ll_array1[i] = i * 7LL;
        ll_array2[i] = i * 11LL;
        
        struct_array[i].c = (char)(i % 128);
        struct_array[i].i = i * 13;
        struct_array[i].s = (short)(i * 17);
        struct_array[i].ll = i * 19LL;
    }
    
    long long total = 0;
    
    // Call test functions with volatile pointers to prevent optimization
    total += test_char_pointer_sum((volatile char *)char_array, n);
    total += test_short_pointer_offset((volatile short *)short_array, n, 0);
    total += test_int_pointer_complex((volatile int *)int_array1, 
                                     (volatile int *)int_array2, n);
    total += test_longlong_copy((volatile long long *)ll_array1,
                               (volatile long long *)ll_array2, n);
    total += test_mixed_access_sizes((volatile char *)char_array,
                                    (volatile short *)short_array,
                                    (volatile int *)int_array1, n);
    total += test_struct_pointer_walk((volatile struct MixedData *)struct_array, n);
    total += test_double_pointer_arithmetic((volatile int *)int_array1, n);
    total += test_pointer_arithmetic_in_loop((volatile int *)int_array2, n, 1);
    
    printf("Total checksum: %lld\n", total);
    
    // Cleanup
    free(char_array);
    free(short_array);
    free(int_array1);
    free(int_array2);
    free(ll_array1);
    free(ll_array2);
    free(struct_array);
    
    return (int)(total % 256);
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Compiler barriers to prevent reordering but keep pointer arithmetic intact
#define COMPILER_BARRIER() asm volatile("" : : : "memory")

// Test functions marked noinline to preserve function boundaries
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
int test_mixed_index_pointer(int *array, int n) {
    int sum = 0;
    volatile int *vptr = array;  // Volatile to prevent simplification
    int *ptr = (int *)vptr;
    
    // Mixed indexing and pointer arithmetic
    for (int i = 0; i < n; i++) {
        // Array index access
        sum += array[i];
        // Pointer arithmetic in same iteration
        ptr = array + i + 1;
        if (ptr < array + n) {
            sum += *ptr;
        }
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
void test_pointer_copy(int *dst, int *src, int n) {
    int *d = dst;
    int *s = src;
    int *end = src + n;
    
    // Classic copy pattern with post-increment
    while (s < end) {
        *d = *s;
        d++;
        s++;  // Two pointers incrementing - should trigger pattern matching
    }
    
    COMPILER_BARRIER();
}

__attribute__((noinline))
long long test_different_sizes(char *carray, short *sarray, 
                               int *iarray, long long *llarray, int n) {
    long long total = 0;
    
    // Char pointer with offset
    char *cptr = carray;
    for (int i = 0; i < n; i++) {
        total += *(cptr + i);  // Offset addressing
        cptr++;  // Also increment pointer
    }
    
    // Short pointer with complex expression
    short *sptr = sarray;
    volatile short *vsptr = sarray;  // Volatile base
    for (int i = 0; i < n; i++) {
        // Complex address expression: *(base + (ptr - ptr))
        total += *(sptr + (vsptr - vsptr));
        sptr += 2;  // Stride of 2 for short
    }
    
    // Int pointer with multiple base registers simulation
    int *iptr1 = iarray;
    int *iptr2 = iarray + n/2;
    for (int i = 0; i < n/2; i++) {
        // Access using two pointers that could be seen as base+index
        total += iptr1[iptr2 - iptr2];  // Should simplify to iptr1[0]
        iptr1++;
    }
    
    // Long long pointer with simple increment
    long long *llptr = llarray;
    for (int i = 0; i < n; i++) {
        total += *llptr;
        llptr++;  // DImode access with increment
    }
    
    COMPILER_BARRIER();
    return total;
}

// Structure for pointer walking
struct MixedData {
    int a;
    char b;
    short c;
    int d;
};

__attribute__((noinline))
int test_structure_walk(struct MixedData *data, int n) {
    int sum = 0;
    struct MixedData *ptr = data;
    
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
int test_complex_addressing(int *array, int n) {
    int sum = 0;
    volatile int *vbase = array;  // Volatile to prevent constant folding
    
    // Complex addressing expressions
    for (int i = 0; i < n; i++) {
        // Multiple addressing forms
        sum += vbase[i];                    // Index form
        sum += *(vbase + i);                // Pointer+offset form
        sum += *(array + (vbase - vbase) + i); // Complex base expression
        
        // Force analysis of XEXP(x, 0)
        int *temp = (int *)vbase;
        sum += *temp;
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
void test_fill_pattern(int *array, int n, int value) {
    int *ptr = array;
    int *end = array + n;
    
    // Fill with post-increment write
    while (ptr < end) {
        *ptr = value;
        ptr++;  // Write with increment
    }
    
    COMPILER_BARRIER();
}

int main(int argc, char **argv) {
    // Use argc to make loop bounds non-constant at compile time
    int n = (argc > 1) ? atoi(argv[1]) : 100;
    if (n < 10) n = 10;
    if (n > 1000) n = 1000;
    
    // Allocate arrays of different types
    int *int_array1 = (int *)malloc(n * sizeof(int));
    int *int_array2 = (int *)malloc(n * sizeof(int));
    char *char_array = (char *)malloc(n * sizeof(char));
    short *short_array = (short *)malloc(n * sizeof(short));
    long long *ll_array = (long long *)malloc(n * sizeof(long long));
    struct MixedData *struct_array = (struct MixedData *)malloc(n * sizeof(struct MixedData));
    
    // Initialize arrays
    for (int i = 0; i < n; i++) {
        int_array1[i] = i;
        int_array2[i] = 0;
        char_array[i] = (char)(i % 128);
        short_array[i] = (short)(i * 2);
        ll_array[i] = i * 100LL;
        struct_array[i].a = i;
        struct_array[i].b = (char)(i % 256);
        struct_array[i].c = (short)(i * 3);
        struct_array[i].d = i * 4;
    }
    
    int checksum = 0;
    
    // Run all test patterns
    checksum += test_pointer_increment_sum(int_array1, n);
    
    test_pointer_copy(int_array2, int_array1, n);
    checksum += test_pointer_increment_sum(int_array2, n);
    
    checksum += test_mixed_index_pointer(int_array1, n);
    
    long long size_test = test_different_sizes(char_array, short_array, 
                                               int_array1, ll_array, n);
    checksum += (int)(size_test & 0xFFFFFFFF) + (int)(size_test >> 32);
    
    checksum += test_structure_walk(struct_array, n);
    
    checksum += test_complex_addressing(int_array1, n);
    
    test_fill_pattern(int_array2, n, 42);
    checksum += test_pointer_increment_sum(int_array2, n);
    
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

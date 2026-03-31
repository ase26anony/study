#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Compiler barrier to prevent reordering while preserving patterns
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
    
    // Copy with post-increment pattern
    while (s < end) {
        *d = *s;
        d++;
        s++;  // Two pointers incrementing
    }
    
    COMPILER_BARRIER();
}

__attribute__((noinline))
long long test_mixed_access_sizes(char *carr, short *sarr, int *iarr, long long *llarr, int n) {
    long long total = 0;
    
    // Mixed pointer types with increments
    char *cp = carr;
    short *sp = sarr;
    int *ip = iarr;
    long long *lp = llarr;
    
    for (int i = 0; i < n; i++) {
        total += *cp;
        cp++;  // char pointer increment
        
        total += *sp;
        sp++;  // short pointer increment
        
        total += *ip;
        ip++;  // int pointer increment
        
        total += *lp;
        lp++;  // long long pointer increment
    }
    
    COMPILER_BARRIER();
    return total;
}

__attribute__((noinline))
int test_complex_addressing(int *array, int n, int offset) {
    int sum = 0;
    int *ptr = array;
    
    // Complex addressing: *(ptr + offset) with ptr increment
    for (int i = 0; i < n; i++) {
        sum += *(ptr + offset);  // Non-trivial address expression
        ptr++;  // Increment after access
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_dual_pointer_arithmetic(int *arr1, int *arr2, int n) {
    int sum = 0;
    int *p1 = arr1;
    int *p2 = arr2;
    
    // Using two pointers with arithmetic that could simplify
    for (int i = 0; i < n; i++) {
        // Complex address: p1[p2 - p2] which simplifies to p1[0]
        // but requires analysis to reach XEXP(x, 0)
        sum += p1[p2 - p2];
        p1++;
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_volatile_pointer(int *array, volatile int *volatile_ptr, int n) {
    int sum = 0;
    volatile int *vp = volatile_ptr;
    int *ptr = array;
    
    // Use volatile pointer to prevent constant folding
    for (int i = 0; i < n; i++) {
        sum += *ptr;
        ptr = (int *)((char *)ptr + *vp);  // Volatile increment amount
    }
    
    COMPILER_BARRIER();
    return sum;
}

// Structure for pointer walking test
struct MixedData {
    int a;
    char b;
    short c;
    int d;
};

__attribute__((noinline))
int test_structure_pointer_walk(struct MixedData *data, int n) {
    int sum = 0;
    struct MixedData *ptr = data;
    
    // Walk through structure array with pointer
    for (int i = 0; i < n; i++) {
        sum += ptr->a;
        sum += ptr->b;
        sum += ptr->c;
        sum += ptr->d;
        ptr++;  // Increment after multiple accesses
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_mixed_index_and_pointer(int *array, int n) {
    int sum = 0;
    int *ptr = array;
    
    // Mix array indexing and pointer arithmetic
    for (int i = 0; i < n; i++) {
        sum += array[i];  // Indexed access
        sum += *ptr;      // Pointer access
        ptr++;            // Pointer increment
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
        sum += ptr[4];    // Constant offset access
        ptr++;            // Base pointer increment
    }
    
    COMPILER_BARRIER();
    return sum;
}

int main(int argc, char **argv) {
    // Use argc to make loop bounds non-constant
    int n = (argc > 1) ? atoi(argv[1]) : 100;
    if (n < 10) n = 100;
    
    // Allocate and initialize arrays of different types
    int *int_array = malloc(n * sizeof(int));
    int *int_array2 = malloc(n * sizeof(int));
    char *char_array = malloc(n * sizeof(char));
    short *short_array = malloc(n * sizeof(short));
    long long *ll_array = malloc(n * sizeof(long long));
    struct MixedData *struct_array = malloc(n * sizeof(struct MixedData));
    volatile int volatile_inc = 4;  // Volatile increment amount
    
    // Initialize arrays
    for (int i = 0; i < n; i++) {
        int_array[i] = i;
        int_array2[i] = i * 2;
        char_array[i] = (char)(i % 256);
        short_array[i] = (short)(i % 65536);
        ll_array[i] = i * 100LL;
        struct_array[i].a = i;
        struct_array[i].b = (char)(i % 256);
        struct_array[i].c = (short)(i % 65536);
        struct_array[i].d = i * 3;
    }
    
    int checksum = 0;
    
    // Run all test patterns
    checksum += test_pointer_increment_sum(int_array, n);
    
    test_pointer_copy(int_array2, int_array, n);
    checksum += int_array2[n/2];  // Sample check
    
    checksum += test_mixed_access_sizes(char_array, short_array, int_array, ll_array, n/4);
    
    checksum += test_complex_addressing(int_array, n, 2);
    
    checksum += test_dual_pointer_arithmetic(int_array, int_array2, n);
    
    checksum += test_volatile_pointer(int_array, &volatile_inc, n);
    
    checksum += test_structure_pointer_walk(struct_array, n);
    
    checksum += test_mixed_index_and_pointer(int_array, n);
    
    checksum += test_pointer_with_constant_offset(int_array, n);
    
    printf("Final checksum: %d\n", checksum);
    
    // Cleanup
    free(int_array);
    free(int_array2);
    free(char_array);
    free(short_array);
    free(ll_array);
    free(struct_array);
    
    return 0;
}

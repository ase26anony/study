#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Compiler barrier to prevent reordering
#define COMPILER_BARRIER() asm volatile("" : : : "memory")

// Test functions marked noinline to preserve boundaries
__attribute__((noinline))
int test_pointer_increment_sum(int *array, volatile int n) {
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
void test_pointer_copy(int *dest, int *src, volatile int n) {
    int *d = dest;
    int *s = src;
    int *end = src + n;
    
    // Copy with post-increment on both sides
    while (s < end) {
        *d = *s;
        d++;
        s++;  // Two auto-inc opportunities here
    }
    
    COMPILER_BARRIER();
}

__attribute__((noinline))
long long test_mixed_access_sizes(char *carr, short *sarr, int *iarr, 
                                   long long *llarr, volatile int n) {
    long long total = 0;
    char *cp = carr;
    short *sp = sarr;
    int *ip = iarr;
    long long *llp = llarr;
    
    // Mixed pointer types with increments
    for (int i = 0; i < n; i++) {
        total += *cp;
        cp++;
        
        total += *sp;
        sp++;
        
        total += *ip;
        ip++;
        
        total += *llp;
        llp++;
    }
    
    COMPILER_BARRIER();
    return total;
}

__attribute__((noinline))
int test_complex_addressing(int *array, volatile int n, volatile int offset) {
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
int test_structure_access(void *data, volatile int n) {
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
        total += sp->a + sp->b;  // Multiple member accesses
        sp++;  // Pointer increment after last access
    }
    
    COMPILER_BARRIER();
    return total;
}

__attribute__((noinline))
int test_dual_pointer_arithmetic(int *arr1, int *arr2, volatile int n) {
    int sum = 0;
    int *p1 = arr1;
    int *p2 = arr2;
    
    // Using two pointers with arithmetic that could be base+index
    for (int i = 0; i < n; i++) {
        // Complex address expression: p1[p2 - p2] simplifies but requires analysis
        sum += p1[p2 - p2];  // Should become *p1
        p1++;
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_volatile_pointer(volatile int *vptr, volatile int n) {
    int sum = 0;
    volatile int *ptr = vptr;
    
    // Volatile pointer prevents some optimizations
    for (int i = 0; i < n; i++) {
        sum += *ptr;
        ptr++;  // Increment volatile pointer
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_mixed_index_and_pointer(int *array, volatile int n) {
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
void test_fill_with_increment(int *array, volatile int value, volatile int n) {
    int *ptr = array;
    int *end = array + n;
    
    // Fill array with pointer write and increment
    while (ptr < end) {
        *ptr = value;
        ptr++;  // Post-increment write
    }
    
    COMPILER_BARRIER();
}

int main(int argc, char **argv) {
    // Use argc to make loop bounds non-constant at compile time
    volatile int size = (argc > 1) ? atoi(argv[1]) : 100;
    if (size < 10) size = 100;
    
    // Allocate arrays of different types
    int *int_array1 = (int*)malloc(size * sizeof(int));
    int *int_array2 = (int*)malloc(size * sizeof(int));
    char *char_array = (char*)malloc(size * sizeof(char));
    short *short_array = (short*)malloc(size * sizeof(short));
    long long *ll_array = (long long*)malloc(size * sizeof(long long));
    struct S *struct_array = (struct S*)malloc(size * sizeof(struct S));
    
    // Initialize arrays
    for (int i = 0; i < size; i++) {
        int_array1[i] = i;
        int_array2[i] = size - i;
        char_array[i] = (char)(i % 256);
        short_array[i] = (short)(i * 2);
        ll_array[i] = (long long)i * 1000;
        struct_array[i].a = i;
        struct_array[i].b = i * 2;
        struct_array[i].c = (char)(i % 128);
        struct_array[i].d = (short)(i * 3);
    }
    
    int result = 0;
    
    // Run all test patterns
    result += test_pointer_increment_sum(int_array1, size);
    
    test_pointer_copy(int_array2, int_array1, size);
    result += int_array2[size/2];  // Use copied value
    
    result += test_mixed_access_sizes(char_array, short_array, 
                                      int_array1, ll_array, size);
    
    volatile int offset = 0;
    result += test_complex_addressing(int_array1, size, offset);
    
    result += test_structure_access(struct_array, size);
    
    result += test_dual_pointer_arithmetic(int_array1, int_array2, size);
    
    volatile int *vptr = int_array1;
    result += test_volatile_pointer(vptr, size);
    
    result += test_mixed_index_and_pointer(int_array1, size);
    
    test_fill_with_increment(int_array2, 42, size);
    result += int_array2[size/2];  // Check filled value
    
    // Cleanup
    free(int_array1);
    free(int_array2);
    free(char_array);
    free(short_array);
    free(ll_array);
    free(struct_array);
    
    printf("Result: %d\n", result);
    return result != 0 ? 0 : 1;
}

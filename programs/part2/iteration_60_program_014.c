#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Compiler barrier to prevent reordering
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
    
    // Copy with post-increment on both pointers
    while (s < end) {
        *d = *s;
        d++;
        s++;  // Two auto-inc opportunities
    }
    
    COMPILER_BARRIER();
}

__attribute__((noinline))
long long test_mixed_types(char *carr, short *sarr, int *iarr, long long *llarr, int n) {
    long long total = 0;
    char *cp = carr;
    short *sp = sarr;
    int *ip = iarr;
    long long *llp = llarr;
    
    // Mixed type accesses with pointer arithmetic
    for (int i = 0; i < n; i++) {
        total += *cp;
        cp++;  // char pointer increment
        
        total += *sp;
        sp++;  // short pointer increment
        
        total += *ip;
        ip++;  // int pointer increment
        
        total += *llp;
        llp++;  // long long pointer increment
    }
    
    COMPILER_BARRIER();
    return total;
}

__attribute__((noinline))
int test_complex_addressing(int *array, int n, int offset) {
    int sum = 0;
    volatile int *volatile_ptr = array;  // Volatile to prevent optimization
    int *ptr = (int*)volatile_ptr;
    
    // Complex addressing: *(ptr + offset) with ptr increment
    for (int i = 0; i < n; i++) {
        // This creates XEXP(x, 0) = ptr, XEXP(x, 1) = offset
        sum += ptr[offset];
        
        // Mixed form: array indexing and pointer arithmetic
        sum += array[i];
        
        ptr++;  // Pointer increment after complex access
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_structure_access(void *data, int n) {
    struct S {
        int a;
        int b;
        char c;
        short d;
    };
    
    struct S *sp = (struct S*)data;
    int total = 0;
    
    // Structure access with pointer walking
    for (int i = 0; i < n; i++) {
        // Multiple member accesses
        total += sp->a;
        total += sp->b;
        total += sp->c;
        total += sp->d;
        
        sp++;  // Structure pointer increment
    }
    
    COMPILER_BARRIER();
    return total;
}

__attribute__((noinline))
int test_dual_pointer_arithmetic(int *arr1, int *arr2, int n) {
    int sum = 0;
    int *p1 = arr1;
    int *p2 = arr2;
    
    // Dual pointer arithmetic that might create base+index addressing
    for (int i = 0; i < n; i++) {
        // Expression: *(p1 + (p2 - p2)) simplifies to *p1 but requires analysis
        sum += p1[p2 - p2];
        
        // Regular access
        sum += *p2;
        
        p1++;
        p2++;
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
void test_pointer_with_constant_offset(int *array, int n) {
    int *ptr = array;
    
    // Access with constant offset then increment
    for (int i = 0; i < n; i++) {
        // Creates address expression: MEM[(int *)ptr_1 + 4B]
        int val = *(ptr + 1);
        // Use val to prevent dead code elimination
        *(ptr) = val + i;
        
        ptr++;  // Increment after offset access
    }
    
    COMPILER_BARRIER();
}

// Main function with non-constant loop bounds
int main(int argc, char **argv) {
    // Use argc to make loop bounds non-constant
    int size = (argc > 1) ? atoi(argv[1]) : 100;
    if (size < 10) size = 100;
    
    // Allocate arrays of different types
    char *char_array = (char*)malloc(size * sizeof(char));
    short *short_array = (short*)malloc(size * sizeof(short));
    int *int_array1 = (int*)malloc(size * sizeof(int));
    int *int_array2 = (int*)malloc(size * sizeof(int));
    long long *ll_array = (long long*)malloc(size * sizeof(long long));
    struct S *struct_array = (struct S*)malloc(size * sizeof(struct S));
    
    // Initialize arrays
    for (int i = 0; i < size; i++) {
        char_array[i] = (char)(i % 256);
        short_array[i] = (short)(i * 2);
        int_array1[i] = i * 3;
        int_array2[i] = i * 4;
        ll_array[i] = i * 5LL;
        struct_array[i].a = i;
        struct_array[i].b = i * 2;
        struct_array[i].c = (char)(i % 128);
        struct_array[i].d = (short)(i % 32767);
    }
    
    int result = 0;
    
    // Run all test patterns
    result += test_pointer_increment_sum(int_array1, size);
    
    test_pointer_copy(int_array2, int_array1, size);
    result += int_array2[size/2];  // Use copied value
    
    result += test_mixed_types(char_array, short_array, int_array1, ll_array, size / 4);
    
    result += test_complex_addressing(int_array1, size, 2);
    
    result += test_structure_access(struct_array, size);
    
    result += test_dual_pointer_arithmetic(int_array1, int_array2, size);
    
    test_pointer_with_constant_offset(int_array1, size);
    result += int_array1[size/3];
    
    // Cleanup
    free(char_array);
    free(short_array);
    free(int_array1);
    free(int_array2);
    free(ll_array);
    free(struct_array);
    
    printf("Result: %d\n", result);
    return 0;
}

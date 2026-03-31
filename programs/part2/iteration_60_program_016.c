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
    char *cp = carr;
    short *sp = sarr;
    int *ip = iarr;
    long long *llp = llarr;
    
    // Mixed access sizes with pointer arithmetic
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
    int *base = array;
    volatile int *volatile_ptr = array;  // Volatile to prevent simplification
    
    // Complex addressing: *(ptr + offset) with ptr increment
    for (int i = 0; i < n; i++) {
        // Multiple addressing forms
        sum += *(base + offset);      // Base + offset
        sum += volatile_ptr[i];       // Array indexing with volatile
        sum += *(base++);             // Post-increment
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_structure_access(int n) {
    struct Data {
        int a;
        int b;
        int c;
        int d;
    };
    
    struct Data *array = malloc(n * sizeof(struct Data));
    if (!array) return 0;
    
    // Initialize
    for (int i = 0; i < n; i++) {
        array[i].a = i;
        array[i].b = i * 2;
        array[i].c = i * 3;
        array[i].d = i * 4;
    }
    
    int sum = 0;
    struct Data *ptr = array;
    
    // Structure access with pointer walking
    for (int i = 0; i < n; i++) {
        sum += ptr->a;
        sum += ptr->b;
        sum += ptr->c;
        sum += ptr->d;
        ptr++;  // Increment after all accesses
    }
    
    COMPILER_BARRIER();
    free(array);
    return sum;
}

__attribute__((noinline))
int test_dual_pointer_arithmetic(int *arr1, int *arr2, int n) {
    int sum = 0;
    int *p1 = arr1;
    int *p2 = arr2;
    
    // Complex pointer arithmetic that might create base+index addressing
    for (int i = 0; i < n; i++) {
        // Expression that could be seen as base-plus-index
        sum += p1[i];           // Array indexing
        sum += *(p2 + i);       // Pointer + index
        
        // More complex: p1[p2 - p2] which simplifies to p1[0]
        // but requires analysis
        sum += p1[p2 - p2];
        
        p1++;
        p2 += 2;  // Different strides
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_loop_with_volatile_bound(volatile int *data, volatile int count) {
    int sum = 0;
    int *ptr = (int*)data;  // Cast from volatile
    
    // Loop bound from volatile prevents constant propagation
    for (int i = 0; i < count; i++) {
        sum += *ptr;
        ptr++;  // Should trigger auto-inc pattern
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
void test_write_with_increment(int *array, int n, int value) {
    int *ptr = array;
    int *end = array + n;
    
    // Write pattern with increment
    while (ptr < end) {
        *ptr = value;
        ptr++;  // Post-increment after write
    }
    
    COMPILER_BARRIER();
}

int main(int argc, char **argv) {
    // Use argc to make sizes non-constant at compile time
    int size = (argc > 1) ? atoi(argv[1]) : 100;
    if (size < 10) size = 100;
    
    // Allocate arrays of different types
    char *char_array = malloc(size * sizeof(char));
    short *short_array = malloc(size * sizeof(short));
    int *int_array1 = malloc(size * sizeof(int));
    int *int_array2 = malloc(size * sizeof(int));
    long long *ll_array = malloc(size * sizeof(long long));
    
    if (!char_array || !short_array || !int_array1 || !int_array2 || !ll_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    // Initialize arrays
    for (int i = 0; i < size; i++) {
        char_array[i] = (char)(i % 256);
        short_array[i] = (short)(i * 2);
        int_array1[i] = i * 3;
        int_array2[i] = i * 4;
        ll_array[i] = (long long)i * 1000;
    }
    
    int result = 0;
    volatile int volatile_size = size;  // Volatile to prevent optimization
    
    // Run all test patterns
    result += test_pointer_increment_sum(int_array1, volatile_size);
    
    test_pointer_copy(int_array2, int_array1, volatile_size);
    result += int_array2[volatile_size / 2];
    
    result += test_mixed_access_sizes(char_array, short_array, 
                                      int_array1, ll_array, 
                                      volatile_size / 4);
    
    result += test_complex_addressing(int_array1, volatile_size, 5);
    
    result += test_structure_access(volatile_size / 2);
    
    result += test_dual_pointer_arithmetic(int_array1, int_array2, volatile_size / 2);
    
    result += test_loop_with_volatile_bound(int_array1, volatile_size / 2);
    
    test_write_with_increment(int_array2, volatile_size, 42);
    result += int_array2[volatile_size - 1];
    
    // Cleanup
    free(char_array);
    free(short_array);
    free(int_array1);
    free(int_array2);
    free(ll_array);
    
    printf("Result: %d\n", result);
    return 0;
}

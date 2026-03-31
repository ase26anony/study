#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Compiler barrier to prevent reordering while keeping patterns intact
#define COMPILER_BARRIER() asm volatile("" : : : "memory")

// Test functions marked noinline to preserve boundaries
__attribute__((noinline))
int test_pointer_increment_sum(int *array, volatile int n) {
    int sum = 0;
    int *ptr = array;
    int *end = array + n;
    
    // Pattern: *ptr followed by ptr++ - should trigger auto-inc pattern
    while (ptr < end) {
        sum += *ptr;
        ptr++;  // Adjacent increment after memory access
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
void test_pointer_copy(volatile int *src, int *dst, volatile int n) {
    volatile int *s = src;
    int *d = dst;
    int *end = d + n;
    
    // Pattern: *d = *s with both pointers incrementing
    while (d < end) {
        *d = *s;
        d++;
        s++;
    }
    
    COMPILER_BARRIER();
}

__attribute__((noinline))
long long test_mixed_access_sizes(char *carr, short *sarr, int *iarr, 
                                   long long *llarr, volatile int n) {
    long long total = 0;
    
    // Mixed pointer types with increments
    char *cp = carr;
    short *sp = sarr;
    int *ip = iarr;
    long long *lp = llarr;
    
    for (int i = 0; i < n; i++) {
        // Multiple memory accesses with pointer increments
        total += *cp;
        cp++;
        
        total += *sp;
        sp++;
        
        total += *ip;
        ip++;
        
        total += *lp;
        lp++;
    }
    
    COMPILER_BARRIER();
    return total;
}

__attribute__((noinline))
int test_complex_addressing(int *base, volatile int offset, volatile int n) {
    int sum = 0;
    int *ptr = base;
    
    // Complex addressing: *(ptr + offset) with ptr increment
    // This creates non-trivial address expressions
    for (int i = 0; i < n; i++) {
        sum += *(ptr + offset);
        ptr++;  // Increment after access with offset
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_double_pointer_arithmetic(int *arr1, int *arr2, volatile int n) {
    int sum = 0;
    int *p1 = arr1;
    int *p2 = arr2;
    
    // Using two pointers with arithmetic that might simplify
    for (int i = 0; i < n; i++) {
        // Complex address: p1[p2 - p2] which simplifies to *p1
        // but requires analysis to reach XEXP(x, 0)
        sum += p1[p2 - p2];
        p1++;
    }
    
    COMPILER_BARRIER();
    return sum;
}

// Structure for pointer walking pattern
struct Data {
    int a;
    int b;
    short c;
    char d;
};

__attribute__((noinline))
int test_structure_pointer_walk(struct Data *data, volatile int n) {
    int sum = 0;
    struct Data *ptr = data;
    
    // Pattern: Access multiple structure members then increment pointer
    for (int i = 0; i < n; i++) {
        sum += ptr->a;
        sum += ptr->b;
        sum += ptr->c;
        sum += ptr->d;
        ptr++;  // Increment after last member access
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_volatile_pointer_arithmetic(volatile int *vptr, volatile int n) {
    int sum = 0;
    volatile int *vp = vptr;
    
    // Volatile pointer prevents constant folding of address calculations
    for (int i = 0; i < n; i++) {
        sum += *vp;
        vp++;  // Volatile pointer increment
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_array_index_and_pointer_mix(int *arr, volatile int n) {
    int sum = 0;
    int *ptr = arr;
    
    // Mix array indexing and pointer arithmetic
    for (int i = 0; i < n; i++) {
        // Array indexing form
        sum += arr[i];
        
        // Pointer arithmetic form in same loop
        sum += *ptr;
        ptr++;
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
void test_pointer_fill(int *arr, volatile int value, volatile int n) {
    int *ptr = arr;
    int *end = arr + n;
    
    // Pattern: Write with pointer and increment
    while (ptr < end) {
        *ptr = value;
        ptr++;
    }
    
    COMPILER_BARRIER();
}

int main(int argc, char **argv) {
    // Use argc to make loop bounds non-constant at compile time
    volatile int size = (argc > 1) ? atoi(argv[1]) : 100;
    if (size < 10) size = 100;
    if (size > 10000) size = 10000;
    
    // Allocate arrays of different types
    int *int_array = malloc(size * sizeof(int));
    int *int_array2 = malloc(size * sizeof(int));
    char *char_array = malloc(size * sizeof(char));
    short *short_array = malloc(size * sizeof(short));
    long long *ll_array = malloc(size * sizeof(long long));
    struct Data *struct_array = malloc(size * sizeof(struct Data));
    
    // Initialize arrays
    for (int i = 0; i < size; i++) {
        int_array[i] = i;
        int_array2[i] = size - i;
        char_array[i] = (char)(i % 256);
        short_array[i] = (short)(i % 32768);
        ll_array[i] = i * 100LL;
        struct_array[i].a = i;
        struct_array[i].b = i * 2;
        struct_array[i].c = (short)(i % 1000);
        struct_array[i].d = (char)(i % 128);
    }
    
    int checksum = 0;
    
    // Run all test patterns
    checksum += test_pointer_increment_sum(int_array, size);
    
    test_pointer_copy(int_array, int_array2, size);
    checksum += int_array2[size/2];
    
    checksum += test_mixed_access_sizes(char_array, short_array, 
                                        int_array, ll_array, size);
    
    checksum += test_complex_addressing(int_array, 0, size);
    
    checksum += test_double_pointer_arithmetic(int_array, int_array2, size);
    
    checksum += test_structure_pointer_walk(struct_array, size);
    
    checksum += test_volatile_pointer_arithmetic(int_array, size);
    
    checksum += test_array_index_and_pointer_mix(int_array, size);
    
    test_pointer_fill(int_array2, 42, size);
    checksum += int_array2[size/3];
    
    // Final compiler barrier
    COMPILER_BARRIER();
    
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

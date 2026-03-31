#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Compiler barriers to prevent reordering
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
        ptr++;  // This should trigger auto-inc-dec pattern
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_mixed_index_pointer(int *array, int n) {
    int sum = 0;
    
    // Mixed indexing and pointer arithmetic
    for (int i = 0; i < n; i++) {
        // Array index access
        sum += array[i];
        
        // Pointer arithmetic in same expression
        int *temp_ptr = array + i;
        sum += *(temp_ptr + 1);  // Complex addressing: *(base + offset)
        
        // More complex: *(ptr + constant - constant)
        sum += *(temp_ptr + 2 - 1);
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
void test_pointer_copy(int *dest, int *src, int n) {
    int *d = dest;
    int *s = src;
    int *end = src + n;
    
    // Classic pointer copy pattern
    while (s < end) {
        *d = *s;
        d++;
        s++;  // Two pointers incrementing
    }
    
    COMPILER_BARRIER();
}

__attribute__((noinline))
long long test_multiple_types(char *carr, short *sarr, int *iarr, long long *llarr, int n) {
    long long total = 0;
    
    // Char pointer with increment
    char *cp = carr;
    for (int i = 0; i < n; i++) {
        total += *cp;
        cp++;  // Different size increment
    }
    
    // Short pointer with increment
    short *sp = sarr;
    for (int i = 0; i < n; i++) {
        total += *sp;
        sp++;
    }
    
    // Int pointer with complex addressing
    int *ip = iarr;
    for (int i = 0; i < n; i++) {
        // Use *(ptr + offset) form
        total += *(ip + 0);
        ip += 1;  // Non-unity stride
    }
    
    // Long long pointer
    long long *lp = llarr;
    for (int i = 0; i < n; i++) {
        total += *lp;
        lp++;
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
int test_struct_pointer_walk(struct MixedData *arr, int n) {
    int sum = 0;
    struct MixedData *ptr = arr;
    
    // Walk through array of structures
    for (int i = 0; i < n; i++) {
        // Access multiple members
        sum += ptr->a;
        sum += ptr->b;
        sum += ptr->c;
        sum += ptr->d;
        
        // Increment after all accesses
        ptr++;  // Should trigger pattern recognition
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_volatile_pointers(volatile int *varr, int n) {
    int sum = 0;
    volatile int *vptr = varr;
    
    // Volatile prevents some optimizations but preserves pointer arithmetic
    for (int i = 0; i < n; i++) {
        sum += *vptr;
        vptr++;  // Volatile pointer increment
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_double_pointer_arithmetic(int *arr1, int *arr2, int n) {
    int sum = 0;
    int *p1 = arr1;
    int *p2 = arr2;
    
    // Complex addressing with two pointers
    for (int i = 0; i < n; i++) {
        // p1[p2 - p2] simplifies to p1[0] but requires analysis
        sum += p1[p2 - p2];
        
        // *(p1 + constant) form
        sum += *(p1 + 1);
        
        // Move both pointers
        p1++;
        p2++;
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
void test_fill_with_increment(int *arr, int value, int n) {
    int *ptr = arr;
    int *end = arr + n;
    
    // Write pattern with increment
    while (ptr < end) {
        *ptr = value;
        ptr++;  // Post-increment after write
    }
    
    COMPILER_BARRIER();
}

int main(int argc, char **argv) {
    // Use argc to make loop bounds non-constant
    int n = (argc > 1) ? atoi(argv[1]) : 100;
    if (n < 10) n = 100;
    if (n > 1000) n = 1000;
    
    // Allocate and initialize arrays
    int *int_array = (int*)malloc(n * sizeof(int));
    int *int_array2 = (int*)malloc(n * sizeof(int));
    char *char_array = (char*)malloc(n * sizeof(char));
    short *short_array = (short*)malloc(n * sizeof(short));
    long long *ll_array = (long long*)malloc(n * sizeof(long long));
    volatile int *volatile_array = (volatile int*)malloc(n * sizeof(int));
    struct MixedData *struct_array = (struct MixedData*)malloc(n * sizeof(struct MixedData));
    
    // Initialize with pattern
    for (int i = 0; i < n; i++) {
        int_array[i] = i * 3 + 1;
        int_array2[i] = i * 2;
        char_array[i] = (char)(i % 256);
        short_array[i] = (short)(i * 5);
        ll_array[i] = i * 7LL;
        volatile_array[i] = i * 11;
        struct_array[i].a = i;
        struct_array[i].b = (char)(i % 128);
        struct_array[i].c = (short)(i * 3);
        struct_array[i].d = i * 2;
    }
    
    long long total = 0;
    
    // Run all test patterns
    total += test_pointer_increment_sum(int_array, n);
    total += test_mixed_index_pointer(int_array, n);
    
    test_pointer_copy(int_array2, int_array, n);
    total += int_array2[n/2];  // Use copied value
    
    total += test_multiple_types(char_array, short_array, int_array, ll_array, n);
    total += test_struct_pointer_walk(struct_array, n);
    total += test_volatile_pointers(volatile_array, n);
    total += test_double_pointer_arithmetic(int_array, int_array2, n);
    
    test_fill_with_increment(int_array, 42, n);
    total += int_array[n/3];  // Use filled value
    
    // Cleanup
    free(int_array);
    free(int_array2);
    free(char_array);
    free(short_array);
    free(ll_array);
    free((void*)volatile_array);
    free(struct_array);
    
    printf("Result: %lld\n", total);
    return (int)(total % 256);
}

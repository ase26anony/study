#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Compiler barriers to prevent reordering
#define COMPILER_BARRIER() asm volatile("" : : : "memory")

// Test functions marked noinline to preserve boundaries
__attribute__((noinline))
int test_pointer_increment_sum(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    int *end = arr + n;
    
    // Simple pointer increment pattern
    while (ptr < end) {
        sum += *ptr;
        ptr++;  // This should trigger auto-inc pattern
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_mixed_index_pointer(int *arr, int n) {
    int sum = 0;
    
    // Mixed index and pointer forms to create complex addressing
    for (int i = 0; i < n; i++) {
        // Use both array indexing and pointer arithmetic
        sum += arr[i];
        
        // Create pointer arithmetic that might be analyzed
        int *temp = arr + i;
        sum += *(temp + 0);  // *(ptr + constant) form
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
void test_copy_with_pointers(int *dst, int *src, int n) {
    int *d = dst;
    int *s = src;
    int *end = src + n;
    
    // Classic copy pattern that should trigger auto-inc
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
    
    // Different pointer types with increments
    char *cp = carr;
    short *sp = sarr;
    int *ip = iarr;
    long long *llp = llarr;
    
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
int test_complex_addressing(int *arr1, int *arr2, int n) {
    int sum = 0;
    
    // Complex addressing with multiple pointers
    volatile int *volatile_ptr1 = arr1;  // Volatile to prevent simplification
    volatile int *volatile_ptr2 = arr2;
    
    for (int i = 0; i < n; i++) {
        // Create complex address expression: *(p1 + p2 - p2)
        // This may simplify but requires analysis
        int *p1 = (int *)volatile_ptr1;
        int *p2 = (int *)volatile_ptr2;
        
        // Force address calculation that needs analysis
        sum += *(p1 + (p2 - p2));  // Should simplify to *p1
        
        // Increment pointers
        volatile_ptr1 = p1 + 1;
        volatile_ptr2 = p2 + 1;
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_structure_access(struct Data {
    int a;
    int b;
    char c;
    short d;
} *data, int n) {
    int total = 0;
    struct Data *ptr = data;
    
    // Structure access with pointer walking
    for (int i = 0; i < n; i++) {
        total += ptr->a;
        total += ptr->b;
        total += ptr->c;
        total += ptr->d;
        
        ptr++;  // Structure pointer increment
    }
    
    COMPILER_BARRIER();
    return total;
}

__attribute__((noinline))
int test_pointer_with_offset(int *arr, int n, int offset) {
    int sum = 0;
    int *base = arr;
    
    // Pointer with offset that gets incremented
    for (int i = 0; i < n; i++) {
        // Access with offset: *(base + offset)
        sum += *(base + offset);
        
        // Also access with array notation
        sum += base[offset];
        
        // Increment base pointer
        base++;
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
void test_fill_with_increment(int *arr, int value, int n) {
    int *ptr = arr;
    int *end = arr + n;
    
    // Fill pattern with write and increment
    while (ptr < end) {
        *ptr = value;
        ptr++;  // Write with pointer increment
    }
    
    COMPILER_BARRIER();
}

int main(int argc, char **argv) {
    // Use argc to make loop bounds non-constant
    int size = (argc > 1) ? atoi(argv[1]) : 100;
    if (size < 10) size = 100;
    
    // Allocate arrays of different types
    int *int_arr1 = malloc(size * sizeof(int));
    int *int_arr2 = malloc(size * sizeof(int));
    char *char_arr = malloc(size * sizeof(char));
    short *short_arr = malloc(size * sizeof(short));
    long long *ll_arr = malloc(size * sizeof(long long));
    struct Data *struct_arr = malloc(size * sizeof(struct Data));
    
    // Initialize arrays
    for (int i = 0; i < size; i++) {
        int_arr1[i] = i;
        int_arr2[i] = size - i;
        char_arr[i] = (char)(i % 256);
        short_arr[i] = (short)(i % 65536);
        ll_arr[i] = i * 100LL;
        struct_arr[i].a = i;
        struct_arr[i].b = i * 2;
        struct_arr[i].c = (char)(i % 256);
        struct_arr[i].d = (short)(i % 65536);
    }
    
    int checksum = 0;
    
    // Run all test patterns
    checksum += test_pointer_increment_sum(int_arr1, size);
    checksum += test_mixed_index_pointer(int_arr1, size);
    
    test_copy_with_pointers(int_arr2, int_arr1, size);
    checksum += int_arr2[size/2];  // Sample value
    
    checksum += test_multiple_types(char_arr, short_arr, int_arr1, ll_arr, size);
    checksum += test_complex_addressing(int_arr1, int_arr2, size);
    checksum += test_structure_access(struct_arr, size);
    checksum += test_pointer_with_offset(int_arr1, size, 3);
    
    test_fill_with_increment(int_arr2, 0xABCD, size);
    checksum += int_arr2[size/3];  // Sample value
    
    // Final compiler barrier
    COMPILER_BARRIER();
    
    printf("Checksum: %d\n", checksum);
    
    // Cleanup
    free(int_arr1);
    free(int_arr2);
    free(char_arr);
    free(short_arr);
    free(ll_arr);
    free(struct_arr);
    
    return 0;
}

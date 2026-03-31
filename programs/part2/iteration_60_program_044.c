#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Compiler barriers to prevent unwanted optimizations
#define COMPILER_BARRIER() asm volatile("" : : : "memory")

// Test functions marked noinline to preserve boundaries
__attribute__((noinline))
int test_pointer_increment_sum(int *arr, volatile int n) {
    int sum = 0;
    int *ptr = arr;
    int *end = arr + n;
    
    // Pattern: *ptr followed by ptr++ in same iteration
    while (ptr < end) {
        sum += *ptr;  // Memory access via pointer
        ptr++;        // Increment immediately after
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
void test_pointer_copy(int *dst, int *src, volatile int n) {
    int *d = dst;
    int *s = src;
    int *end = src + n;
    
    // Classic *dst++ = *src++ pattern
    while (s < end) {
        *d = *s;  // Two memory accesses with pointer arithmetic
        d++;
        s++;
    }
    
    COMPILER_BARRIER();
}

__attribute__((noinline))
long long test_mixed_sizes(char *carr, short *sarr, int *iarr, long long *llarr, volatile int n) {
    long long total = 0;
    
    // Process char array with pointer
    char *cp = carr;
    for (int i = 0; i < n; i++) {
        total += *cp;
        cp++;  // Char pointer increment
    }
    
    // Process short array with pointer + offset
    short *sp = sarr;
    for (int i = 0; i < n; i++) {
        total += *(sp + 0);  // Access with offset 0
        sp += 1;  // Increment by stride
    }
    
    // Process int array with complex addressing
    int *ip = iarr;
    volatile int *vip = iarr;  // Volatile pointer to prevent simplification
    for (int i = 0; i < n; i++) {
        // Create complex address expression: *(ip + (vip - vip))
        total += *(ip + (*vip - *vip));
        ip++;
    }
    
    // Process long long array with pointer arithmetic
    long long *llp = llarr;
    for (int i = 0; i < n; i++) {
        total += *llp;
        llp += 1;  // Different stride
    }
    
    COMPILER_BARRIER();
    return total;
}

__attribute__((noinline))
int test_structure_access(volatile int n) {
    struct Data {
        int a;
        int b;
        short c;
        char d;
    };
    
    struct Data *arr = malloc(n * sizeof(struct Data));
    if (!arr) return 0;
    
    // Initialize
    for (int i = 0; i < n; i++) {
        arr[i].a = i;
        arr[i].b = i * 2;
        arr[i].c = i * 3;
        arr[i].d = i * 4;
    }
    
    // Access structure members via pointer with increment
    struct Data *ptr = arr;
    int sum = 0;
    for (int i = 0; i < n; i++) {
        // Multiple member accesses followed by pointer increment
        sum += ptr->a;
        sum += ptr->b;
        sum += ptr->c;
        sum += ptr->d;
        ptr++;  // Increment after all accesses
    }
    
    free(arr);
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_dual_pointer_arithmetic(int *arr1, int *arr2, volatile int n) {
    int sum = 0;
    int *p1 = arr1;
    int *p2 = arr2;
    
    // Use two pointers with arithmetic that could create base+index addressing
    for (int i = 0; i < n; i++) {
        // Complex addressing: p1[p2 - p2] simplifies to p1[0] but requires analysis
        sum += p1[p2 - p2];
        
        // Also access via pointer arithmetic
        sum += *(p2 + 0);
        
        // Increment both pointers
        p1++;
        p2++;
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
void test_pointer_write_fill(int *arr, volatile int value, volatile int n) {
    int *ptr = arr;
    int *end = arr + n;
    
    // Write pattern with pointer increment
    while (ptr < end) {
        *ptr = value;  // Memory write
        ptr++;         // Increment after write
    }
    
    COMPILER_BARRIER();
}

__attribute__((noinline))
int test_mixed_index_and_pointer(int *arr, volatile int n) {
    int sum = 0;
    
    // Mix array indexing and pointer arithmetic
    for (int i = 0; i < n; i++) {
        // Array index access
        sum += arr[i];
        
        // Convert to pointer and use in same loop
        int *p = &arr[i];
        sum += *p;
        
        // Additional pointer arithmetic
        if (i + 1 < n) {
            sum += *(p + 1);  // Offset access
        }
    }
    
    COMPILER_BARRIER();
    return sum;
}

int main(int argc, char **argv) {
    // Use argc to make loop bounds non-constant at compile time
    volatile int size = (argc > 1) ? atoi(argv[1]) : 100;
    if (size <= 0) size = 100;
    if (size > 10000) size = 10000;  // Cap for safety
    
    // Allocate arrays of different types
    int *int_arr1 = malloc(size * sizeof(int));
    int *int_arr2 = malloc(size * sizeof(int));
    char *char_arr = malloc(size * sizeof(char));
    short *short_arr = malloc(size * sizeof(short));
    long long *llong_arr = malloc(size * sizeof(long long));
    
    if (!int_arr1 || !int_arr2 || !char_arr || !short_arr || !llong_arr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    // Initialize arrays
    for (int i = 0; i < size; i++) {
        int_arr1[i] = i;
        int_arr2[i] = size - i;
        char_arr[i] = (char)(i % 256);
        short_arr[i] = (short)(i * 2);
        llong_arr[i] = (long long)i * i;
    }
    
    int checksum = 0;
    
    // Run all test patterns
    checksum += test_pointer_increment_sum(int_arr1, size);
    
    test_pointer_copy(int_arr2, int_arr1, size);
    checksum += int_arr2[size / 2];  // Sample value
    
    checksum += test_mixed_sizes(char_arr, short_arr, int_arr1, llong_arr, size) % 1000000;
    
    checksum += test_structure_access(size);
    
    checksum += test_dual_pointer_arithmetic(int_arr1, int_arr2, size);
    
    test_pointer_write_fill(int_arr1, 42, size);
    checksum += int_arr1[size / 3];  // Sample value
    
    checksum += test_mixed_index_and_pointer(int_arr2, size);
    
    // Clean up
    free(int_arr1);
    free(int_arr2);
    free(char_arr);
    free(short_arr);
    free(llong_arr);
    
    printf("Final checksum: %d\n", checksum);
    return 0;
}

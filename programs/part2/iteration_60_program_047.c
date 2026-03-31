#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Compiler barrier to prevent reordering while keeping patterns intact
#define COMPILER_BARRIER() asm volatile("" : : : "memory")

// Test functions marked noinline to preserve function boundaries
__attribute__((noinline))
int test_pointer_increment_sum(int *array, int n) {
    int sum = 0;
    int *ptr = array;
    int *end = array + n;
    
    // Pattern: *ptr followed by ptr++ - should trigger auto-inc recognition
    while (ptr < end) {
        sum += *ptr;  // Memory access via pointer
        ptr++;        // Increment immediately after
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
void test_pointer_copy(int *dst, int *src, int n) {
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
long long test_mixed_types(char *carr, short *sarr, int *iarr, long long *llarr, int n) {
    long long total = 0;
    
    // Different pointer types with increment patterns
    char *cp = carr;
    for (int i = 0; i < n; i++) {
        total += *cp;
        cp++;
    }
    
    short *sp = sarr;
    for (int i = 0; i < n; i++) {
        total += *sp;
        sp++;
    }
    
    int *ip = iarr;
    for (int i = 0; i < n; i++) {
        total += *ip;
        ip++;
    }
    
    long long *llp = llarr;
    for (int i = 0; i < n; i++) {
        total += *llp;
        llp++;
    }
    
    COMPILER_BARRIER();
    return total;
}

__attribute__((noinline))
int test_complex_addressing(int *array, int n) {
    int sum = 0;
    volatile int *volatile_ptr = array;  // Volatile to prevent simplification
    
    // Complex addressing: *(ptr + offset) with ptr increment
    int *ptr = (int*)volatile_ptr;
    for (int i = 0; i < n; i++) {
        // Multiple base register-like expressions
        sum += *(ptr + 0);  // XEXP(x, 0) should extract 'ptr'
        ptr += 1;           // Increment after access
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_structure_access(void *data, int n) {
    struct Data {
        int a;
        int b;
        int c;
    };
    
    struct Data *ptr = (struct Data*)data;
    int sum = 0;
    
    // Structure access with pointer walking
    for (int i = 0; i < n; i++) {
        sum += ptr->a + ptr->b + ptr->c;  // Multiple accesses to same base
        ptr++;  // Increment after last access
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_mixed_index_pointer(int *array, int n) {
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
int test_double_pointer_arithmetic(int *array1, int *array2, int n) {
    int sum = 0;
    int *p1 = array1;
    int *p2 = array2;
    
    // Two pointers with arithmetic that could be seen as base-plus-index
    for (int i = 0; i < n; i++) {
        // Expression that might simplify but requires analysis
        sum += *(p1 + (p2 - p2));  // p2 - p2 = 0, but compiler needs to analyze
        p1++;
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
void test_fill_with_increment(int *array, int value, int n) {
    int *ptr = array;
    int *end = array + n;
    
    // Write pattern with increment
    while (ptr < end) {
        *ptr = value;  // Memory write
        ptr++;         // Increment after write
    }
    
    COMPILER_BARRIER();
}

int main(int argc, char **argv) {
    // Use argc to make loop bounds non-constant at compile time
    int n = (argc > 1) ? atoi(argv[1]) : 100;
    if (n < 10) n = 10;
    if (n > 1000) n = 1000;
    
    // Allocate arrays of different types
    int *int_array1 = (int*)malloc(n * sizeof(int));
    int *int_array2 = (int*)malloc(n * sizeof(int));
    char *char_array = (char*)malloc(n * sizeof(char));
    short *short_array = (short*)malloc(n * sizeof(short));
    long long *ll_array = (long long*)malloc(n * sizeof(long long));
    void *struct_data = malloc(n * 3 * sizeof(int));  // For structure test
    
    // Initialize with volatile to prevent compile-time computation
    volatile int seed = 42;
    for (int i = 0; i < n; i++) {
        int_array1[i] = seed + i;
        int_array2[i] = seed - i;
        char_array[i] = (char)(seed + i);
        short_array[i] = (short)(seed + i);
        ll_array[i] = seed + i;
        
        // Initialize structure data
        int *struct_ptr = (int*)struct_data;
        struct_ptr[i*3 + 0] = seed + i;
        struct_ptr[i*3 + 1] = seed + i * 2;
        struct_ptr[i*3 + 2] = seed + i * 3;
    }
    
    int result = 0;
    
    // Run all test patterns
    result += test_pointer_increment_sum(int_array1, n);
    
    test_pointer_copy(int_array2, int_array1, n);
    result += test_pointer_increment_sum(int_array2, n);
    
    result += test_mixed_types(char_array, short_array, int_array1, ll_array, n);
    
    result += test_complex_addressing(int_array1, n);
    
    result += test_structure_access(struct_data, n);
    
    result += test_mixed_index_pointer(int_array1, n);
    
    result += test_double_pointer_arithmetic(int_array1, int_array2, n);
    
    test_fill_with_increment(int_array1, 0x55, n);
    result += test_pointer_increment_sum(int_array1, n);
    
    // Final compiler barrier
    COMPILER_BARRIER();
    
    printf("Result: %d\n", result);
    
    // Cleanup
    free(int_array1);
    free(int_array2);
    free(char_array);
    free(short_array);
    free(ll_array);
    free(struct_data);
    
    return 0;
}

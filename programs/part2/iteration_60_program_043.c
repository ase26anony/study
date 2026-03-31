#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Compiler barrier to prevent reordering
#define COMPILER_BARRIER() asm volatile("" : : : "memory")

// Test functions marked noinline to preserve boundaries
__attribute__((noinline))
static int test_char_pointer_increment(volatile char *data, int n) {
    char *ptr = (char *)data;
    int sum = 0;
    
    // Simple pointer increment pattern
    for (int i = 0; i < n; i++) {
        sum += *ptr;
        ptr++;  // This should trigger auto-inc pattern
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
static int test_short_pointer_with_offset(volatile short *data, int n) {
    short *ptr = (short *)data;
    int sum = 0;
    
    // Access with offset then increment
    for (int i = 0; i < n; i++) {
        // Complex addressing: *(ptr + 0) then ptr++
        sum += *(ptr + 0);
        ptr += 1;  // Increment by element size
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
static int test_int_mixed_index_pointer(volatile int *data, int n) {
    int *ptr = (int *)data;
    int sum = 0;
    
    // Mixed forms: array indexing and pointer arithmetic
    for (int i = 0; i < n; i++) {
        // This creates XEXP(x, 0) analysis opportunity
        sum += ptr[0];  // Array indexing form
        ptr = ptr + 1;  // Pointer arithmetic form
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
static long long test_longlong_pointer_chain(volatile long long *data, int n) {
    long long *ptr = (long long *)data;
    long long sum = 0;
    
    // Chain of accesses with pointer increment
    for (int i = 0; i < n; i++) {
        sum += *ptr;
        // Force address computation
        ptr = (long long *)((char *)ptr + sizeof(long long));
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
static int test_two_pointer_copy(volatile int *src, volatile int *dst, int n) {
    int *s = (int *)src;
    int *d = (int *)dst;
    
    // Classic copy pattern: *dst++ = *src++
    for (int i = 0; i < n; i++) {
        *d = *s;
        d++;
        s++;
    }
    
    COMPILER_BARRIER();
    return n;
}

__attribute__((noinline))
static int test_pointer_arithmetic_complex(volatile int *data1, volatile int *data2, int n) {
    int *p1 = (int *)data1;
    int *p2 = (int *)data2;
    int sum = 0;
    
    // Complex addressing that might simplify to base+0
    for (int i = 0; i < n; i++) {
        // p1[p2 - p2] simplifies to p1[0] but requires analysis
        int idx = (int)(p2 - p2);  // Should be 0
        sum += p1[idx];
        p1++;
        
        // Keep p2 moving too to prevent complete optimization
        p2 = p2 + (i & 0);  // Always adds 0, but not trivially visible
    }
    
    COMPILER_BARRIER();
    return sum;
}

struct MixedData {
    int a;
    short b;
    char c;
    long long d;
};

__attribute__((noinline))
static long long test_struct_pointer_walk(volatile struct MixedData *data, int n) {
    struct MixedData *ptr = (struct MixedData *)data;
    long long sum = 0;
    
    // Walk through struct array, accessing multiple members
    for (int i = 0; i < n; i++) {
        sum += ptr->a;
        sum += ptr->b;
        sum += ptr->c;
        sum += ptr->d;
        
        // Increment after all accesses
        ptr++;
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
static int test_volatile_pointer_aliasing(volatile int *data, int n) {
    volatile int *vptr = data;
    int *regptr = (int *)data;
    int sum = 0;
    
    // Mix volatile and regular pointers
    for (int i = 0; i < n; i++) {
        // Access through volatile pointer
        int val = *vptr;
        
        // Same access through regular pointer
        sum += regptr[0];
        
        // Increment both
        vptr = vptr + 1;
        regptr++;
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
static int test_nested_pointer_ops(volatile int **data, int n) {
    int sum = 0;
    volatile int **ptr = data;
    
    // Pointer to pointer access
    for (int i = 0; i < n; i++) {
        if (*ptr) {
            sum += **ptr;
        }
        ptr++;
    }
    
    COMPILER_BARRIER();
    return sum;
}

int main(int argc, char **argv) {
    // Use argc to make loop bounds non-constant
    int base_size = (argc > 1) ? atoi(argv[1]) : 100;
    if (base_size < 10) base_size = 100;
    
    // Allocate arrays of different types
    char *char_array = malloc(base_size * sizeof(char));
    short *short_array = malloc(base_size * sizeof(short));
    int *int_array = malloc(base_size * sizeof(int));
    long long *ll_array = malloc(base_size * sizeof(long long));
    struct MixedData *struct_array = malloc(base_size * sizeof(struct MixedData));
    int *src_array = malloc(base_size * sizeof(int));
    int *dst_array = malloc(base_size * sizeof(int));
    volatile int **ptr_array = malloc(base_size * sizeof(int *));
    
    // Initialize data
    for (int i = 0; i < base_size; i++) {
        char_array[i] = (char)(i % 128);
        short_array[i] = (short)(i * 2);
        int_array[i] = i * 3;
        ll_array[i] = i * 100LL;
        struct_array[i].a = i;
        struct_array[i].b = (short)(i * 2);
        struct_array[i].c = (char)(i % 64);
        struct_array[i].d = i * 50LL;
        src_array[i] = i * 7;
        dst_array[i] = 0;
        ptr_array[i] = (volatile int *)&src_array[i];
    }
    
    int result = 0;
    
    // Run all test patterns
    result += test_char_pointer_increment((volatile char *)char_array, base_size);
    result += test_short_pointer_with_offset((volatile short *)short_array, base_size);
    result += test_int_mixed_index_pointer((volatile int *)int_array, base_size);
    result += test_longlong_pointer_chain((volatile long long *)ll_array, base_size);
    result += test_two_pointer_copy((volatile int *)src_array, (volatile int *)dst_array, base_size);
    result += test_pointer_arithmetic_complex((volatile int *)int_array, (volatile int *)src_array, base_size);
    result += test_struct_pointer_walk((volatile struct MixedData *)struct_array, base_size);
    result += test_volatile_pointer_aliasing((volatile int *)int_array, base_size);
    result += test_nested_pointer_ops(ptr_array, base_size);
    
    // Verify copy worked
    for (int i = 0; i < base_size; i++) {
        if (dst_array[i] != src_array[i]) {
            result += 1;
        }
    }
    
    printf("Result: %d\n", result);
    
    // Cleanup
    free(char_array);
    free(short_array);
    free(int_array);
    free(ll_array);
    free(struct_array);
    free(src_array);
    free(dst_array);
    free(ptr_array);
    
    return 0;
}

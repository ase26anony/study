/* auto_inc_dec_test.c
 * Test program to trigger GCC's auto-increment/decrement pattern recognition
 * Compile with: gcc -O2 -fno-unroll-loops -fno-tree-vectorize -c auto_inc_dec_test.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prevent inlining to preserve function boundaries */
#define NOINLINE __attribute__((noinline))

/* Structure for pointer walking tests */
struct MixedData {
    char c;
    int i;
    short s;
    long long ll;
    char pad[7];
};

/* Volatile variables to prevent compile-time optimization */
static volatile int g_volatile_size = 100;
static volatile long long g_seed = 123456789LL;

/* Test 1: Basic pointer increment in loop - should trigger simple pattern */
NOINLINE static long long test_basic_pointer_increment(int *array, int size) {
    volatile int *volatile_ptr = array; /* Volatile pointer to prevent simplification */
    int *ptr = (int *)volatile_ptr;
    long long sum = 0;
    
    /* Basic pattern: *ptr then ptr++ */
    for (int i = 0; i < size; i++) {
        sum += *ptr;
        ptr++;  /* Increment immediately after access */
    }
    
    /* Compiler barrier to prevent reordering but keep pattern intact */
    asm volatile("" : : : "memory");
    return sum;
}

/* Test 2: Mixed array indexing and pointer arithmetic */
NOINLINE static long long test_mixed_index_pointer(int *array, int size) {
    long long sum = 0;
    int *ptr = array;
    
    /* Mix array indexing and pointer arithmetic */
    for (int i = 0; i < size; i++) {
        /* Access via pointer with offset */
        sum += *(ptr + 0);
        
        /* Also use array indexing on same pointer */
        if (i % 3 == 0) {
            sum += ptr[0];  /* Same as *(ptr + 0) but different syntax */
        }
        
        /* Increment pointer - adjacent to accesses */
        ptr++;
    }
    
    return sum;
}

/* Test 3: Structure access with pointer walking */
NOINLINE static long long test_struct_pointer_walk(struct MixedData *data, int size) {
    struct MixedData *ptr = data;
    long long sum = 0;
    
    /* Access multiple structure members then increment pointer */
    for (int i = 0; i < size; i++) {
        sum += ptr->c + ptr->i + ptr->s;
        sum += ptr->ll;  /* Last access before increment */
        
        /* Pointer increment adjacent to last member access */
        ptr++;
    }
    
    asm volatile("" : : : "memory");
    return sum;
}

/* Test 4: Copy between arrays using dual pointers */
NOINLINE static void test_dual_pointer_copy(int *src, int *dst, int size) {
    volatile int *volatile_src = src;
    volatile int *volatile_dst = dst;
    int *s = (int *)volatile_src;
    int *d = (int *)volatile_dst;
    
    /* Classic *dst++ = *src++ pattern */
    for (int i = 0; i < size; i++) {
        *d = *s;
        d++;  /* Increment after write */
        s++;  /* Increment after read */
    }
}

/* Test 5: Complex addressing with multiple base registers */
NOINLINE static long long test_complex_addressing(int *array1, int *array2, int size) {
    long long sum = 0;
    int *p1 = array1;
    int *p2 = array2;
    
    /* Create complex addressing that might simplify to base+0 */
    for (int i = 0; i < size; i++) {
        /* Access via p1 with offset derived from p2-p2 (should be 0) */
        sum += *(p1 + (p2 - p2));  /* Should become *(p1 + 0) */
        
        /* Another complex expression */
        sum += p1[(p2 - array2) * 0];  /* Should become p1[0] */
        
        /* Increment both pointers */
        p1++;
        p2++;
    }
    
    return sum;
}

/* Test 6: Different memory access sizes */
NOINLINE static long long test_mixed_sizes(char *carr, short *sarr, int *iarr, long long *llarr, int size) {
    long long sum = 0;
    char *cp = carr;
    short *sp = sarr;
    int *ip = iarr;
    long long *llp = llarr;
    
    /* Process each array with pointer increment */
    for (int i = 0; i < size; i++) {
        sum += *cp;
        cp++;
    }
    
    for (int i = 0; i < size; i++) {
        sum += *sp;
        sp++;
    }
    
    for (int i = 0; i < size; i++) {
        sum += *ip;
        ip++;
    }
    
    for (int i = 0; i < size; i++) {
        sum += *llp;
        llp++;
    }
    
    asm volatile("" : : : "memory");
    return sum;
}

/* Test 7: Pointer arithmetic with stride */
NOINLINE static long long test_pointer_stride(int *array, int size, int stride) {
    long long sum = 0;
    int *ptr = array;
    
    /* Access with stride using pointer arithmetic */
    for (int i = 0; i < size; i++) {
        sum += *ptr;
        ptr += stride;  /* Variable stride increment */
    }
    
    return sum;
}

/* Test 8: Fill array with pointer write */
NOINLINE static void test_fill_array(int *array, int size, int value) {
    int *ptr = array;
    
    /* Write then increment pattern */
    for (int i = 0; i < size; i++) {
        *ptr = value;
        ptr++;  /* Increment after write */
    }
}

/* Main driver that calls all tests */
int main(int argc, char **argv) {
    /* Use argc to make sizes non-constant at compile time */
    int base_size = g_volatile_size;
    if (argc > 1) {
        base_size = atoi(argv[1]) % 100 + 50;
    }
    
    const int size = base_size;
    long long total_sum = 0;
    
    /* Allocate and initialize arrays of different types */
    int *int_array1 = (int *)malloc(size * sizeof(int));
    int *int_array2 = (int *)malloc(size * sizeof(int));
    char *char_array = (char *)malloc(size * sizeof(char));
    short *short_array = (short *)malloc(size * sizeof(short));
    long long *ll_array = (long long *)malloc(size * sizeof(long long));
    struct MixedData *struct_array = (struct MixedData *)malloc(size * sizeof(struct MixedData));
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < size; i++) {
        int_array1[i] = (i * 37 + 123) % 1000;
        int_array2[i] = (i * 73 + 456) % 1000;
        char_array[i] = (char)(i % 256);
        short_array[i] = (short)(i * 3);
        ll_array[i] = (long long)i * 1000000LL;
        
        struct_array[i].c = (char)(i % 128);
        struct_array[i].i = i * 7;
        struct_array[i].s = (short)(i * 11);
        struct_array[i].ll = (long long)i * 1000LL;
    }
    
    /* Run all tests */
    total_sum += test_basic_pointer_increment(int_array1, size);
    total_sum += test_mixed_index_pointer(int_array2, size);
    total_sum += test_struct_pointer_walk(struct_array, size);
    
    test_dual_pointer_copy(int_array1, int_array2, size);
    total_sum += test_complex_addressing(int_array1, int_array2, size);
    
    total_sum += test_mixed_sizes(char_array, short_array, int_array1, ll_array, size);
    total_sum += test_pointer_stride(int_array1, size, 1);
    
    test_fill_array(int_array1, size, 42);
    
    /* Verify some results */
    int verify_sum = 0;
    for (int i = 0; i < size; i++) {
        verify_sum += int_array1[i];
    }
    
    printf("Total checksum: %lld\n", total_sum);
    printf("Array fill verify sum: %d (expected %d)\n", verify_sum, 42 * size);
    
    /* Cleanup */
    free(int_array1);
    free(int_array2);
    free(char_array);
    free(short_array);
    free(ll_array);
    free(struct_array);
    
    return 0;
}

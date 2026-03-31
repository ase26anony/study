/* auto_inc_dec_test.c
 * Designed to trigger GCC's auto-increment/decrement pattern recognition
 * Specifically targets lines 1352-1358 in auto-inc-dec.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Compiler barriers to prevent unwanted optimizations */
#define COMPILER_BARRIER() asm volatile("" : : : "memory")

/* Test functions marked noinline to preserve boundaries */
__attribute__((noinline, noipa))
static int test_char_pointer_loop(volatile char *data, int n) {
    char *ptr = (char *)data;
    int sum = 0;
    
    /* Simple pointer increment pattern */
    for (int i = 0; i < n; i++) {
        sum += *ptr;
        ptr++;  /* Adjacent increment - should trigger pattern */
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline, noipa))
static int test_short_pointer_offset(volatile short *data, int n) {
    short *ptr = (short *)data;
    int sum = 0;
    
    /* Access with offset then increment */
    for (int i = 0; i < n; i++) {
        /* Complex addressing: *(ptr + 0) then ptr++ */
        sum += *(ptr + 0);  /* XEXP(x, 0) should extract ptr */
        ptr += 1;  /* reg1_is_const = true, reg1_val = sizeof(short) */
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline, noipa))
static int test_int_mixed_index_pointer(volatile int *data, int n) {
    int *ptr = (int *)data;
    int sum = 0;
    
    /* Mixed indexing and pointer arithmetic */
    for (int i = 0; i < n; i++) {
        /* Both forms in proximity */
        sum += ptr[0];      /* Array indexing */
        sum += *(ptr + 0);  /* Pointer arithmetic */
        ptr++;              /* Adjacent increment */
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline, noipa))
static long long test_longlong_copy(volatile long long *src, 
                                   volatile long long *dst, int n) {
    long long *s = (long long *)src;
    long long *d = (long long *)dst;
    long long checksum = 0;
    
    /* Classic copy pattern with two moving pointers */
    for (int i = 0; i < n; i++) {
        *d = *s;    /* Memory access */
        checksum += *d;
        s++;        /* Both pointers increment */
        d++;        /* Should trigger dual auto-inc pattern */
    }
    
    COMPILER_BARRIER();
    return checksum;
}

/* Structure for pointer walking test */
struct MixedData {
    int a;
    char b;
    short c;
    int d;
};

__attribute__((noinline, noipa))
static int test_struct_pointer_walk(volatile struct MixedData *data, int n) {
    struct MixedData *ptr = (struct MixedData *)data;
    int sum = 0;
    
    /* Walk through struct array with pointer */
    for (int i = 0; i < n; i++) {
        /* Multiple member accesses */
        sum += ptr->a;
        sum += ptr->b;
        sum += ptr->c;
        sum += ptr->d;
        ptr++;  /* Increment after last access */
    }
    
    COMPILER_BARRIER();
    return sum;
}

/* Complex addressing with multiple base registers */
__attribute__((noinline, noipa))
static int test_complex_addressing(volatile int *base1, 
                                  volatile int *base2, int n) {
    int *p1 = (int *)base1;
    int *p2 = (int *)base2;
    int sum = 0;
    
    /* Potentially confusing address pattern */
    for (int i = 0; i < n; i++) {
        /* Address that might require analysis: *(p1 + (p2 - p2)) */
        sum += *(p1 + (p2 - p2));  /* Should simplify to *p1 */
        p1++;  /* But p1 is incremented */
        
        /* Keep p2 moving too to prevent complete optimization */
        sum += *p2;
        p2++;
    }
    
    COMPILER_BARRIER();
    return sum;
}

/* Test with volatile pointers to prevent constant folding */
__attribute__((noinline, noipa))
static int test_volatile_pointer_arithmetic(int n) {
    static int array[1024];
    volatile int *vptr = array;
    int *ptr = (int *)vptr;  /* Cast away volatile for arithmetic */
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < n; i++) {
        array[i] = i;
    }
    
    COMPILER_BARRIER();
    
    /* Access through pointer with increment */
    for (int i = 0; i < n; i++) {
        sum += *ptr;
        ptr++;  /* Pointer increment adjacent to access */
    }
    
    COMPILER_BARRIER();
    return sum;
}

/* Main test driver */
int main(int argc, char **argv) {
    /* Use argc to make loop bounds non-constant */
    int n = (argc > 1) ? atoi(argv[1]) : 100;
    if (n > 1000) n = 1000;  /* Safety limit */
    if (n < 10) n = 10;
    
    /* Allocate and initialize test arrays */
    char *char_array = malloc(n * sizeof(char));
    short *short_array = malloc(n * sizeof(short));
    int *int_array = malloc(n * sizeof(int));
    long long *ll_array = malloc(n * sizeof(long long));
    long long *ll_array2 = malloc(n * sizeof(long long));
    struct MixedData *struct_array = malloc(n * sizeof(struct MixedData));
    int *base1_array = malloc(n * sizeof(int));
    int *base2_array = malloc(n * sizeof(int));
    
    /* Initialize with pattern */
    for (int i = 0; i < n; i++) {
        char_array[i] = (char)(i % 256);
        short_array[i] = (short)(i * 2);
        int_array[i] = i * 3;
        ll_array[i] = i * 5LL;
        ll_array2[i] = 0;
        
        struct_array[i].a = i;
        struct_array[i].b = (char)(i % 128);
        struct_array[i].c = (short)(i * 7);
        struct_array[i].d = i * 11;
        
        base1_array[i] = i * 13;
        base2_array[i] = i * 17;
    }
    
    int total = 0;
    
    /* Run all tests */
    total += test_char_pointer_loop((volatile char *)char_array, n);
    total += test_short_pointer_offset((volatile short *)short_array, n);
    total += test_int_mixed_index_pointer((volatile int *)int_array, n);
    
    long long copy_sum = test_longlong_copy((volatile long long *)ll_array,
                                           (volatile long long *)ll_array2, n);
    total += (int)copy_sum;
    
    total += test_struct_pointer_walk((volatile struct MixedData *)struct_array, n);
    total += test_complex_addressing((volatile int *)base1_array,
                                    (volatile int *)base2_array, n);
    total += test_volatile_pointer_arithmetic(n);
    
    /* Cleanup */
    free(char_array);
    free(short_array);
    free(int_array);
    free(ll_array);
    free(ll_array2);
    free(struct_array);
    free(base1_array);
    free(base2_array);
    
    printf("Total checksum: %d\n", total);
    return total != 0 ? 0 : 1;
}

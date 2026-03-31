/* test_auto_inc_dec.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Compiler barriers to prevent reordering */
#define COMPILER_BARRIER() asm volatile("" : : : "memory")

/* Test functions marked noinline to preserve boundaries */
__attribute__((noinline))
int test_char_sum(volatile char *data, int n) {
    char *ptr = (char *)data;
    int sum = 0;
    
    /* Simple pointer increment pattern */
    for (int i = 0; i < n; i++) {
        sum += *ptr;
        ptr++;  /* Should trigger auto-inc pattern */
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_short_sum(volatile short *data, int n) {
    short *ptr = (short *)data;
    int sum = 0;
    
    /* Pointer with constant offset pattern */
    for (int i = 0; i < n; i++) {
        sum += *(ptr + 0);  /* Non-trivial address expression */
        ptr += 1;  /* Increment by stride */
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_int_copy(volatile int *src, volatile int *dst, int n) {
    int *s = (int *)src;
    int *d = (int *)dst;
    int checksum = 0;
    
    /* Classic copy pattern with two moving pointers */
    for (int i = 0; i < n; i++) {
        *d = *s;
        checksum += *d;
        s++;
        d++;
    }
    
    COMPILER_BARRIER();
    return checksum;
}

__attribute__((noinline))
long long test_longlong_fill(volatile long long *data, long long value, int n) {
    long long *ptr = (long long *)data;
    long long sum = 0;
    
    /* Mixed indexing and pointer arithmetic */
    for (int i = 0; i < n; i++) {
        ptr[0] = value + i;  /* Array indexing form */
        sum += ptr[0];
        ptr = ptr + 1;  /* Pointer arithmetic form */
    }
    
    COMPILER_BARRIER();
    return sum;
}

/* Structure for complex pattern */
struct MixedData {
    int a;
    short b;
    char c;
    long long d;
};

__attribute__((noinline))
long long test_struct_walk(volatile struct MixedData *data, int n) {
    struct MixedData *ptr = (struct MixedData *)data;
    long long sum = 0;
    
    /* Walk through array of structures */
    for (int i = 0; i < n; i++) {
        /* Multiple member accesses with same base pointer */
        sum += ptr->a;
        sum += ptr->b;
        sum += ptr->c;
        sum += ptr->d;
        
        /* Increment after all accesses */
        ptr++;
    }
    
    COMPILER_BARRIER();
    return sum;
}

/* Complex addressing with multiple base registers */
__attribute__((noinline))
int test_complex_addressing(volatile int *base1, volatile int *base2, int n) {
    int *p1 = (int *)base1;
    int *p2 = (int *)base2;
    int sum = 0;
    
    /* Create complex address expressions that might simplify */
    for (int i = 0; i < n; i++) {
        /* Expression that could be seen as base-plus-index */
        sum += *(p1 + (p2 - p2));  /* Should simplify to *p1 */
        sum += p1[0];              /* Another form of the same */
        
        /* Increment both pointers differently */
        p1++;
        p2 += 0;  /* p2 doesn't change, but compiler doesn't know */
    }
    
    COMPILER_BARRIER();
    return sum;
}

/* Function with mixed access sizes */
__attribute__((noinline))
int test_mixed_sizes(volatile void *data, int n) {
    char *cptr = (char *)data;
    short *sptr = (short *)(cptr + 256);
    int *iptr = (int *)(sptr + 128);
    long long *llptr = (long long *)(iptr + 64);
    
    int sum = 0;
    
    /* Access different types with pointer arithmetic */
    for (int i = 0; i < n && i < 32; i++) {
        sum += *cptr;
        cptr++;
        
        sum += *sptr;
        sptr++;
        
        sum += *iptr;
        iptr++;
        
        sum += (int)*llptr;
        llptr++;
    }
    
    COMPILER_BARRIER();
    return sum;
}

int main(int argc, char **argv) {
    /* Use argc to make loop bounds non-constant */
    int n = (argc > 1) ? atoi(argv[1]) : 100;
    if (n < 10) n = 10;
    if (n > 1000) n = 1000;
    
    /* Allocate and initialize arrays */
    char *char_array = malloc(n * sizeof(char));
    short *short_array = malloc(n * sizeof(short));
    int *int_array1 = malloc(n * sizeof(int));
    int *int_array2 = malloc(n * sizeof(int));
    long long *ll_array = malloc(n * sizeof(long long));
    struct MixedData *struct_array = malloc(n * sizeof(struct MixedData));
    
    /* Initialize with pattern */
    for (int i = 0; i < n; i++) {
        char_array[i] = (char)(i % 256);
        short_array[i] = (short)(i * 3);
        int_array1[i] = i * 5;
        int_array2[i] = 0;
        ll_array[i] = i * 7LL;
        
        struct_array[i].a = i;
        struct_array[i].b = (short)(i * 2);
        struct_array[i].c = (char)(i % 128);
        struct_array[i].d = i * 11LL;
    }
    
    int total = 0;
    
    /* Run all test patterns */
    total += test_char_sum((volatile char *)char_array, n);
    total += test_short_sum((volatile short *)short_array, n);
    total += test_int_copy((volatile int *)int_array1, 
                          (volatile int *)int_array2, n);
    total += (int)test_longlong_fill((volatile long long *)ll_array, 
                                    42LL, n);
    total += (int)test_struct_walk((volatile struct MixedData *)struct_array, n);
    total += test_complex_addressing((volatile int *)int_array1,
                                    (volatile int *)int_array2, n);
    total += test_mixed_sizes((volatile void *)char_array, n);
    
    /* Verify int_array2 was modified */
    for (int i = 0; i < n; i++) {
        total += int_array2[i];
    }
    
    printf("Total checksum: %d\n", total);
    
    /* Cleanup */
    free(char_array);
    free(short_array);
    free(int_array1);
    free(int_array2);
    free(ll_array);
    free(struct_array);
    
    return 0;
}

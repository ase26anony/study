/* test_auto_inc_dec.c - Target auto-inc-dec.cc lines 1352-1358 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Compiler barriers to prevent reordering */
#define COMPILER_BARRIER() asm volatile("" : : : "memory")

/* Test functions marked noinline to preserve boundaries */
__attribute__((noinline))
int test_pointer_loop_sum(int *array, volatile int n) {
    int sum = 0;
    int *ptr = array;
    int *end = array + n;
    
    /* Simple pointer increment pattern */
    while (ptr < end) {
        sum += *ptr;
        ptr++;  /* Should trigger auto-inc pattern */
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_mixed_index_pointer(int *array, volatile int n) {
    int sum = 0;
    int *ptr = array;
    
    /* Mixed indexing and pointer arithmetic */
    for (int i = 0; i < n; i++) {
        sum += array[i];      /* Array index form */
        sum += *(ptr + i);    /* Pointer + offset form */
        ptr++;                /* Pointer increment - complex addressing */
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
void test_pointer_copy(int *dst, int *src, volatile int n) {
    int *d = dst;
    int *s = src;
    int *end = src + n;
    
    /* Classic pointer copy pattern */
    while (s < end) {
        *d = *s;
        d++;
        s++;  /* Two pointers incrementing */
    }
    
    COMPILER_BARRIER();
}

__attribute__((noinline))
long long test_multiple_types(volatile int n) {
    /* Different pointer types for different access sizes */
    char *cptr;
    short *sptr;
    int *iptr;
    long long *llptr;
    
    /* Allocate and initialize arrays */
    char *carr = malloc(n * sizeof(char));
    short *sarr = malloc(n * sizeof(short));
    int *iarr = malloc(n * sizeof(int));
    long long *llarr = malloc(n * sizeof(long long));
    
    for (int i = 0; i < n; i++) {
        carr[i] = (char)(i % 256);
        sarr[i] = (short)(i * 2);
        iarr[i] = i * 3;
        llarr[i] = i * 5LL;
    }
    
    long long total = 0;
    
    /* Process char array with pointer */
    cptr = carr;
    for (int i = 0; i < n; i++) {
        total += *cptr;
        cptr++;  /* char pointer increment */
    }
    
    /* Process short array with pointer */
    sptr = sarr;
    for (int i = 0; i < n; i++) {
        total += *sptr;
        sptr++;  /* short pointer increment */
    }
    
    /* Process int array with pointer */
    iptr = iarr;
    for (int i = 0; i < n; i++) {
        total += *iptr;
        iptr++;  /* int pointer increment */
    }
    
    /* Process long long array with pointer */
    llptr = llarr;
    for (int i = 0; i < n; i++) {
        total += *llptr;
        llptr++;  /* long long pointer increment */
    }
    
    free(carr);
    free(sarr);
    free(iarr);
    free(llarr);
    
    COMPILER_BARRIER();
    return total;
}

/* Structure for pointer walking */
struct MixedData {
    int a;
    char b;
    short c;
    int d;
};

__attribute__((noinline))
int test_struct_pointer_walk(struct MixedData *data, volatile int n) {
    int sum = 0;
    struct MixedData *ptr = data;
    
    /* Walk through array of structures */
    for (int i = 0; i < n; i++) {
        /* Access multiple members */
        sum += ptr->a;
        sum += ptr->b;
        sum += ptr->c;
        sum += ptr->d;
        
        ptr++;  /* Structure pointer increment - complex addressing */
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_complex_addressing(volatile int *base, volatile int offset, volatile int n) {
    int sum = 0;
    
    /* Complex addressing expression that might simplify */
    for (int i = 0; i < n; i++) {
        /* *(base + offset + i - i) should simplify but require analysis */
        sum += *(base + offset + i - i);
        
        /* More complex: *(base + (offset * 2) / 2) */
        sum += *(base + (offset * 2) / 2);
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_dual_pointer_arithmetic(int *arr1, int *arr2, volatile int n) {
    int sum = 0;
    int *p1 = arr1;
    int *p2 = arr2;
    
    /* Two pointers with arithmetic that could be base+index */
    for (int i = 0; i < n; i++) {
        /* p1[p2 - p2] creates complex addressing */
        sum += p1[p2 - p2];
        
        /* Increment both pointers */
        p1++;
        p2++;
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
void test_volatile_pointers(volatile int *vptr, volatile int n) {
    volatile int *ptr = vptr;
    
    /* Volatile pointer prevents constant folding */
    for (int i = 0; i < n; i++) {
        int val = *ptr;
        (void)val;  /* Use value */
        ptr++;  /* Volatile pointer increment */
    }
    
    COMPILER_BARRIER();
}

__attribute__((noinline))
int test_stride_access(int *array, volatile int stride, volatile int n) {
    int sum = 0;
    int *ptr = array;
    
    /* Pointer with stride (not 1) */
    for (int i = 0; i < n; i++) {
        sum += *ptr;
        ptr += stride;  /* Non-unit stride increment */
    }
    
    COMPILER_BARRIER();
    return sum;
}

/* Main driver */
int main(int argc, char **argv) {
    /* Use argc to make loop bounds non-constant */
    volatile int size = (argc > 1) ? atoi(argv[1]) : 100;
    if (size <= 0) size = 100;
    
    /* Allocate test arrays */
    int *array1 = malloc(size * sizeof(int));
    int *array2 = malloc(size * sizeof(int));
    struct MixedData *struct_array = malloc(size * sizeof(struct MixedData));
    
    /* Initialize arrays */
    for (int i = 0; i < size; i++) {
        array1[i] = i * 2;
        array2[i] = i * 3;
        struct_array[i].a = i;
        struct_array[i].b = (char)(i % 128);
        struct_array[i].c = (short)(i * 2);
        struct_array[i].d = i * 4;
    }
    
    long long total = 0;
    
    /* Run all test patterns */
    total += test_pointer_loop_sum(array1, size);
    
    total += test_mixed_index_pointer(array2, size);
    
    test_pointer_copy(array1, array2, size);
    total += array1[size/2];  /* Use copied value */
    
    total += test_multiple_types(size % 50);
    
    total += test_struct_pointer_walk(struct_array, size);
    
    total += test_complex_addressing(array1, size/4, size/2);
    
    total += test_dual_pointer_arithmetic(array1, array2, size);
    
    test_volatile_pointers(array1, size);
    
    total += test_stride_access(array2, 2, size/2);
    
    /* Clean up */
    free(array1);
    free(array2);
    free(struct_array);
    
    printf("Result: %lld\n", total);
    return 0;
}

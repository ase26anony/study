/* test_auto_inc_dec.c - Target coverage for auto-inc-dec.cc lines 1352-1358 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Compiler barriers to prevent reordering */
#define COMPILER_BARRIER() asm volatile("" : : : "memory")

/* Test functions marked noinline to preserve boundaries */
__attribute__((noinline, noipa))
static int test_pointer_loop_sum(int *array, int n) {
    volatile int *volatile_ptr = array; /* Volatile to prevent constant folding */
    int *ptr = (int *)volatile_ptr;
    int sum = 0;
    
    /* Simple pointer increment pattern */
    for (int i = 0; i < n; i++) {
        sum += *ptr;  /* Memory access via pointer */
        ptr++;        /* Immediate increment - should trigger auto-inc pattern */
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline, noipa))
static long test_mixed_index_pointer(int *array, int n) {
    volatile int *volatile_base = array;
    int *ptr = (int *)volatile_base;
    long sum = 0;
    
    /* Mixed indexing and pointer arithmetic */
    for (int i = 0; i < n; i++) {
        /* Access via pointer with offset - creates complex address expression */
        sum += *(ptr + (i & 1));  /* XEXP(x, 0) should extract ptr */
        
        /* Then increment the base pointer */
        ptr += 1;  /* This increment should be adjacent enough for pattern matching */
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline, noipa))
static long long test_multiple_base_registers(long long *arr1, long long *arr2, int n) {
    volatile long long *volatile_p1 = arr1;
    volatile long long *volatile_p2 = arr2;
    long long *p1 = (long long *)volatile_p1;
    long long *p2 = (long long *)volatile_p2;
    long long sum = 0;
    
    /* Complex addressing that might simplify to base+index */
    for (int i = 0; i < n; i++) {
        /* Expression that could be seen as base-plus-index: p1 + (p2 - p2) */
        sum += p1[p2 - p2 + i];  /* Should simplify to p1[i] but requires analysis */
        
        /* Increment one of the pointers */
        p1++;  /* This should be recognized as auto-increment candidate */
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline, noipa))
static int test_structure_pointer_walk(int n) {
    struct Data {
        int a;
        int b;
        short c;
        char d;
    };
    
    /* Dynamically allocate to prevent stack optimization */
    struct Data *data = (struct Data *)malloc(n * sizeof(struct Data));
    if (!data) return -1;
    
    /* Initialize with pattern */
    for (int i = 0; i < n; i++) {
        data[i].a = i;
        data[i].b = i * 2;
        data[i].c = (short)i;
        data[i].d = (char)(i & 0xFF);
    }
    
    volatile struct Data *volatile_ptr = data;
    struct Data *ptr = (struct Data *)volatile_ptr;
    int sum = 0;
    
    /* Walk through structure array with pointer */
    for (int i = 0; i < n; i++) {
        /* Access multiple members - pointer increment should be after last access */
        sum += ptr->a + ptr->b;
        sum += ptr->c;
        sum += ptr->d;
        
        ptr++;  /* Increment after all accesses - should trigger pattern */
    }
    
    COMPILER_BARRIER();
    free(data);
    return sum;
}

__attribute__((noinline, noipa))
static long test_different_sizes(int n) {
    /* Arrays of different types/sizes */
    char *carr = (char *)malloc(n * 4);
    short *sarr = (short *)malloc(n * 4);
    int *iarr = (int *)malloc(n * 4);
    long *larr = (long *)malloc(n * 4);
    
    if (!carr || !sarr || !iarr || !larr) {
        free(carr); free(sarr); free(iarr); free(larr);
        return -1;
    }
    
    /* Initialize */
    for (int i = 0; i < n * 4; i++) {
        carr[i] = (char)(i & 0xFF);
        if (i < n * 2) sarr[i] = (short)i;
        if (i < n) iarr[i] = i;
        if (i < n) larr[i] = i * 3L;
    }
    
    volatile char *volatile_cptr = carr;
    volatile short *volatile_sptr = sarr;
    volatile int *volatile_iptr = iarr;
    volatile long *volatile_lptr = larr;
    
    char *cptr = (char *)volatile_cptr;
    short *sptr = (short *)volatile_sptr;
    int *iptr = (int *)volatile_iptr;
    long *lptr = (long *)volatile_lptr;
    
    long sum = 0;
    
    /* Process each array with pointer arithmetic */
    for (int i = 0; i < n; i++) {
        sum += *cptr; cptr += 4;  /* Strided access */
        sum += *sptr; sptr += 2;  /* Different stride */
        sum += *iptr; iptr++;     /* Unit stride */
        sum += *lptr; lptr++;     /* Unit stride with different type */
    }
    
    COMPILER_BARRIER();
    
    free(carr);
    free(sarr);
    free(iarr);
    free(larr);
    
    return sum;
}

__attribute__((noinline, noipa))
static int test_pointer_copy(int n) {
    int *src = (int *)malloc(n * sizeof(int));
    int *dst = (int *)malloc(n * sizeof(int));
    
    if (!src || !dst) {
        free(src); free(dst);
        return -1;
    }
    
    /* Initialize source */
    for (int i = 0; i < n; i++) {
        src[i] = i * 7;
    }
    
    volatile int *volatile_src = src;
    volatile int *volatile_dst = dst;
    
    int *s = (int *)volatile_src;
    int *d = (int *)volatile_dst;
    
    /* Classic copy pattern: *dst++ = *src++ */
    for (int i = 0; i < n; i++) {
        *d = *s;  /* Memory access to address in s */
        d++;      /* Increment after access */
        s++;      /* Increment after access - both should be candidates */
    }
    
    /* Verify copy */
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += dst[i];
    }
    
    COMPILER_BARRIER();
    
    free(src);
    free(dst);
    
    return sum;
}

__attribute__((noinline, noipa))
static int test_complex_address_expression(int n) {
    int *array = (int *)malloc((n + 8) * sizeof(int));
    if (!array) return -1;
    
    for (int i = 0; i < n + 8; i++) {
        array[i] = i * 11;
    }
    
    volatile int *volatile_base = array + 4;  /* Start with offset */
    int *ptr = (int *)volatile_base;
    int sum = 0;
    
    /* Complex addressing: *(ptr + constant - constant) */
    for (int i = 0; i < n; i++) {
        /* This creates MEM address that needs analysis */
        sum += *(ptr + 2 - 2 + i);  /* Should simplify to ptr[i] */
        
        /* But we also increment ptr in a way that might be merged */
        ptr += 1;  /* This increment might be recognized */
    }
    
    /* Alternative: pointer arithmetic in the loop */
    int *p2 = array;
    for (int i = 0; i < n; i++) {
        /* Access with offset, then increment */
        sum += p2[3];  /* Constant offset access */
        p2++;          /* Base pointer increment */
    }
    
    COMPILER_BARRIER();
    free(array);
    return sum;
}

int main(int argc, char **argv) {
    /* Use argc to make loop bounds non-constant */
    int n = (argc > 1) ? atoi(argv[1]) : 100;
    if (n <= 0) n = 100;
    if (n > 10000) n = 10000;  /* Limit for safety */
    
    printf("Testing auto-inc-dec patterns with n=%d\n", n);
    
    /* Allocate test arrays */
    int *int_array = (int *)malloc(n * sizeof(int));
    long long *ll_array = (long long *)malloc(n * sizeof(long long));
    
    if (!int_array || !ll_array) {
        fprintf(stderr, "Memory allocation failed\n");
        free(int_array); free(ll_array);
        return 1;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < n; i++) {
        int_array[i] = i * 3 + 1;
        ll_array[i] = i * 5LL + 2;
    }
    
    long total = 0;
    
    /* Run all test patterns */
    total += test_pointer_loop_sum(int_array, n);
    total += test_mixed_index_pointer(int_array, n);
    total += test_multiple_base_registers(ll_array, ll_array, n);
    total += test_structure_pointer_walk(n);
    total += test_different_sizes(n / 4);  /* Smaller size for allocation */
    total += test_pointer_copy(n);
    total += test_complex_address_expression(n);
    
    printf("Total checksum: %ld\n", total);
    
    /* Verify with simple calculation */
    long expected = 0;
    for (int i = 0; i < n; i++) {
        expected += int_array[i];
    }
    printf("Array sum: %ld\n", expected);
    
    free(int_array);
    free(ll_array);
    
    return 0;
}

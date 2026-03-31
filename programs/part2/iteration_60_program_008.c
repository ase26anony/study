/* test_auto_inc_dec.c - Target auto-inc-dec.cc lines 1352-1358 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Compiler barriers to prevent reordering but preserve patterns */
#define COMPILER_BARRIER() asm volatile("" : : : "memory")

/* Test functions marked noinline to preserve boundaries */
__attribute__((noinline))
int test_pointer_loop_sum(int *array, int n) {
    volatile int *volatile_ptr = array; /* Volatile to prevent constant folding */
    int *ptr = (int*)volatile_ptr;
    int sum = 0;
    
    /* Simple pointer increment pattern */
    for (int i = 0; i < n; i++) {
        sum += *ptr;  /* Memory access via pointer */
        ptr++;        /* Increment adjacent - should trigger auto-inc-dec */
    }
    
    COMPILER_BARRIER(); /* Outside loop to preserve pattern */
    return sum;
}

__attribute__((noinline))
int test_mixed_index_pointer(int *array, int n) {
    volatile int *volatile_base = array;
    int *base = (int*)volatile_base;
    int sum = 0;
    
    /* Mixed indexing and pointer arithmetic */
    for (int i = 0; i < n; i++) {
        /* Complex addressing: base + i - i + 0 */
        sum += *(base + i - i);  /* Should simplify to *base but requires analysis */
        base++;  /* Increment after access */
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_dual_pointer_copy(int *src, int *dst, int n) {
    volatile int *volatile_src = src;
    volatile int *volatile_dst = dst;
    int *s = (int*)volatile_src;
    int *d = (int*)volatile_dst;
    
    /* Classic pointer copy pattern */
    for (int i = 0; i < n; i++) {
        *d = *s;  /* Memory access */
        d++;      /* Dual increments */
        s++;
    }
    
    COMPILER_BARRIER();
    return n;
}

__attribute__((noinline))
int test_structure_pointer_loop(int n) {
    struct S {
        int a;
        int b;
        short c;
        char d;
    };
    
    /* Dynamic allocation prevents compile-time optimization */
    struct S *array = malloc(n * sizeof(struct S));
    if (!array) return -1;
    
    /* Initialize with pattern */
    for (int i = 0; i < n; i++) {
        array[i].a = i;
        array[i].b = i * 2;
        array[i].c = i & 0xFFFF;
        array[i].d = i & 0xFF;
    }
    
    volatile struct S *volatile_sp = array;
    struct S *sp = (struct S*)volatile_sp;
    int sum = 0;
    
    /* Structure access with pointer increment */
    for (int i = 0; i < n; i++) {
        /* Multiple member accesses followed by pointer increment */
        sum += sp->a + sp->b;
        sum += sp->c + sp->d;
        sp++;  /* Increment after last access */
    }
    
    COMPILER_BARRIER();
    free(array);
    return sum;
}

__attribute__((noinline))
int test_mixed_sizes(int n) {
    /* Different pointer types for different access sizes */
    char *cptr = malloc(n * 4);
    short *sptr = (short*)cptr;
    int *iptr = (int*)cptr;
    long long *llptr = (long long*)cptr;
    
    if (!cptr) return -1;
    
    /* Initialize */
    for (int i = 0; i < n; i++) {
        cptr[i] = i & 0xFF;
        sptr[i] = i & 0xFFFF;
        iptr[i] = i;
        llptr[i] = i;
    }
    
    volatile char *volatile_cp = cptr;
    volatile short *volatile_sp = sptr;
    char *cp = (char*)volatile_cp;
    short *sp = (short*)volatile_sp;
    int sum = 0;
    
    /* Mixed size accesses with pointer arithmetic */
    for (int i = 0; i < n; i++) {
        sum += *cp;  /* char access */
        cp++;
        
        sum += *sp;  /* short access */
        sp++;
        
        /* Complex addressing with offset */
        sum += *(iptr + i - i);  /* Should become *iptr */
        /* iptr not incremented here - testing different pattern */
    }
    
    /* Separate loop for int pointer increment */
    for (int i = 0; i < n; i++) {
        sum += *iptr;
        iptr++;  /* int pointer increment */
    }
    
    COMPILER_BARRIER();
    free(cptr);
    return sum;
}

__attribute__((noinline))
int test_complex_addressing(int *base1, int *base2, int n) {
    volatile int *volatile_b1 = base1;
    volatile int *volatile_b2 = base2;
    int *p1 = (int*)volatile_b1;
    int *p2 = (int*)volatile_b2;
    int sum = 0;
    
    /* Complex addressing that might require XEXP analysis */
    for (int i = 0; i < n; i++) {
        /* p1[p2 - p2 + 0] should simplify to *p1 */
        sum += p1[p2 - p2];  /* Complex base expression */
        p1++;  /* Increment after access */
        
        /* Another complex form: *(p1 + (p2 - p2)) */
        sum += *(p1 + (p2 - p2));
        /* p1 already incremented above */
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_pointer_with_constant_offset(int *array, int n, int stride) {
    volatile int *volatile_ptr = array;
    int *ptr = (int*)volatile_ptr;
    int sum = 0;
    
    /* Pointer with constant offset in addressing */
    for (int i = 0; i < n; i++) {
        /* Access with offset: *(ptr + 0) */
        sum += *(ptr + 0);  /* Constant offset of 0 */
        ptr += stride;  /* Variable stride increment */
    }
    
    COMPILER_BARRIER();
    return sum;
}

int main(int argc, char **argv) {
    /* Use argc for non-constant loop bounds */
    int n = (argc > 1) ? atoi(argv[1]) : 100;
    if (n <= 0) n = 100;
    if (n > 10000) n = 10000; /* Reasonable limit */
    
    int stride = (argc > 2) ? atoi(argv[2]) : 1;
    
    /* Allocate test arrays */
    int *array1 = malloc(n * sizeof(int));
    int *array2 = malloc(n * sizeof(int));
    
    if (!array1 || !array2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < n; i++) {
        array1[i] = i * 3 + 1;
        array2[i] = 0;
    }
    
    int total = 0;
    
    /* Run all test patterns */
    total += test_pointer_loop_sum(array1, n);
    total += test_mixed_index_pointer(array1, n);
    total += test_dual_pointer_copy(array1, array2, n);
    total += test_structure_pointer_loop(n / 4); /* Smaller for structures */
    total += test_mixed_sizes(n / 2);
    total += test_complex_addressing(array1, array2, n);
    total += test_pointer_with_constant_offset(array1, n, stride);
    
    /* Verify copy worked */
    int verify_sum = 0;
    for (int i = 0; i < n; i++) {
        verify_sum += array2[i];
    }
    total += verify_sum;
    
    printf("Result: %d\n", total);
    
    free(array1);
    free(array2);
    
    return 0;
}

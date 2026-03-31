/* test_auto_inc_dec.c
 * Designed to trigger GCC's auto-increment/decrement pattern recognition
 * Target: auto-inc-dec.cc lines 1352-1358
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prevent inlining to preserve function boundaries */
#define NOINLINE __attribute__((noinline))

/* Memory barrier to prevent reordering without breaking patterns */
#define COMPILER_BARRIER() asm volatile("" : : : "memory")

/* Structure for pointer walking tests */
struct MixedData {
    char c;
    int i;
    short s;
    long long ll;
    volatile int v; /* volatile member to inhibit optimizations */
};

/* Volatile globals to prevent constant propagation */
volatile int g_loop_count = 100;
volatile int g_stride = 1;

/* Test 1: Basic pointer increment with mixed access patterns */
NOINLINE long long test_basic_pointer_inc(int *array, int n) {
    volatile int *volatile_ptr = array; /* Volatile pointer to prevent simplification */
    int *ptr = (int *)volatile_ptr;
    long long sum = 0;
    
    /* Pattern 1: Simple pointer increment */
    for (int i = 0; i < n; i++) {
        sum += *ptr;  /* Memory access via pointer */
        ptr++;        /* Increment immediately after - should trigger auto-inc */
    }
    
    COMPILER_BARRIER();
    return sum;
}

/* Test 2: Pointer with constant offset - creates complex addressing */
NOINLINE long long test_pointer_offset(short *array, int n) {
    volatile short *volatile_base = array;
    short *base = (short *)volatile_base;
    short *ptr = base + 3; /* Start with offset */
    long long sum = 0;
    
    /* Access via ptr[constant] and ptr++ */
    for (int i = 0; i < n; i++) {
        sum += ptr[0];      /* *(ptr + 0) */
        sum += ptr[2];      /* *(ptr + 2) - different offset */
        ptr += g_stride;    /* Volatile stride prevents constant folding */
    }
    
    /* Mixed index and pointer forms */
    short *p2 = base;
    for (int i = 0; i < n; i++) {
        sum += p2[i];       /* Array indexing */
        sum += *(p2 + i);   /* Pointer arithmetic - same but different form */
        p2++;               /* Pointer increment */
    }
    
    COMPILER_BARRIER();
    return sum;
}

/* Test 3: Structure access with pointer walking */
NOINLINE long long test_struct_pointer(struct MixedData *array, int n) {
    volatile struct MixedData *volatile_sp = array;
    struct MixedData *sp = (struct MixedData *)volatile_sp;
    long long sum = 0;
    
    /* Access multiple structure members with single pointer increment */
    for (int i = 0; i < n; i++) {
        sum += sp->c;      /* char access */
        sum += sp->i;      /* int access - different size */
        sum += sp->s;      /* short access */
        sum += sp->ll;     /* long long access - largest size */
        sp++;              /* Increment after all accesses */
    }
    
    COMPILER_BARRIER();
    return sum;
}

/* Test 4: Multiple base registers and complex addressing */
NOINLINE long long test_multiple_pointers(char *arr1, int *arr2, int n) {
    volatile char *volatile_cptr = arr1;
    volatile int *volatile_iptr = arr2;
    char *cptr = (char *)volatile_cptr;
    int *iptr = (int *)volatile_iptr;
    long long sum = 0;
    
    /* Two independent pointer chains */
    for (int i = 0; i < n; i++) {
        /* First pointer chain */
        sum += *cptr;
        cptr += 2;  /* Different stride */
        
        /* Second pointer chain - interleaved */
        sum += *iptr;
        iptr++;     /* Unit stride */
        
        /* Create addressing that could be seen as base+index */
        sum += cptr[iptr - iptr];  /* cptr[0] but with complex expression */
    }
    
    COMPILER_BARRIER();
    return sum;
}

/* Test 5: Copy between arrays using moving pointers */
NOINLINE void test_copy_pointers(int *src, int *dst, int n) {
    volatile int *volatile_src = src;
    volatile int *volatile_dst = dst;
    int *s = (int *)volatile_src;
    int *d = (int *)volatile_dst;
    
    /* Classic copy pattern: *dst++ = *src++ */
    for (int i = 0; i < n; i++) {
        *d = *s;  /* Read from src, write to dst */
        d++;      /* Increment both pointers */
        s++;
    }
    
    /* Reverse copy with different stride */
    s = (int *)volatile_src + n - 1;
    d = (int *)volatile_dst;
    for (int i = 0; i < n; i++) {
        *d = *s;
        d++;
        s--;  /* Decrement instead of increment */
    }
    
    COMPILER_BARRIER();
}

/* Test 6: Fill array with pointer write and increment */
NOINLINE void test_fill_array(long long *array, int n, long long value) {
    volatile long long *volatile_ptr = array;
    long long *ptr = (long long *)volatile_ptr;
    
    for (int i = 0; i < n; i++) {
        *ptr = value;  /* Write via pointer */
        ptr++;         /* Increment after write */
    }
    
    COMPILER_BARRIER();
}

/* Test 7: Mixed types and access sizes */
NOINLINE long long test_mixed_types(int n) {
    /* Local arrays of different types */
    char chars[256];
    short shorts[256];
    int ints[256];
    long long longs[256];
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) {
        chars[i] = i % 128;
        shorts[i] = i * 2;
        ints[i] = i * 100;
        longs[i] = i * 1000LL;
    }
    
    long long sum = 0;
    char *cptr = chars;
    short *sptr = shorts;
    int *iptr = ints;
    long long *llptr = longs;
    
    /* Access all arrays with pointer arithmetic */
    for (int i = 0; i < n && i < 256; i++) {
        sum += *cptr; cptr++;
        sum += *sptr; sptr += 1;  /* Explicit constant */
        sum += *iptr; iptr += g_stride;  /* Volatile stride */
        sum += *llptr; llptr++;
    }
    
    COMPILER_BARRIER();
    return sum;
}

/* Main driver that calls all tests */
int main(int argc, char **argv) {
    /* Use argc to make loop bounds non-constant */
    int n = (argc > 1) ? atoi(argv[1]) : 100;
    if (n <= 0) n = 100;
    if (n > 1000) n = 1000;  /* Reasonable limit */
    
    printf("Running auto-inc-dec tests with n=%d\n", n);
    
    /* Allocate and initialize test arrays */
    int *int_array = (int *)malloc(n * sizeof(int));
    short *short_array = (short *)malloc(n * sizeof(short));
    struct MixedData *struct_array = (struct MixedData *)malloc(n * sizeof(struct MixedData));
    char *char_array = (char *)malloc(n * sizeof(char));
    int *src_array = (int *)malloc(n * sizeof(int));
    int *dst_array = (int *)malloc(n * sizeof(int));
    long long *ll_array = (long long *)malloc(n * sizeof(long long));
    
    if (!int_array || !short_array || !struct_array || 
        !char_array || !src_array || !dst_array || !ll_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < n; i++) {
        int_array[i] = i * 3;
        short_array[i] = i * 5;
        char_array[i] = i % 128;
        src_array[i] = i * 7;
        dst_array[i] = 0;
        ll_array[i] = i * 1000LL;
        
        struct_array[i].c = i % 128;
        struct_array[i].i = i * 11;
        struct_array[i].s = i * 13;
        struct_array[i].ll = i * 10000LL;
        struct_array[i].v = i;  /* volatile member */
    }
    
    long long total_sum = 0;
    
    /* Run all tests */
    total_sum += test_basic_pointer_inc(int_array, n);
    total_sum += test_pointer_offset(short_array, n);
    total_sum += test_struct_pointer(struct_array, n);
    total_sum += test_multiple_pointers(char_array, int_array, n);
    test_copy_pointers(src_array, dst_array, n);
    test_fill_array(ll_array, n, 0x123456789ABCDEFLL);
    total_sum += test_mixed_types(n);
    
    /* Verify copy worked */
    for (int i = 0; i < n; i++) {
        total_sum += dst_array[i];
    }
    
    /* Verify fill worked */
    for (int i = 0; i < n; i++) {
        total_sum += ll_array[i];
    }
    
    printf("Total checksum: %lld\n", total_sum);
    
    /* Cleanup */
    free(int_array);
    free(short_array);
    free(struct_array);
    free(char_array);
    free(src_array);
    free(dst_array);
    free(ll_array);
    
    return 0;
}

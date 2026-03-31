/* auto_inc_dec_test.c
 * Test program to trigger GCC's auto-increment/decrement pattern recognition
 * Compile with: gcc -O2 -fno-unroll-loops -fno-tree-vectorize -c auto_inc_dec_test.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_SIZE 1024

/* Prevent inlining to preserve function boundaries */
#define NOINLINE __attribute__((noinline))

/* Structures to force complex addressing */
struct MixedData {
    int a;
    char b;
    short c;
    long long d;
};

struct Nested {
    struct MixedData inner;
    int extra;
};

/* Test 1: Basic pointer increment in loop */
NOINLINE long long test1_basic_pointer_increment(int *array, int n) {
    volatile int *volatile_ptr = array; /* volatile to prevent early optimization */
    int *ptr = (int *)volatile_ptr;
    long long sum = 0;
    
    for (int i = 0; i < n; i++) {
        sum += *ptr;    /* Memory access */
        ptr++;          /* Pointer increment - should trigger auto-inc pattern */
    }
    
    /* Compiler barrier to prevent reordering but keep pattern intact */
    asm volatile("" : : : "memory");
    return sum;
}

/* Test 2: Mixed array indexing and pointer arithmetic */
NOINLINE long long test2_mixed_index_pointer(int *array, int n) {
    volatile int *volatile_base = array;
    int *base = (int *)volatile_base;
    long long sum = 0;
    
    /* Use both indexing and pointer arithmetic */
    for (int i = 0; i < n; i++) {
        sum += base[i];         /* Array index form */
        sum += *(base + i);     /* Pointer+offset form */
        /* base++ would be too simple - use complex expression */
    }
    
    /* Now use pure pointer arithmetic in separate loop */
    int *p = base;
    for (int i = 0; i < n; i++) {
        sum += *p;
        p = p + 1;  /* Not p++ to create different pattern */
    }
    
    asm volatile("" : : : "memory");
    return sum;
}

/* Test 3: Structure access with pointer walking */
NOINLINE long long test3_struct_pointer_walk(struct MixedData *sarray, int n) {
    volatile struct MixedData *volatile_sp = sarray;
    struct MixedData *sp = (struct MixedData *)volatile_sp;
    long long sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Access multiple structure members */
        sum += sp->a;
        sum += sp->b;
        sum += sp->c;
        sum += sp->d;
        
        sp++;  /* Pointer increment after last access */
    }
    
    asm volatile("" : : : "memory");
    return sum;
}

/* Test 4: Copy between arrays using two moving pointers */
NOINLINE void test4_pointer_copy(int *dest, int *src, int n) {
    volatile int *volatile_dest = dest;
    volatile int *volatile_src = src;
    int *d = (int *)volatile_dest;
    int *s = (int *)volatile_src;
    
    /* Classic copy pattern that should trigger auto-inc */
    for (int i = 0; i < n; i++) {
        *d = *s;    /* Memory access from two pointers */
        d++;        /* Both pointers increment */
        s++;        /* Should create base+0 addressing */
    }
    
    /* Alternative with offset */
    d = (int *)volatile_dest;
    s = (int *)volatile_src;
    for (int i = 0; i < n; i++) {
        d[i] = s[i];  /* Indexed form */
        /* Force pointer arithmetic nearby */
        if (i % 2) {
            d = d + 1;
            s = s + 1;
        }
    }
    
    asm volatile("" : : : "memory");
}

/* Test 5: Complex addressing with multiple base registers */
NOINLINE long long test5_complex_addressing(int *array1, int *array2, int n) {
    volatile int *volatile_p1 = array1;
    volatile int *volatile_p2 = array2;
    int *p1 = (int *)volatile_p1;
    int *p2 = (int *)volatile_p2;
    long long sum = 0;
    
    /* Create complex address expressions that might simplify to base+0 */
    for (int i = 0; i < n; i++) {
        /* Expression: *(p1 + (p2 - p2)) which should equal *p1 */
        sum += *(p1 + (p2 - p2));
        
        /* Another complex expression */
        sum += p1[p2 - p2];
        
        p1++;  /* Increment after access */
    }
    
    /* Use pointer with constant offset */
    p1 = (int *)volatile_p1;
    for (int i = 0; i < n; i++) {
        sum += *(p1 + 4);  /* Constant offset */
        p1 += 2;           /* Non-unit stride */
    }
    
    asm volatile("" : : : "memory");
    return sum;
}

/* Test 6: Different memory access sizes */
NOINLINE long long test6_mixed_sizes(char *carr, short *sarr, int *iarr, long long *llarr, int n) {
    volatile char *volatile_cp = carr;
    volatile short *volatile_sp = sarr;
    volatile int *volatile_ip = iarr;
    volatile long long *volatile_lp = llarr;
    
    char *cp = (char *)volatile_cp;
    short *sp = (short *)volatile_sp;
    int *ip = (int *)volatile_ip;
    long long *lp = (long long *)volatile_lp;
    
    long long sum = 0;
    
    /* Process each type with pointer increment */
    for (int i = 0; i < n; i++) {
        sum += *cp;
        cp++;
    }
    
    for (int i = 0; i < n; i++) {
        sum += *sp;
        sp++;
    }
    
    for (int i = 0; i < n; i++) {
        sum += *ip;
        ip++;
    }
    
    for (int i = 0; i < n; i++) {
        sum += *lp;
        lp++;
    }
    
    asm volatile("" : : : "memory");
    return sum;
}

/* Test 7: Nested structure with pointer arithmetic */
NOINLINE long long test7_nested_struct(struct Nested *narray, int n) {
    volatile struct Nested *volatile_np = narray;
    struct Nested *np = (struct Nested *)volatile_np;
    long long sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Access nested structure members */
        sum += np->inner.a;
        sum += np->inner.b;
        sum += np->inner.c;
        sum += np->inner.d;
        sum += np->extra;
        
        /* Pointer increment with potential auto-inc */
        np = np + 1;  /* Use np++ alternative */
    }
    
    /* Second loop with post-increment in expression */
    np = (struct Nested *)volatile_np;
    for (int i = 0; i < n; i++) {
        sum += (np++)->extra;  /* Post-increment in access */
    }
    
    asm volatile("" : : : "memory");
    return sum;
}

/* Test 8: Fill array with pointer write */
NOINLINE void test8_pointer_fill(int *array, int value, int n) {
    volatile int *volatile_ptr = array;
    int *ptr = (int *)volatile_ptr;
    
    /* Fill using pointer write and increment */
    for (int i = 0; i < n; i++) {
        *ptr = value;  /* Memory write */
        ptr++;         /* Pointer increment */
    }
    
    /* Alternative with pre-increment */
    ptr = (int *)volatile_ptr;
    for (int i = 0; i < n; i++) {
        *(++ptr) = value;  /* Pre-increment */
    }
    
    asm volatile("" : : : "memory");
}

int main(int argc, char **argv) {
    /* Use argc to make loop bounds non-constant */
    int n = (argc > 1) ? atoi(argv[1]) : ARRAY_SIZE;
    if (n > ARRAY_SIZE) n = ARRAY_SIZE;
    if (n < 10) n = 10;
    
    /* Allocate and initialize arrays */
    int *array1 = (int *)malloc(n * sizeof(int));
    int *array2 = (int *)malloc(n * sizeof(int));
    char *carray = (char *)malloc(n * sizeof(char));
    short *sarray = (short *)malloc(n * sizeof(short));
    long long *llarray = (long long *)malloc(n * sizeof(long long));
    struct MixedData *sarr = (struct MixedData *)malloc(n * sizeof(struct MixedData));
    struct Nested *narr = (struct Nested *)malloc(n * sizeof(struct Nested));
    
    /* Initialize with pattern */
    for (int i = 0; i < n; i++) {
        array1[i] = i * 3 + 1;
        array2[i] = i * 5 + 2;
        carray[i] = (char)(i % 256);
        sarray[i] = (short)(i * 7);
        llarray[i] = i * 11LL;
        sarr[i].a = i;
        sarr[i].b = (char)(i % 128);
        sarr[i].c = (short)(i * 2);
        sarr[i].d = i * 13LL;
        narr[i].inner = sarr[i];
        narr[i].extra = i * 17;
    }
    
    long long checksum = 0;
    
    /* Run all tests */
    checksum += test1_basic_pointer_increment(array1, n);
    checksum += test2_mixed_index_pointer(array1, n);
    checksum += test3_struct_pointer_walk(sarr, n);
    
    test4_pointer_copy(array2, array1, n);
    checksum += test5_complex_addressing(array1, array2, n);
    checksum += test6_mixed_sizes(carray, sarray, array1, llarray, n);
    checksum += test7_nested_struct(narr, n);
    
    test8_pointer_fill(array1, 0xABCD, n);
    
    /* Final computation to use results */
    for (int i = 0; i < n; i++) {
        checksum += array1[i] + array2[i];
    }
    
    printf("Final checksum: %lld\n", checksum);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(carray);
    free(sarray);
    free(llarray);
    free(sarr);
    free(narr);
    
    return 0;
}

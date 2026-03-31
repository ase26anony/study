/* test_auto_inc_dec.c */
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
        ptr++;  /* This should trigger auto-inc-dec pattern */
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
void test_pointer_copy(volatile int *src, int *dst, volatile int n) {
    volatile int *s = src;
    int *d = dst;
    int *end = d + n;
    
    /* Copy with pointer increment - classic auto-inc pattern */
    while (d < end) {
        *d = *s;
        d++;
        s++;
    }
    
    COMPILER_BARRIER();
}

__attribute__((noinline))
long long test_mixed_types(volatile char *cptr, volatile short *sptr, 
                           volatile int *iptr, volatile long long *llptr, 
                           int n) {
    long long total = 0;
    volatile char *cp = cptr;
    volatile short *sp = sptr;
    volatile int *ip = iptr;
    volatile long long *lp = llptr;
    
    /* Mixed pointer types with different increments */
    for (int i = 0; i < n; i++) {
        total += *cp;
        cp++;  /* char pointer increments by 1 */
        
        total += *sp;
        sp++;  /* short pointer increments by 2 */
        
        total += *ip;
        ip++;  /* int pointer increments by 4 */
        
        total += *lp;
        lp++;  /* long long pointer increments by 8 */
    }
    
    COMPILER_BARRIER();
    return total;
}

__attribute__((noinline))
int test_complex_addressing(int *base, volatile int offset, int n) {
    int sum = 0;
    
    /* Complex addressing: *(ptr + offset + i) pattern */
    for (int i = 0; i < n; i++) {
        /* This creates XEXP(x, 0) = (plus (reg) (const_int)) */
        sum += *(base + offset + i);
    }
    
    /* Follow with pointer arithmetic that could be merged */
    int *ptr = base + offset;
    for (int i = 0; i < n; i++) {
        sum += *ptr;
        ptr++;  /* Adjacent increment */
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_structure_access(volatile int n) {
    struct Data {
        int a;
        int b;
        int c;
        int d;
    };
    
    struct Data *array = malloc(n * sizeof(struct Data));
    if (!array) return 0;
    
    /* Initialize */
    for (int i = 0; i < n; i++) {
        array[i].a = i;
        array[i].b = i * 2;
        array[i].c = i * 3;
        array[i].d = i * 4;
    }
    
    /* Structure access with pointer walking */
    int sum = 0;
    struct Data *ptr = array;
    struct Data *end = array + n;
    
    while (ptr < end) {
        /* Multiple member accesses followed by pointer increment */
        sum += ptr->a;
        sum += ptr->b;
        sum += ptr->c;
        sum += ptr->d;
        ptr++;  /* This increment should be adjacent to last access */
    }
    
    COMPILER_BARRIER();
    free(array);
    return sum;
}

__attribute__((noinline))
int test_dual_pointer_arithmetic(int *arr1, volatile int *arr2, int n) {
    int sum = 0;
    
    /* Two pointers with arithmetic that might create complex addressing */
    int *p1 = arr1;
    volatile int *p2 = arr2;
    
    for (int i = 0; i < n; i++) {
        /* Complex address: p1[p2 - p2 + i] simplifies but requires analysis */
        sum += p1[p2 - p2 + i];
        
        /* Also use direct pointer dereference with offset */
        sum += *(p1 + i);
        
        /* And pointer with constant offset */
        sum += p1[3];
    }
    
    /* Follow with simple pointer increment loop */
    p1 = arr1;
    for (int i = 0; i < n; i++) {
        sum += *p1;
        p1++;  /* Should trigger find_inc(true) */
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_volatile_pointer_chain(volatile int *start, int n) {
    int sum = 0;
    
    /* Chain of volatile pointers to prevent optimization */
    volatile int *ptr1 = start;
    volatile int *ptr2 = ptr1 + 1;
    volatile int *ptr3 = ptr2 + 1;
    
    for (int i = 0; i < n; i++) {
        /* Multiple volatile accesses */
        sum += *ptr1;
        sum += *ptr2;
        sum += *ptr3;
        
        /* Increment all pointers */
        ptr1++;
        ptr2++;
        ptr3++;
    }
    
    COMPILER_BARRIER();
    return sum;
}

int main(int argc, char **argv) {
    /* Use argc to make loop bounds non-constant */
    volatile int size = (argc > 1) ? atoi(argv[1]) : 100;
    if (size <= 0) size = 100;
    
    /* Allocate arrays of different types */
    int *int_array = malloc(size * sizeof(int));
    char *char_array = malloc(size * sizeof(char));
    short *short_array = malloc(size * sizeof(short));
    long long *ll_array = malloc(size * sizeof(long long));
    int *src_array = malloc(size * sizeof(int));
    int *dst_array = malloc(size * sizeof(int));
    
    if (!int_array || !char_array || !short_array || !ll_array || 
        !src_array || !dst_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < size; i++) {
        int_array[i] = i;
        char_array[i] = (char)(i % 128);
        short_array[i] = (short)(i % 32768);
        ll_array[i] = i * 100LL;
        src_array[i] = size - i;
    }
    
    int checksum = 0;
    
    /* Test 1: Simple pointer increment loop */
    checksum += test_pointer_loop_sum(int_array, size);
    
    /* Test 2: Pointer copy */
    test_pointer_copy(src_array, dst_array, size);
    for (int i = 0; i < size; i++) {
        checksum += dst_array[i];
    }
    
    /* Test 3: Mixed types */
    checksum += test_mixed_types(char_array, short_array, int_array, ll_array, size);
    
    /* Test 4: Complex addressing */
    checksum += test_complex_addressing(int_array, size/2, size/2);
    
    /* Test 5: Structure access */
    checksum += test_structure_access(size);
    
    /* Test 6: Dual pointer arithmetic */
    checksum += test_dual_pointer_arithmetic(int_array, src_array, size);
    
    /* Test 7: Volatile pointer chain */
    checksum += test_volatile_pointer_chain(src_array, size);
    
    printf("Final checksum: %d\n", checksum);
    
    /* Cleanup */
    free(int_array);
    free(char_array);
    free(short_array);
    free(ll_array);
    free(src_array);
    free(dst_array);
    
    return 0;
}

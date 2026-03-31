/* auto_inc_dec_test.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Compiler barrier to prevent reordering */
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
int test_short_pointer_arithmetic(volatile short *data, int n, int stride) {
    short *ptr = (short *)data;
    int sum = 0;
    
    /* Pointer with constant offset - creates complex addressing */
    for (int i = 0; i < n; i++) {
        /* Access with offset, then increment */
        sum += *(ptr + 0);  /* Force XEXP(x, 0) extraction */
        ptr += stride;      /* Variable stride prevents other optimizations */
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
long long test_longlong_mixed_index(volatile long long *data, int n) {
    long long *ptr = (long long *)data;
    long long sum = 0;
    
    /* Mix array indexing and pointer arithmetic */
    for (int i = 0; i < n; i++) {
        /* Multiple addressing forms */
        sum += ptr[0];      /* Array form */
        sum += *(ptr + 0);  /* Pointer+offset form */
        
        /* Complex expression that might simplify */
        long long *tmp = ptr;
        sum += *tmp;
        
        ptr++;  /* Increment after mixed accesses */
    }
    
    COMPILER_BARRIER();
    return sum;
}

/* Structure for pointer walking */
struct MixedData {
    char c;
    int i;
    short s;
    long long ll;
};

__attribute__((noinline))
long long test_struct_walk(volatile struct MixedData *data, int n) {
    struct MixedData *ptr = (struct MixedData *)data;
    long long sum = 0;
    
    /* Walk through struct array with pointer */
    for (int i = 0; i < n; i++) {
        /* Access multiple members */
        sum += ptr->c;
        sum += ptr->i;
        sum += ptr->s;
        sum += ptr->ll;
        
        ptr++;  /* Increment after last access */
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_double_pointer_math(volatile int *data1, volatile int *data2, int n) {
    int *p1 = (int *)data1;
    int *p2 = (int *)data2;
    int sum = 0;
    
    /* Complex pointer arithmetic that might create base+index addressing */
    for (int i = 0; i < n; i++) {
        /* Expression: p1 + (p2 - p2) which simplifies to p1 */
        int offset = (int)(p2 - p2);  /* Should be 0 but not compile-time constant */
        sum += *(p1 + offset);
        
        /* Another complex addressing mode */
        sum += p1[0];
        
        p1++;
        /* p2 stays constant or moves differently */
        if (i % 2 == 0) {
            p2++;
        }
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
void test_volatile_pointer(volatile int *volatile vptr, int n) {
    /* Volatile pointer to prevent optimization */
    volatile int *ptr = vptr;
    
    for (int i = 0; i < n; i++) {
        /* Write pattern */
        *ptr = i;
        ptr++;  /* Post-increment after write */
    }
    
    COMPILER_BARRIER();
}

int main(int argc, char **argv) {
    /* Use argc to make loop bounds non-constant */
    int n = (argc > 1) ? atoi(argv[1]) : 100;
    if (n < 10) n = 10;
    if (n > 1000) n = 1000;
    
    /* Allocate and initialize arrays of different types */
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
        ll_array[i] = (long long)i * i;
        
        struct_array[i].c = (char)(i % 128);
        struct_array[i].i = i * 7;
        struct_array[i].s = (short)(i * 11);
        struct_array[i].ll = (long long)i * i * i;
    }
    
    long long total_sum = 0;
    
    /* Run all test patterns */
    total_sum += test_char_sum((volatile char *)char_array, n);
    total_sum += test_short_pointer_arithmetic((volatile short *)short_array, n, 1);
    total_sum += test_int_copy((volatile int *)int_array1, (volatile int *)int_array2, n);
    total_sum += test_longlong_mixed_index((volatile long long *)ll_array, n);
    total_sum += test_struct_walk((volatile struct MixedData *)struct_array, n);
    total_sum += test_double_pointer_math((volatile int *)int_array1, 
                                         (volatile int *)int_array2, n);
    
    /* Test volatile pointer write pattern */
    test_volatile_pointer((volatile int *)int_array1, n);
    
    /* Verify copy worked */
    for (int i = 0; i < n; i++) {
        total_sum += int_array2[i];
    }
    
    printf("Total checksum: %lld\n", total_sum);
    
    /* Cleanup */
    free(char_array);
    free(short_array);
    free(int_array1);
    free(int_array2);
    free(ll_array);
    free(struct_array);
    
    return (total_sum > 0) ? 0 : 1;
}

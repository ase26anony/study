/* test_auto_inc_dec.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Compiler barriers to prevent reordering */
#define COMPILER_BARRIER() asm volatile("" : : : "memory")

/* Test functions marked noinline to preserve boundaries */
__attribute__((noinline))
int test_pointer_increment_sum(int *array, int n) {
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
void test_pointer_copy(int *dst, int *src, int n) {
    int *d = dst;
    int *s = src;
    int *end = src + n;
    
    /* Classic copy pattern with post-increment */
    while (s < end) {
        *d = *s;
        d++;
        s++;  /* Two pointers incrementing - complex addressing */
    }
    
    COMPILER_BARRIER();
}

__attribute__((noinline))
long long test_mixed_sizes(char *carr, short *sarr, int *iarr, long long *llarr, int n) {
    long long total = 0;
    char *cp = carr;
    short *sp = sarr;
    int *ip = iarr;
    long long *llp = llarr;
    
    /* Mixed pointer types with different increments */
    for (int i = 0; i < n; i++) {
        total += *cp;
        cp++;  /* char pointer increments by 1 */
        
        total += *sp;
        sp++;  /* short pointer increments by 2 */
        
        total += *ip;
        ip++;  /* int pointer increments by 4 */
        
        total += *llp;
        llp++;  /* long long pointer increments by 8 */
    }
    
    COMPILER_BARRIER();
    return total;
}

__attribute__((noinline))
int test_pointer_with_offset(int *array, int n, int stride) {
    int sum = 0;
    int *ptr = array;
    
    /* Pointer with constant offset in loop */
    for (int i = 0; i < n; i++) {
        sum += *(ptr + 0);  /* Force XEXP(x, 0) extraction */
        ptr += stride;      /* Variable stride to prevent optimization */
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_complex_addressing(int *array1, int *array2, int n) {
    int sum = 0;
    volatile int *volatile_ptr = array1;  /* Volatile to prevent simplification */
    int *ptr2 = array2;
    
    /* Complex addressing that might simplify to base+0 */
    for (int i = 0; i < n; i++) {
        sum += *(volatile_ptr + (ptr2 - ptr2));  /* Should become *(volatile_ptr + 0) */
        volatile_ptr++;  /* But volatile prevents early optimization */
    }
    
    COMPILER_BARRIER();
    return sum;
}

struct Data {
    int a;
    int b;
    char c;
    short d;
};

__attribute__((noinline))
int test_struct_pointer(struct Data *data, int n) {
    int sum = 0;
    struct Data *ptr = data;
    
    /* Structure access with pointer walking */
    for (int i = 0; i < n; i++) {
        sum += ptr->a;
        sum += ptr->b;
        sum += ptr->c;
        sum += ptr->d;
        ptr++;  /* Increment after multiple member accesses */
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
int test_mixed_index_pointer(int *array, int n) {
    int sum = 0;
    int *ptr = array;
    
    /* Mix array indexing and pointer arithmetic */
    for (int i = 0; i < n; i++) {
        sum += array[i];    /* Index form */
        sum += *(ptr + 0);  /* Pointer form with explicit offset */
        ptr++;              /* Pointer increment - should be adjacent to access */
    }
    
    COMPILER_BARRIER();
    return sum;
}

__attribute__((noinline))
void test_double_pointer_inc(int *dst, int *src1, int *src2, int n) {
    int *d = dst;
    int *s1 = src1;
    int *s2 = src2;
    
    /* Two source pointers, one destination - complex pattern */
    for (int i = 0; i < n; i++) {
        *d = *s1 + *s2;
        d++;
        s1++;
        s2++;  /* Three pointers incrementing */
    }
    
    COMPILER_BARRIER();
}

int main(int argc, char **argv) {
    /* Use argc to make loop bounds non-constant */
    int n = (argc > 1) ? atoi(argv[1]) : 100;
    if (n < 10) n = 10;
    if (n > 1000) n = 1000;
    
    /* Allocate and initialize arrays of different types */
    int *int_array1 = (int*)malloc(n * sizeof(int));
    int *int_array2 = (int*)malloc(n * sizeof(int));
    int *int_array3 = (int*)malloc(n * sizeof(int));
    char *char_array = (char*)malloc(n * sizeof(char));
    short *short_array = (short*)malloc(n * sizeof(short));
    long long *ll_array = (long long*)malloc(n * sizeof(long long));
    struct Data *struct_array = (struct Data*)malloc(n * sizeof(struct Data));
    
    /* Initialize with pattern */
    for (int i = 0; i < n; i++) {
        int_array1[i] = i;
        int_array2[i] = i * 2;
        int_array3[i] = i * 3;
        char_array[i] = (char)(i % 256);
        short_array[i] = (short)(i % 65536);
        ll_array[i] = i * 1000LL;
        struct_array[i].a = i;
        struct_array[i].b = i * 2;
        struct_array[i].c = (char)(i % 256);
        struct_array[i].d = (short)(i % 65536);
    }
    
    int checksum = 0;
    
    /* Run all test patterns */
    checksum += test_pointer_increment_sum(int_array1, n);
    
    test_pointer_copy(int_array3, int_array2, n);
    for (int i = 0; i < n; i++) {
        checksum += int_array3[i];
    }
    
    checksum += test_mixed_sizes(char_array, short_array, int_array1, ll_array, n);
    
    checksum += test_pointer_with_offset(int_array1, n, 1);
    
    checksum += test_complex_addressing(int_array1, int_array2, n);
    
    checksum += test_struct_pointer(struct_array, n);
    
    checksum += test_mixed_index_pointer(int_array1, n);
    
    test_double_pointer_inc(int_array3, int_array1, int_array2, n);
    for (int i = 0; i < n; i++) {
        checksum += int_array3[i];
    }
    
    printf("Final checksum: %d\n", checksum);
    
    /* Cleanup */
    free(int_array1);
    free(int_array2);
    free(int_array3);
    free(char_array);
    free(short_array);
    free(ll_array);
    free(struct_array);
    
    return 0;
}

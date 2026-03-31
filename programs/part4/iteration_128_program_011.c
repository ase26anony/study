/* test-auto-inc-dec.c */
#include <stdio.h>
#include <stdlib.h>

/* External function to prevent optimization */
extern void dummy_external(int);

/* Volatile variables to prevent constant propagation */
volatile int g_volatile_bound = 100;
volatile int g_volatile_sum = 0;

/* Function 1: Using *(p + 0) and p++ */
__attribute__((noinline))
int test1_zero_offset_plus_plus(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Access with explicit zero offset */
        int val = *(p + 0);
        sum += val;
        /* Separate increment */
        p++;
    }
    
    /* Use volatile to prevent elimination */
    g_volatile_sum += sum;
    return sum;
}

/* Function 2: Using p[0] and p += 1 */
__attribute__((noinline))
int test2_array_zero_plus_equals(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Array access with index 0 */
        int val = p[0];
        sum += val;
        /* Separate increment with += */
        p += 1;
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Function 3: Using structure with pointer member */
struct PointerHolder {
    int *ptr;
    int count;
};

__attribute__((noinline))
int test3_struct_member(struct PointerHolder *holder) {
    int *p = holder->ptr;
    int sum = 0;
    int n = holder->count;
    
    for (int i = 0; i < n; i++) {
        /* Access with *(p + 0) */
        int val = *(p + 0);
        sum += val;
        /* Decrement instead of increment */
        p = p + 1;
    }
    
    holder->ptr = p; /* Update structure member */
    g_volatile_sum += sum;
    return sum;
}

/* Function 4: Store instead of load */
__attribute__((noinline))
void test4_store_zero_offset(int *dest, int *src, int n) {
    int *p_dest = dest;
    int *p_src = src;
    
    for (int i = 0; i < n; i++) {
        /* Store with zero offset */
        *(p_dest + 0) = *p_src;
        /* Separate increments */
        p_dest = p_dest + 1;
        p_src++;
    }
    
    /* Force side effect */
    asm volatile("" : : "r"(dest[0]), "r"(src[0]) : "memory");
}

/* Function 5: Different increment patterns */
__attribute__((noinline))
int test5_mixed_patterns(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    /* First half: load with zero offset, post-increment */
    for (int i = 0; i < n/2; i++) {
        sum += *(p + 0);
        p++;
    }
    
    /* Second half: load with zero offset, pre-increment */
    for (int i = n/2; i < n; i++) {
        p = p + 1;
        sum += *(p - 1 + 0); /* Still has zero offset after arithmetic */
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Function 6: Using pointer to pointer */
__attribute__((noinline))
int test6_pointer_to_pointer(int **arr_pp, int n) {
    int **pp = arr_pp;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Dereference pointer, then access with zero offset */
        int *p = *pp;
        sum += *(p + 0);
        /* Increment the pointer-to-pointer */
        pp++;
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Main function */
int main(int argc, char **argv) {
    /* Use argc to make array size non-constant */
    int size = g_volatile_bound + argc;
    if (size < 10) size = 100;
    
    /* Allocate and initialize arrays */
    int *arr1 = (int *)malloc(size * sizeof(int));
    int *arr2 = (int *)malloc(size * sizeof(int));
    int **arr_pp = (int **)malloc(size * sizeof(int *));
    
    for (int i = 0; i < size; i++) {
        arr1[i] = i + 1;
        arr2[i] = i * 2;
        arr_pp[i] = &arr1[i];
    }
    
    /* Create struct for test3 */
    struct PointerHolder holder;
    holder.ptr = arr1;
    holder.count = size;
    
    int total = 0;
    
    /* Call all test functions */
    total += test1_zero_offset_plus_plus(arr1, size);
    total += test2_array_zero_plus_equals(arr1, size);
    total += test3_struct_member(&holder);
    test4_store_zero_offset(arr2, arr1, size);
    total += test5_mixed_patterns(arr1, size);
    total += test6_pointer_to_pointer(arr_pp, size);
    
    /* Use results to prevent dead code elimination */
    printf("Total sum: %d (volatile: %d)\n", total, g_volatile_sum);
    
    /* Clean up */
    free(arr1);
    free(arr2);
    free(arr_pp);
    
    return 0;
}

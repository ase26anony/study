/* Compile with: gcc -O2 -fno-omit-frame-pointer -march=armv7-a -fno-strict-aliasing -o test_auto_inc test_auto_inc.c */

#include <stdio.h>
#include <stdlib.h>

/* External dummy function to prevent elimination */
extern void dummy_use(int val);

/* Volatile variable to prevent constant propagation */
volatile int g_volatile_bound = 100;

/* Prevent inlining to isolate the pattern */
__attribute__((noinline))
int test_ptr_plus_zero(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    /* Loop with pointer + 0 access followed by increment */
    for (int i = 0; i < n; i++) {
        /* Access with explicit zero offset - should not fold to *p */
        int val = *(p + 0);
        
        /* Separate increment statement */
        p++;
        
        /* Use the value to prevent elimination */
        sum += val;
        dummy_use(val);
    }
    
    return sum;
}

__attribute__((noinline))
int test_array_zero_index(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    /* Alternative: array notation with [0] */
    for (int i = 0; i < n; i++) {
        /* Access with zero index */
        int val = p[0];
        
        /* Increment with += 1 */
        p += 1;
        
        sum += val;
        asm volatile("" : : "r"(val)); /* Prevent elimination */
    }
    
    return sum;
}

__attribute__((noinline))
int test_store_pattern(int *arr, int n, int value) {
    int *p = arr;
    
    /* Store instead of load */
    for (int i = 0; i < n; i++) {
        /* Store with zero offset */
        *(p + 0) = value + i;
        
        /* Decrement pattern */
        p--;
        
        /* Memory barrier to prevent reordering */
        asm volatile("" : : : "memory");
    }
    
    return 0;
}

__attribute__((noinline))
int test_mixed_offsets(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    /* Mix positive and negative increments */
    for (int i = 0; i < n; i++) {
        if (i & 1) {
            /* Even iterations: *(p + 0) */
            sum += *(p + 0);
            p = p + 1;  /* Alternative increment syntax */
        } else {
            /* Odd iterations: p[0] */
            sum -= p[0];
            p++;        /* Standard increment */
        }
    }
    
    return sum;
}

/* Structure with pointer member */
struct ptr_wrapper {
    int *current;
    int *end;
};

__attribute__((noinline))
int test_struct_member(struct ptr_wrapper *w) {
    int sum = 0;
    
    while (w->current < w->end) {
        /* Access through structure member with zero offset */
        int val = *(w->current + 0);
        
        /* Increment the structure member */
        w->current++;
        
        sum ^= val; /* Different operation to avoid pattern recognition */
    }
    
    return sum;
}

__attribute__((noinline))
int test_double_deref(int **arr_ptr, int n) {
    int **pp = arr_ptr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Double dereference with zero offset */
        int val = *(*(pp + 0) + 0);
        
        /* Increment the pointer-to-pointer */
        pp++;
        
        sum |= val;
    }
    
    return sum;
}

/* Dummy function definition */
void dummy_use(int val) {
    /* Empty but prevents dead code elimination */
    asm volatile("" : : "r"(val));
}

int main(int argc, char *argv[]) {
    /* Dynamic size to prevent complete unrolling */
    int size = g_volatile_bound;
    if (argc > 1) {
        size = atoi(argv[1]);
        if (size <= 0) size = 100;
    }
    
    /* Allocate array with dynamic size */
    int *arr = (int*)malloc(size * sizeof(int));
    if (!arr) return 1;
    
    /* Initialize array */
    for (int i = 0; i < size; i++) {
        arr[i] = i + 1;
    }
    
    int total = 0;
    
    /* Call all test functions */
    total += test_ptr_plus_zero(arr, size);
    total += test_array_zero_index(arr, size);
    total += test_store_pattern(arr, size, 42);
    total += test_mixed_offsets(arr, size);
    
    /* Test with structure */
    struct ptr_wrapper w;
    w.current = arr;
    w.end = arr + size;
    total += test_struct_member(&w);
    
    /* Test double pointer */
    int **ptr_array = (int**)malloc(size * sizeof(int*));
    for (int i = 0; i < size; i++) {
        ptr_array[i] = &arr[i];
    }
    total += test_double_deref(ptr_array, size);
    
    free(ptr_array);
    free(arr);
    
    printf("Total: %d\n", total);
    return 0;
}

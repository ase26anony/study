/* test-auto-inc-dec.c
 * Program designed to trigger GCC's auto-increment/decrement optimization
 * for memory accesses with zero offset followed by pointer increments.
 */

#include <stdio.h>
#include <stdlib.h>

/* External function to prevent elimination of memory accesses */
extern void dummy_external(void*);

/* Volatile variables to prevent constant propagation */
volatile int g_volatile_bound = 100;
volatile int g_volatile_sum = 0;

/* Function 1: Using *(p + 0) and p++ */
__attribute__((noinline))
static int test1_zero_offset_plus_plus(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Access with explicit zero offset - should not be folded by frontend */
        int val = *(p + 0);
        sum += val;
        /* Separate increment statement */
        p++;
    }
    
    /* Use result to prevent elimination */
    g_volatile_sum += sum;
    return sum;
}

/* Function 2: Using p[0] and p += 1 */
__attribute__((noinline))
static int test2_array_zero_plus_equals(int *arr, int n) {
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
struct ptr_wrapper {
    int *current;
    int *end;
};

__attribute__((noinline))
static int test3_struct_member(int *arr, int n) {
    struct ptr_wrapper wrapper;
    wrapper.current = arr;
    wrapper.end = arr + n;
    
    int sum = 0;
    while (wrapper.current < wrapper.end) {
        /* Access through structure member with zero offset */
        int val = *(wrapper.current + 0);
        sum += val;
        /* Increment structure member */
        wrapper.current++;
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Function 4: Store instead of load with decrement */
__attribute__((noinline))
static void test4_store_decrement(int *arr, int n, int value) {
    int *p = arr + n - 1;  /* Start from end */
    
    for (int i = 0; i < n; i++) {
        /* Store with zero offset */
        *(p + 0) = value + i;
        /* Separate decrement */
        p--;
    }
    
    /* Force memory side effects */
    asm volatile("" : : "r"(arr[0]) : "memory");
}

/* Function 5: Mixed pattern with conditional */
__attribute__((noinline))
static int test5_mixed_pattern(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Force zero offset through temporary */
        int *tmp = p;
        int val = *(tmp + 0);
        sum += val;
        
        /* Increment in separate statement */
        p = p + 1;
        
        /* Prevent loop unrolling with volatile check */
        if (i == g_volatile_bound) break;
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Function 6: Using asm to ensure memory access isn't eliminated */
__attribute__((noinline))
static int test6_asm_volatile(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Use asm to force memory access */
        int val;
        asm volatile("ldr %0, [%1, #0]" : "=r"(val) : "r"(p) : "memory");
        sum += val;
        
        /* Increment */
        p++;
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Main function that calls all test variants */
int main(int argc, char **argv) {
    /* Use argc to determine array size (non-constant) */
    int size = (argc > 1) ? atoi(argv[1]) : 100;
    if (size <= 0) size = 100;
    
    /* Allocate and initialize array */
    int *array = (int*)malloc(size * sizeof(int));
    if (!array) return 1;
    
    for (int i = 0; i < size; i++) {
        array[i] = i + 1;
    }
    
    int total_sum = 0;
    
    /* Call all test functions */
    total_sum += test1_zero_offset_plus_plus(array, size);
    total_sum += test2_array_zero_plus_equals(array, size);
    total_sum += test3_struct_member(array, size);
    
    /* Test store pattern */
    test4_store_decrement(array, size, 42);
    
    total_sum += test5_mixed_pattern(array, size);
    
    /* Only use asm version if compiling for ARM */
    #ifdef __arm__
    total_sum += test6_asm_volatile(array, size);
    #endif
    
    /* Use results to prevent dead code elimination */
    printf("Total sum: %d\n", total_sum);
    printf("Volatile sum: %d\n", g_volatile_sum);
    
    /* Verify array was modified by store test */
    int verify_sum = 0;
    for (int i = 0; i < size; i++) {
        verify_sum += array[i];
    }
    printf("Array sum after modifications: %d\n", verify_sum);
    
    free(array);
    return 0;
}

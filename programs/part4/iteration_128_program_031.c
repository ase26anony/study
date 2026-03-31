/* test-auto-inc-dec.c */
#include <stdio.h>
#include <stdlib.h>

/* External function to prevent elimination */
extern void dummy_external(void);

/* Volatile sink to prevent dead code elimination */
static volatile int volatile_sink;

/* Test function 1: Using *(p + 0) and p++ */
__attribute__((noinline))
int test1_zero_offset_plus_plus(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Memory access with explicit zero offset */
        int val = *(p + 0);
        sum += val;
        /* Separate pointer increment */
        p++;
    }
    
    volatile_sink = sum;
    return sum;
}

/* Test function 2: Using p[0] and p += 1 */
__attribute__((noinline))
int test2_array_zero_offset(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Array access with index 0 */
        int val = p[0];
        sum += val;
        /* Separate pointer increment with assignment */
        p += 1;
    }
    
    volatile_sink = sum;
    return sum;
}

/* Test function 3: Store instead of load with decrement */
__attribute__((noinline))
void test3_store_zero_offset_decrement(int *arr, int n, int value) {
    int *p = arr + n - 1;  /* Start from end */
    
    for (int i = 0; i < n; i++) {
        /* Store with zero offset */
        *(p + 0) = value + i;
        /* Separate pointer decrement */
        p--;
    }
    
    /* Use asm to ensure stores aren't eliminated */
    asm volatile("" : : "r"(arr[0]) : "memory");
}

/* Test function 4: Using pointer in struct */
struct ptr_wrapper {
    int *current;
    int *end;
};

__attribute__((noinline))
int test4_struct_pointer(struct ptr_wrapper *wrapper) {
    int sum = 0;
    
    while (wrapper->current != wrapper->end) {
        /* Access through struct member with zero offset */
        int val = *(wrapper->current + 0);
        sum += val;
        /* Increment struct member */
        wrapper->current++;
    }
    
    volatile_sink = sum;
    return sum;
}

/* Test function 5: Complex expression with zero offset */
__attribute__((noinline))
int test5_complex_zero_offset(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* More complex but still zero offset */
        int val = *(0 + p);
        sum += val;
        
        /* Increment with post-increment in separate statement */
        int *old = p;
        p = p + 1;
        
        /* Use old pointer to prevent optimization */
        asm volatile("" : : "r"(*old) : "memory");
    }
    
    volatile_sink = sum;
    return sum;
}

/* Test function 6: Byte access with char pointer */
__attribute__((noinline))
int test6_char_pointer(char *arr, int n) {
    char *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Char access with zero offset */
        char val = *(p + 0);
        sum += val;
        /* Pointer increment */
        p++;
    }
    
    volatile_sink = sum;
    return sum;
}

/* Main function that calls all tests */
int main(int argc, char **argv) {
    /* Use volatile to prevent constant propagation */
    volatile int size = (argc > 1) ? atoi(argv[1]) : 100;
    if (size <= 0) size = 100;
    
    /* Allocate array with runtime size */
    int *array = (int*)malloc(size * sizeof(int));
    if (!array) return 1;
    
    /* Initialize array */
    for (int i = 0; i < size; i++) {
        array[i] = i + 1;
    }
    
    int total_sum = 0;
    
    /* Run test 1 */
    total_sum += test1_zero_offset_plus_plus(array, size);
    
    /* Run test 2 */
    total_sum += test2_array_zero_offset(array, size);
    
    /* Run test 3 (stores) */
    test3_store_zero_offset_decrement(array, size, 42);
    
    /* Run test 4 with struct wrapper */
    struct ptr_wrapper wrapper;
    wrapper.current = array;
    wrapper.end = array + size;
    total_sum += test4_struct_pointer(&wrapper);
    
    /* Re-initialize for test 5 */
    for (int i = 0; i < size; i++) {
        array[i] = i + 1;
    }
    total_sum += test5_complex_zero_offset(array, size);
    
    /* Test 6 with char array */
    char *char_array = (char*)malloc(size);
    for (int i = 0; i < size; i++) {
        char_array[i] = (char)(i % 256);
    }
    total_sum += test6_char_pointer(char_array, size);
    
    /* Prevent dead code elimination */
    printf("Total sum: %d\n", total_sum);
    printf("Volatile sink: %d\n", volatile_sink);
    
    free(array);
    free(char_array);
    
    return 0;
}

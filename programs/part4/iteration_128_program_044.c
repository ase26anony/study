/* auto_inc_test.c - Test program for GCC auto-increment/decrement optimization */
#include <stdio.h>
#include <stdlib.h>

/* External function to prevent optimization */
extern void dummy_external(void);

/* Volatile variables to prevent constant propagation */
volatile int g_volatile_bound = 100;
volatile int g_volatile_sum = 0;

/* Test 1: Using *(p + 0) and p++ */
__attribute__((noinline))
int test1_ptr_plus_zero(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Memory access with explicit zero offset */
        int val = *(p + 0);
        sum += val;
        /* Separate pointer increment */
        p++;
        
        /* Prevent loop optimization */
        asm volatile("" : : "r"(val) : "memory");
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Test 2: Using p[0] and p += 1 */
__attribute__((noinline))
int test2_array_zero_index(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Array access with index 0 */
        int val = p[0];
        sum += val;
        /* Pointer increment with assignment */
        p += 1;
        
        /* Use volatile to prevent elimination */
        volatile int dummy = val;
        (void)dummy;
    }
    
    return sum;
}

/* Test 3: Store instead of load with p-- */
__attribute__((noinline))
void test3_store_and_decrement(int *arr, int n, int value) {
    int *p = arr + n - 1;  /* Start from end */
    
    for (int i = 0; i < n; i++) {
        /* Store with zero offset */
        *(p + 0) = value + i;
        /* Separate pointer decrement */
        p--;
        
        /* Memory barrier */
        asm volatile("" : : : "memory");
    }
}

/* Test 4: Structure with pointer member */
struct pointer_wrapper {
    int *ptr;
    int count;
};

__attribute__((noinline))
int test4_struct_member(struct pointer_wrapper *wrapper, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Access through structure pointer with zero offset */
        int val = *(wrapper->ptr + 0);
        sum += val;
        /* Increment the pointer in the structure */
        wrapper->ptr++;
        
        /* Call external function to prevent optimization */
        dummy_external();
    }
    
    return sum;
}

/* Test 5: Different increment pattern with byte pointer */
__attribute__((noinline))
int test5_byte_pointer(char *arr, int n) {
    char *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Char access with zero offset */
        char val = *(p + 0);
        sum += val;
        /* Increment by 1 (byte pointer) */
        p = p + 1;
    }
    
    return sum;
}

/* Test 6: Complex expression with zero offset */
__attribute__((noinline))
int test6_complex_expr(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* More complex zero offset expression */
        int val = *((int*)((char*)p + 0));
        sum += val;
        /* Post-increment in separate statement */
        p = p + 1;
        
        /* Prevent dead code elimination */
        if (val == 0x12345678) {
            dummy_external();
        }
    }
    
    return sum;
}

/* Dummy external function definition */
void dummy_external(void) {
    /* Empty but prevents optimization */
    asm volatile("" : : : "memory");
}

int main(int argc, char **argv) {
    /* Use argc to make array size non-constant */
    int array_size = g_volatile_bound;
    if (argc > 1) {
        array_size = atoi(argv[1]);
        if (array_size <= 0) array_size = 100;
    }
    
    /* Allocate and initialize array */
    int *array = (int*)malloc(array_size * sizeof(int));
    char *byte_array = (char*)malloc(array_size * sizeof(char));
    
    if (!array || !byte_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < array_size; i++) {
        array[i] = i * 3 + 1;
        byte_array[i] = (char)(i & 0xFF);
    }
    
    int total_sum = 0;
    
    /* Run all tests */
    total_sum += test1_ptr_plus_zero(array, array_size);
    total_sum += test2_array_zero_index(array, array_size);
    
    test3_store_and_decrement(array, array_size, 42);
    total_sum += test2_array_zero_index(array, array_size);  /* Re-read after store */
    
    /* Test with structure */
    struct pointer_wrapper wrapper;
    wrapper.ptr = array;
    wrapper.count = array_size;
    total_sum += test4_struct_member(&wrapper, array_size);
    
    total_sum += test5_byte_pointer(byte_array, array_size);
    total_sum += test6_complex_expr(array, array_size);
    
    /* Print result to prevent elimination */
    printf("Total sum: %d\n", total_sum);
    printf("Volatile sum: %d\n", g_volatile_sum);
    
    free(array);
    free(byte_array);
    
    return 0;
}

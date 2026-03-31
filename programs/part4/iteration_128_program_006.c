/* auto_inc_test.c - Test auto-increment/decrement optimization patterns */

#include <stdio.h>
#include <stdlib.h>

/* External function to prevent elimination */
extern void dummy_external(void);

/* Volatile variables to prevent constant propagation */
static volatile int volatile_bound = 100;
static volatile int volatile_sum = 0;

/* Function 1: Using *(p + 0) and p++ */
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
    
    /* Use volatile to prevent dead code elimination */
    volatile_sum += sum;
    return sum;
}

/* Function 2: Using p[0] and p += 1 */
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
    
    volatile_sum += sum;
    return sum;
}

/* Function 3: Structure with pointer member */
struct PointerHolder {
    int *ptr;
    int count;
};

__attribute__((noinline))
int test3_struct_member(struct PointerHolder *holder, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Access through structure pointer with zero offset */
        int val = *(holder->ptr + 0);
        sum += val;
        /* Increment the pointer member */
        holder->ptr = holder->ptr + 1;
    }
    
    volatile_sum += sum;
    return sum;
}

/* Function 4: Store instead of load with decrement */
__attribute__((noinline))
void test4_store_decrement(int *arr, int n, int value) {
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

/* Function 5: Mixed pattern with post-increment in expression */
__attribute__((noinline))
int test5_mixed_pattern(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Access with zero offset, increment in separate statement */
        int val = *(p + 0);
        sum += val;
        /* Different increment pattern */
        p = p + 1;
    }
    
    volatile_sum += sum;
    return sum;
}

/* Function 6: Using volatile to force memory access */
__attribute__((noinline))
int test6_volatile_access(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    volatile int *volatile_ptr = (volatile int *)p;
    
    for (int i = 0; i < n; i++) {
        /* Force memory access with inline asm */
        int val;
        asm volatile("mov %[val], %[ptr]\n\t"
                     : [val] "=r" (val)
                     : [ptr] "m" (*(p + 0))
                     : "memory");
        sum += val;
        p++;
    }
    
    volatile_sum += sum;
    return sum;
}

/* Function 7: Nested loops to create complex pattern */
__attribute__((noinline))
int test7_nested_loops(int *arr, int rows, int cols) {
    int sum = 0;
    int *p = arr;
    
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            /* Zero offset access in inner loop */
            sum += *(p + 0);
            p++;
        }
    }
    
    volatile_sum += sum;
    return sum;
}

/* Main function to drive tests */
int main(int argc, char *argv[]) {
    /* Use argc to create runtime-determined size */
    int size = (argc > 1) ? atoi(argv[1]) : 100;
    if (size <= 0) size = 100;
    
    /* Allocate array with runtime size */
    int *array = (int *)malloc(size * sizeof(int));
    if (!array) return 1;
    
    /* Initialize array */
    for (int i = 0; i < size; i++) {
        array[i] = i + 1;
    }
    
    int total_sum = 0;
    
    /* Test 1: Basic zero offset with post-increment */
    total_sum += test1_zero_offset_plus_plus(array, size);
    
    /* Test 2: Array notation */
    total_sum += test2_array_zero_offset(array, size);
    
    /* Test 3: Structure member */
    struct PointerHolder holder;
    holder.ptr = array;
    holder.count = size;
    total_sum += test3_struct_member(&holder, size);
    
    /* Test 4: Store pattern */
    test4_store_decrement(array, size, 42);
    
    /* Test 5: Mixed pattern */
    total_sum += test5_mixed_pattern(array, size);
    
    /* Test 6: Volatile access */
    total_sum += test6_volatile_access(array, size);
    
    /* Test 7: Nested loops */
    int rows = 10;
    int cols = size / 10;
    if (cols < 1) cols = 1;
    total_sum += test7_nested_loops(array, rows, cols);
    
    /* Print result to prevent elimination */
    printf("Total sum: %d\n", total_sum);
    printf("Volatile sum: %d\n", volatile_sum);
    
    free(array);
    return 0;
}

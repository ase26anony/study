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
int test1_zero_offset_plus_plus(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Memory access with explicit zero offset */
        int val = *(p + 0);
        
        /* Separate pointer increment */
        p++;
        
        /* Use the value to prevent elimination */
        sum += val;
        asm volatile("" : : "r"(val) : "memory");
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Test 2: Using p[0] and p += 1 */
__attribute__((noinline))
int test2_array_zero_offset(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Array access with index 0 */
        int val = p[0];
        
        /* Separate pointer increment with assignment */
        p += 1;
        
        sum += val;
        /* Use inline asm to prevent optimization */
        __asm__ volatile ("# dummy" : : "r"(val));
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Test 3: Using structure with pointer member */
struct PointerHolder {
    int *ptr;
    int count;
};

__attribute__((noinline))
int test3_struct_member(struct PointerHolder *holder, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Access through structure member with zero offset */
        int val = *(holder->ptr + 0);
        
        /* Increment the pointer member */
        holder->ptr = holder->ptr + 1;
        
        sum += val;
        /* Force memory barrier */
        asm volatile("" : : : "memory");
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Test 4: Store operation with *(p + 0) and p-- */
__attribute__((noinline))
int test4_store_decrement(int *arr, int n, int value) {
    int *p = arr + n - 1;  /* Start from end */
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Store with zero offset */
        *(p + 0) = value + i;
        
        /* Separate pointer decrement */
        p--;
        
        /* Read back to ensure store happens */
        sum += arr[n - 1 - i];
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Test 5: More complex zero offset expression */
__attribute__((noinline))
int test5_complex_zero_offset(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Zero offset with explicit cast */
        int val = *((int *)((char *)p + 0));
        
        /* Increment with post-increment in separate statement */
        p = p + 1;
        
        sum += val;
        /* Call dummy external to prevent optimization */
        dummy_external();
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Test 6: Nested loops to expose pattern in different contexts */
__attribute__((noinline))
int test6_nested_loops(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    int outer = n / 4;
    
    for (int j = 0; j < outer; j++) {
        for (int k = 0; k < 4; k++) {
            /* Access with zero offset */
            int val = *(p + 0);
            
            /* Pointer increment */
            p++;
            
            sum += val;
        }
        /* Some computation to prevent loop fusion */
        asm volatile("" : : : "memory");
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Dummy external function implementation */
void dummy_external(void) {
    /* Empty but prevents optimization */
    asm volatile("" : : : "memory");
}

int main(int argc, char *argv[]) {
    /* Use argc to make array size non-constant */
    int size = g_volatile_bound;
    if (argc > 1) {
        size = atoi(argv[1]);
        if (size <= 0) size = 100;
    }
    
    /* Allocate and initialize array */
    int *array = (int *)malloc(size * sizeof(int));
    if (!array) return 1;
    
    for (int i = 0; i < size; i++) {
        array[i] = i + 1;
    }
    
    int total_sum = 0;
    
    /* Run all test functions */
    total_sum += test1_zero_offset_plus_plus(array, size);
    total_sum += test2_array_zero_offset(array, size);
    
    struct PointerHolder holder = {array, size};
    total_sum += test3_struct_member(&holder, size);
    
    /* Reset array for store test */
    for (int i = 0; i < size; i++) {
        array[i] = 0;
    }
    total_sum += test4_store_decrement(array, size, 10);
    
    /* Re-initialize array */
    for (int i = 0; i < size; i++) {
        array[i] = i + 1;
    }
    total_sum += test5_complex_zero_offset(array, size);
    total_sum += test6_nested_loops(array, size);
    
    /* Print result to prevent dead code elimination */
    printf("Total sum: %d\n", total_sum);
    printf("Volatile sum: %d\n", g_volatile_sum);
    
    free(array);
    return 0;
}

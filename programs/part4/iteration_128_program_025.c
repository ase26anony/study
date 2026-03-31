/* auto_inc_test.c - Test program for GCC auto-increment/decrement optimization */
#include <stdio.h>
#include <stdlib.h>

/* External dummy function to prevent elimination */
extern void dummy_external(int);

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
        
        /* Separate pointer increment */
        p++;
        
        /* Use the value to prevent elimination */
        sum += val;
        asm volatile("" : : "r"(val)); /* Prevent optimization */
    }
    
    return sum;
}

/* Test 2: Using p[0] and p += 1 */
__attribute__((noinline))
int test2_array_zero(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Array access with index 0 */
        int val = p[0];
        
        /* Separate pointer increment with assignment */
        p += 1;
        
        /* Use value and prevent optimization */
        sum += val;
        dummy_external(val);
    }
    
    return sum;
}

/* Test 3: Store instead of load with decrement */
__attribute__((noinline))
void test3_store_decrement(int *arr, int n, int value) {
    int *p = &arr[n - 1]; /* Start from end */
    
    for (int i = 0; i < n; i++) {
        /* Store with zero offset */
        *(p + 0) = value + i;
        
        /* Separate pointer decrement */
        p--;
        
        /* Memory barrier to prevent reordering */
        asm volatile("" : : : "memory");
    }
}

/* Test 4: Structure with pointer member */
typedef struct {
    int *current;
    int *end;
} PointerStruct;

__attribute__((noinline))
int test4_struct_member(PointerStruct *ps) {
    int sum = 0;
    int *p = ps->current;
    
    while (p != ps->end) {
        /* Access through structure pointer */
        int val = *(p + 0);
        
        /* Increment the pointer */
        p = p + 1;
        
        sum += val;
        
        /* Prevent optimization */
        g_volatile_sum = val;
    }
    
    ps->current = p;
    return sum;
}

/* Test 5: Different increment patterns in same function */
__attribute__((noinline))
int test5_mixed_patterns(int *arr, int n) {
    int *p1 = arr;
    int *p2 = &arr[n/2];
    int sum = 0;
    
    /* First half: increment after access */
    for (int i = 0; i < n/2; i++) {
        sum += *(p1 + 0);
        p1++;  /* Post-increment */
    }
    
    /* Second half: increment before access (different pattern) */
    for (int i = 0; i < n/2; i++) {
        p2 = p2 + 1;  /* Pre-increment form */
        sum += *(p2 + 0);
    }
    
    return sum;
}

/* Test 6: Byte access with char pointer */
__attribute__((noinline))
int test6_char_pointer(char *arr, int n) {
    char *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Char access with zero offset */
        sum += (int)*(p + 0);
        
        /* Pointer increment */
        p = p + 1;
    }
    
    return sum;
}

/* Dummy external function implementation */
void dummy_external(int x) {
    /* Empty but prevents dead code elimination */
    asm volatile("" : : "r"(x));
}

int main(int argc, char **argv) {
    /* Use argc to determine array size at runtime */
    int size = (argc > 1) ? atoi(argv[1]) : g_volatile_bound;
    if (size <= 0) size = 100;
    
    /* Allocate and initialize array */
    int *array = (int*)malloc(size * sizeof(int));
    char *char_array = (char*)malloc(size * sizeof(char));
    
    for (int i = 0; i < size; i++) {
        array[i] = i + 1;
        char_array[i] = (char)(i % 256);
    }
    
    int total_sum = 0;
    
    /* Run all tests */
    total_sum += test1_ptr_plus_zero(array, size);
    total_sum += test2_array_zero(array, size);
    
    test3_store_decrement(array, size, 42);
    
    PointerStruct ps = {array, array + size};
    total_sum += test4_struct_member(&ps);
    
    total_sum += test5_mixed_patterns(array, size);
    total_sum += test6_char_pointer(char_array, size);
    
    /* Print result to prevent elimination */
    printf("Total sum: %d\n", total_sum);
    
    free(array);
    free(char_array);
    
    return 0;
}

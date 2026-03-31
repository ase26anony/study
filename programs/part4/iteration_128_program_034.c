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
        asm volatile("" : : "r"(val)); /* Prevent dead code elimination */
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
        
        /* Separate pointer increment */
        p += 1;
        
        sum += val;
        /* Use inline asm to ensure access isn't optimized away */
        __asm__ volatile ("# dummy" : : "r"(val));
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Test 3: Store instead of load with p-- */
__attribute__((noinline))
void test3_store_zero_offset_decrement(int *arr, int n, int value) {
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

/* Test 4: Using structure with pointer member */
struct PointerHolder {
    int *ptr;
    int count;
};

__attribute__((noinline))
int test4_struct_member_access(struct PointerHolder *holder, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Access through structure member with zero offset */
        int val = *(holder->ptr + 0);
        
        /* Increment the pointer member */
        holder->ptr = holder->ptr + 1;
        
        sum += val;
        /* Prevent optimization */
        if (val) dummy_external();
    }
    
    g_volatile_sum += sum;
    return sum;
}

/* Test 5: Different increment pattern with byte pointer */
__attribute__((noinline))
int test5_char_pointer_zero_offset(char *arr, int n) {
    char *p = arr;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Char access with zero offset */
        char val = *(p + 0);
        
        /* Separate increment */
        p = p + 1;
        
        sum += (int)val;
        /* Volatile write to prevent elimination */
        *(volatile char *)p = val;
    }
    
    return sum;
}

/* Test 6: Post-increment in different statement */
__attribute__((noinline))
int test6_separate_post_increment(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    int *temp;
    
    for (int i = 0; i < n; i++) {
        /* Load with zero offset using temporary */
        temp = p + 0;
        int val = *temp;
        
        /* Post-increment in separate statement */
        p = p + 1;
        
        sum += val;
        /* Use value in volatile context */
        g_volatile_sum = val;
    }
    
    return sum;
}

/* Dummy external function implementation */
void dummy_external(void) {
    /* Empty but prevents optimization */
    asm volatile("");
}

int main(int argc, char **argv) {
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
    
    test3_store_zero_offset_decrement(array, size, 42);
    
    struct PointerHolder holder;
    holder.ptr = array;
    holder.count = size;
    total_sum += test4_struct_member_access(&holder, size);
    
    char *char_array = (char *)malloc(size);
    for (int i = 0; i < size; i++) {
        char_array[i] = (char)(i % 256);
    }
    total_sum += test5_char_pointer_zero_offset(char_array, size);
    
    total_sum += test6_separate_post_increment(array, size);
    
    /* Print result to prevent dead code elimination */
    printf("Total sum: %d\n", total_sum);
    printf("Volatile sum: %d\n", g_volatile_sum);
    
    free(array);
    free(char_array);
    
    return 0;
}

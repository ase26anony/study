/* test-auto-inc-dec.c */
#include <stdio.h>
#include <stdlib.h>

/* External function to prevent optimization */
extern void use_value(int val);

/* Volatile variables to prevent constant propagation */
volatile int g_volatile_bound = 100;
volatile int* g_volatile_ptr = NULL;

/* Test function 1: Using *(p + 0) and p++ */
__attribute__((noinline))
int test1_zero_offset_plus_plus(int* arr, int n) {
    int sum = 0;
    int *p = arr;
    
    for (int i = 0; i < n; i++) {
        /* Memory access with explicit zero offset */
        int val = *(p + 0);
        sum += val;
        /* Separate pointer increment */
        p++;
    }
    
    /* Use result to prevent elimination */
    asm volatile("" : : "r"(sum));
    return sum;
}

/* Test function 2: Using p[0] and p += 1 */
__attribute__((noinline))
int test2_array_zero_offset(int* arr, int n) {
    int sum = 0;
    int *p = arr;
    
    for (int i = 0; i < n; i++) {
        /* Array access with index 0 */
        int val = p[0];
        sum += val;
        /* Separate pointer increment with assignment */
        p += 1;
    }
    
    asm volatile("" : : "r"(sum));
    return sum;
}

/* Test function 3: Using structure with pointer member */
struct PointerHolder {
    int* ptr;
    int count;
};

__attribute__((noinline))
int test3_struct_member(int* arr, int n) {
    struct PointerHolder holder;
    holder.ptr = arr;
    holder.count = n;
    
    int sum = 0;
    for (int i = 0; i < holder.count; i++) {
        /* Access through structure member with zero offset */
        int val = *(holder.ptr + 0);
        sum += val;
        /* Increment structure member */
        holder.ptr = holder.ptr + 1;
    }
    
    asm volatile("" : : "r"(sum));
    return sum;
}

/* Test function 4: Store instead of load */
__attribute__((noinline))
void test4_store_zero_offset(int* arr, int n, int value) {
    int *p = arr;
    
    for (int i = 0; i < n; i++) {
        /* Store with zero offset */
        *(p + 0) = value + i;
        /* Decrement pointer */
        p--;
    }
    
    /* Force memory writes */
    asm volatile("" : : "m"(arr[0]));
}

/* Test function 5: More complex zero offset expression */
__attribute__((noinline))
int test5_complex_zero(int* arr, int n) {
    int sum = 0;
    int *p = arr;
    
    for (int i = 0; i < n; i++) {
        /* Zero offset with explicit cast and arithmetic */
        int val = *((int*)((char*)p + 0));
        sum += val;
        /* Pointer increment by 1 */
        p = p + 1;
    }
    
    asm volatile("" : : "r"(sum));
    return sum;
}

/* Test function 6: Mixed increment/decrement */
__attribute__((noinline))
int test6_mixed_inc_dec(int* arr, int n) {
    int sum = 0;
    int *p = arr;
    int *end = arr + n;
    
    while (p < end) {
        /* Load with zero offset */
        int val = *(p + 0);
        sum += val;
        
        /* Conditional increment */
        if (val > 0) {
            p++;
        } else {
            p += 1;  /* Still increment by 1, just different syntax */
        }
    }
    
    asm volatile("" : : "r"(sum));
    return sum;
}

/* Test function 7: Nested loops with pointer reset */
__attribute__((noinline))
int test7_nested_loops(int* arr, int n, int m) {
    int total = 0;
    
    for (int j = 0; j < m; j++) {
        int *p = arr;
        int sum = 0;
        
        for (int i = 0; i < n; i++) {
            /* Access with zero offset */
            sum += *(p + 0);
            /* Increment pointer */
            ++p;
        }
        
        total += sum;
    }
    
    asm volatile("" : : "r"(total));
    return total;
}

/* Main function that exercises all test cases */
int main(int argc, char** argv) {
    /* Use argc to make array size non-constant */
    int size = (argc > 1) ? atoi(argv[1]) : 100;
    if (size <= 0) size = 100;
    
    /* Allocate array with dynamic size */
    int* array = (int*)malloc(size * sizeof(int));
    if (!array) return 1;
    
    /* Initialize array */
    for (int i = 0; i < size; i++) {
        array[i] = i + 1;
    }
    
    /* Set volatile pointer to prevent optimizations */
    g_volatile_ptr = array;
    
    int total_sum = 0;
    
    /* Call all test functions */
    total_sum += test1_zero_offset_plus_plus(array, size);
    total_sum += test2_array_zero_offset(array, size);
    total_sum += test3_struct_member(array, size);
    
    /* Test store pattern */
    test4_store_zero_offset(array + size - 1, size, 42);
    
    total_sum += test5_complex_zero(array, size);
    total_sum += test6_mixed_inc_dec(array, size);
    total_sum += test7_nested_loops(array, size, 3);
    
    /* Use volatile bound to prevent loop unrolling */
    int volatile_size = g_volatile_bound;
    if (volatile_size > size) volatile_size = size;
    
    /* One more test with volatile bound */
    int *volatile_ptr = g_volatile_ptr;
    int volatile_sum = 0;
    for (int i = 0; i < volatile_size; i++) {
        volatile_sum += *(volatile_ptr + 0);
        volatile_ptr++;
    }
    total_sum += volatile_sum;
    
    printf("Total sum: %d\n", total_sum);
    
    free(array);
    return 0;
}

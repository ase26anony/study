/* Compile with: gcc -O2 -fno-omit-frame-pointer -march=armv7-a -fno-strict-aliasing -c auto_inc_test.c */

#include <stdio.h>
#include <stdlib.h>

/* External function to prevent elimination */
extern void use_value(int val);

/* Volatile variables to prevent constant propagation */
volatile int g_volatile_bound = 100;
volatile int* g_volatile_ptr = NULL;

/* Test 1: Using *(p + 0) and p++ */
__attribute__((noinline))
int test1_zero_offset_plus_plus(int* arr, int n) {
    int sum = 0;
    int* p = arr;
    
    for (int i = 0; i < n; i++) {
        /* Memory access with explicit zero offset */
        int val = *(p + 0);
        sum += val;
        /* Separate increment statement */
        p++;
    }
    
    /* Use result to prevent elimination */
    asm volatile("" : : "r"(sum) : "memory");
    return sum;
}

/* Test 2: Using p[0] and p += 1 */
__attribute__((noinline))
int test2_array_zero_plus_equals(int* arr, int n) {
    int sum = 0;
    int* p = arr;
    
    for (int i = 0; i < n; i++) {
        /* Array access with index 0 */
        int val = p[0];
        sum += val;
        /* Separate increment with += */
        p += 1;
    }
    
    asm volatile("" : : "r"(sum) : "memory");
    return sum;
}

/* Test 3: Structure with pointer member */
struct PointerHolder {
    int* ptr;
    int count;
};

__attribute__((noinline))
int test3_struct_member(struct PointerHolder* holder, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Access through structure member with zero offset */
        int val = *(holder->ptr + 0);
        sum += val;
        /* Increment the pointer member */
        holder->ptr = holder->ptr + 1;
    }
    
    asm volatile("" : : "r"(sum) : "memory");
    return sum;
}

/* Test 4: Store instead of load with decrement */
__attribute__((noinline))
void test4_store_zero_decrement(int* arr, int n, int value) {
    int* p = arr + n - 1; /* Start from end */
    
    for (int i = 0; i < n; i++) {
        /* Store with zero offset */
        *(p + 0) = value + i;
        /* Separate decrement */
        p--;
    }
    
    /* Memory barrier */
    asm volatile("" : : : "memory");
}

/* Test 5: Different type (char) to test different memory modes */
__attribute__((noinline))
int test5_char_zero_offset(char* arr, int n) {
    int sum = 0;
    char* p = arr;
    
    for (int i = 0; i < n; i++) {
        /* Char access with zero offset */
        char val = *(p + 0);
        sum += val;
        /* Increment */
        p = p + 1;
    }
    
    asm volatile("" : : "r"(sum) : "memory");
    return sum;
}

/* Test 6: Using volatile to force memory access */
__attribute__((noinline))
int test6_volatile_base(int n) {
    int sum = 0;
    /* Get base from volatile to prevent optimization */
    int* p = (int*)g_volatile_ptr;
    
    if (!p) return 0;
    
    for (int i = 0; i < n; i++) {
        /* Force memory access with inline asm */
        int val;
        asm volatile("ldr %0, [%1, #0]" : "=r"(val) : "r"(p) : "memory");
        sum += val;
        p++;
    }
    
    return sum;
}

/* Test 7: Nested pointer arithmetic */
__attribute__((noinline))
int test7_nested_zero(int* arr, int n) {
    int sum = 0;
    int** pp = &arr;
    
    for (int i = 0; i < n; i++) {
        /* Double dereference with zero offset */
        int val = *(*pp + 0);
        sum += val;
        /* Increment the pointed-to pointer */
        (*pp)++;
    }
    
    asm volatile("" : : "r"(sum) : "memory");
    return sum;
}

/* Main function that calls all tests */
int main(int argc, char** argv) {
    /* Use argc to make array size non-constant */
    int size = g_volatile_bound;
    if (argc > 1) {
        size = atoi(argv[1]);
        if (size <= 0) size = 100;
    }
    
    /* Allocate and initialize array */
    int* array = (int*)malloc(size * sizeof(int));
    if (!array) return 1;
    
    for (int i = 0; i < size; i++) {
        array[i] = i + 1;
    }
    
    /* Set volatile pointer */
    g_volatile_ptr = array;
    
    int total_sum = 0;
    
    /* Run all tests */
    total_sum += test1_zero_offset_plus_plus(array, size);
    total_sum += test2_array_zero_plus_equals(array, size);
    
    struct PointerHolder holder;
    holder.ptr = array;
    holder.count = size;
    total_sum += test3_struct_member(&holder, size);
    
    test4_store_zero_decrement(array, size, 42);
    
    /* Test with char array */
    char* char_array = (char*)malloc(size * sizeof(char));
    for (int i = 0; i < size; i++) {
        char_array[i] = (char)(i % 256);
    }
    total_sum += test5_char_zero_offset(char_array, size);
    
    total_sum += test6_volatile_base(size);
    total_sum += test7_nested_zero(array, size);
    
    /* Print result to prevent elimination */
    printf("Total sum: %d\n", total_sum);
    
    free(array);
    free(char_array);
    
    return 0;
}

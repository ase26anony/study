/* auto-inc-dec-test.c */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define ARRAY_SIZE 1024
#define STRUCT_COUNT 256

/* Simple structure for nested access */
typedef struct {
    int32_t field1;
    int32_t field2;
    int32_t field3;
} TestStruct;

/* Global arrays to ensure they're addressable */
static int32_t int_array[ARRAY_SIZE];
static TestStruct struct_array[STRUCT_COUNT];

/* ========== Test Functions ========== */

/* Function A: Forward traversal with ptr++ - likely to be inlined */
static int32_t sum_array_forward(int32_t* arr, size_t n) {
    int32_t sum = 0;
    int32_t* ptr = arr;
    int32_t* end = arr + n;
    
    /* Pattern: ptr++ in loop condition */
    while (ptr < end) {
        sum += *ptr++;
    }
    return sum;
}

/* Function B: Backward traversal with ptr-- - noinline to test boundaries */
__attribute__((noinline))
static int32_t process_array_backward(int32_t* arr, size_t n) {
    int32_t result = 0;
    int32_t* ptr = arr + n - 1;
    
    /* Pattern: *ptr-- with explicit decrement */
    for (size_t i = 0; i < n; i++) {
        result ^= *ptr;
        ptr--;  /* Separate decrement - should create reg update pattern */
    }
    return result;
}

/* Function C: Structure field access with mixed patterns */
static int32_t sum_struct_fields(TestStruct* arr, size_t n) {
    int32_t total = 0;
    TestStruct* ptr = arr;
    
    /* Access fields with constant offsets within structure */
    for (size_t i = 0; i < n; i++) {
        /* Multiple accesses to same base pointer with constant offsets */
        total += ptr->field1;  /* offset 0 */
        total += ptr->field2;  /* offset 4 */
        total += ptr->field3;  /* offset 8 */
        ptr++;  /* Increment by structure size */
    }
    return total;
}

/* Function D: Volatile pointer walk - inhibits some optimizations */
__attribute__((noinline))
static int32_t volatile_walk(volatile int32_t* arr, size_t n) {
    int32_t sum = 0;
    volatile int32_t* ptr = arr;
    
    /* Volatile access pattern */
    for (size_t i = 0; i < n; i++) {
        sum += *ptr;
        ptr = ptr + 1;  /* Separate assignment to create reg update */
    }
    return sum;
}

/* Function E: Complex pattern with multiple pointer updates */
static void transform_array(int32_t* src, int32_t* dst, size_t n) {
    int32_t* s = src;
    int32_t* d = dst;
    
    /* Mixed patterns in same loop */
    for (size_t i = 0; i < n; i++) {
        /* Read with post-increment */
        int32_t val = *s++;
        
        /* Modify and write with post-increment */
        *d++ = val * 2 + 1;
    }
}

/* Function F: Nested loop with stride */
static int32_t matrix_like_access(int32_t* arr, size_t rows, size_t cols) {
    int32_t sum = 0;
    
    for (size_t i = 0; i < rows; i++) {
        int32_t* row_ptr = arr + i * cols;
        
        /* Inner loop with pointer increment */
        for (size_t j = 0; j < cols; j++) {
            sum += *row_ptr++;
        }
    }
    return sum;
}

/* Function G: Pointer comparison as loop condition */
static int32_t find_value(int32_t* arr, size_t n, int32_t target) {
    int32_t* ptr = arr;
    int32_t* end = arr + n;
    int32_t count = 0;
    
    /* while (ptr < end) pattern */
    while (ptr < end) {
        if (*ptr == target) {
            count++;
        }
        ptr++;  /* Increment after use */
    }
    return count;
}

/* Function H: Mixed volatile and non-volatile in same function */
static int32_t mixed_volatile_access(int32_t* regular, volatile int32_t* vol, size_t n) {
    int32_t sum = 0;
    
    for (size_t i = 0; i < n; i++) {
        /* Non-volatile access with potential auto-inc */
        sum += regular[i];
        
        /* Volatile access - separate pointer arithmetic */
        sum += *vol;
        vol = vol + 1;
    }
    return sum;
}

/* ========== Main Test Driver ========== */
int main(void) {
    int32_t result = 0;
    
    /* Initialize arrays */
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = (int32_t)(i % 100);
    }
    
    for (size_t i = 0; i < STRUCT_COUNT; i++) {
        struct_array[i].field1 = (int32_t)i;
        struct_array[i].field2 = (int32_t)(i * 2);
        struct_array[i].field3 = (int32_t)(i * 3);
    }
    
    /* Create destination array for transform */
    int32_t dest_array[ARRAY_SIZE];
    
    /* Execute all test patterns */
    
    /* 1. Forward traversal with ptr++ */
    result += sum_array_forward(int_array, ARRAY_SIZE);
    
    /* 2. Backward traversal */
    result += process_array_backward(int_array, ARRAY_SIZE);
    
    /* 3. Structure field access */
    result += sum_struct_fields(struct_array, STRUCT_COUNT);
    
    /* 4. Volatile walk */
    volatile int32_t volatile_copy[ARRAY_SIZE];
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        volatile_copy[i] = int_array[i];
    }
    result += volatile_walk(volatile_copy, ARRAY_SIZE / 2);
    
    /* 5. Transform with multiple pointers */
    transform_array(int_array, dest_array, ARRAY_SIZE);
    result += sum_array_forward(dest_array, ARRAY_SIZE / 4);
    
    /* 6. Matrix-like access */
    result += matrix_like_access(int_array, 32, 32);  /* 32x32 = 1024 */
    
    /* 7. Pointer comparison loop */
    result += find_value(int_array, ARRAY_SIZE, 42);
    
    /* 8. Mixed volatile/non-volatile */
    result += mixed_volatile_access(int_array, volatile_copy, ARRAY_SIZE / 8);
    
    /* Additional pressure: create register pressure with live values */
    int32_t temp = result;
    for (int i = 0; i < 100; i++) {
        temp = (temp * 1103515245 + 12345) & 0x7fffffff;
    }
    result ^= temp;
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return 0;
}

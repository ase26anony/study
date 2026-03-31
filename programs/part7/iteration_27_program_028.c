#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define ARRAY_SIZE 1024
#define STRUCT_COUNT 512

/* Structure for nested access patterns */
typedef struct {
    int32_t field1;
    int32_t field2;
    int32_t field3;
    int32_t field4;
} DataStruct;

/* Global arrays to ensure they're not optimized away */
static int32_t int_array[ARRAY_SIZE];
static DataStruct struct_array[STRUCT_COUNT];

/* NOINLINE functions to prevent inlining */
__attribute__((noinline)) 
static void test_noinline_forward(int32_t *arr, int size, int32_t *result) {
    int32_t sum = 0;
    int32_t *ptr = arr;
    int32_t *end = arr + size;
    
    /* Pattern 1: while loop with pointer comparison */
    while (ptr < end) {
        sum += *ptr;
        ptr = ptr + 1;  /* Force base register update */
    }
    
    /* Pattern 2: Another loop in same function */
    ptr = arr;
    int i;
    for (i = 0; i < size; i++) {
        sum += *ptr;
        ptr++;  /* Post-increment pattern */
    }
    
    *result = sum;
}

/* Likely to be inlined */
static int32_t test_inline_backward(int32_t *arr, int size) {
    int32_t sum = 0;
    int32_t *ptr = arr + size - 1;
    
    /* Backward traversal with pre-decrement */
    while (ptr >= arr) {
        sum += *ptr;
        ptr--;  /* Should create reg0 update with constant offset */
    }
    
    return sum;
}

/* Mixed volatile and non-volatile access */
__attribute__((noinline))
static void test_volatile_mixed(volatile int32_t *varr, int32_t *normal_arr, int size) {
    volatile int32_t *vptr = varr;
    int32_t *nptr = normal_arr;
    int i;
    
    /* Volatile access pattern */
    for (i = 0; i < size; i++) {
        *nptr = *vptr;  /* Load from volatile, store to normal */
        vptr = vptr + 1;  /* Base register update */
        nptr = nptr + 1;
    }
    
    /* Another pattern with different stride */
    vptr = varr;
    for (i = 0; i < size; i += 2) {
        int32_t temp = *vptr;
        vptr = vptr + 2;  /* Constant stride of 2 */
    }
}

/* Structure field access with pointer arithmetic */
static int32_t test_struct_access(DataStruct *arr, int count) {
    int32_t total = 0;
    DataStruct *ptr = arr;
    DataStruct *end = arr + count;
    
    /* Access structure fields with constant offsets */
    while (ptr < end) {
        /* Multiple field accesses with same base pointer */
        total += ptr->field1;
        total += ptr->field2;
        total += ptr->field3;
        total += ptr->field4;
        
        ptr = ptr + 1;  /* Base register update by structure size */
    }
    
    /* Alternative: array indexing with structure field access */
    int i;
    for (i = 0; i < count; i++) {
        total += arr[i].field1;
        total += arr[i].field2;
    }
    
    return total;
}

/* Complex pattern with multiple pointer updates */
__attribute__((noinline))
static void test_complex_pattern(int32_t *arr1, int32_t *arr2, int size, int32_t *out) {
    int32_t *p1 = arr1;
    int32_t *p2 = arr2;
    int32_t sum1 = 0, sum2 = 0;
    int i;
    
    /* Interleaved pointer updates */
    for (i = 0; i < size; i++) {
        sum1 += *p1;
        p1 = p1 + 1;  /* Base register update instruction */
        
        sum2 += *p2;
        p2 = p2 + 1;  /* Another base register update */
        
        /* Force register pressure */
        sum1 ^= sum2;
        sum2 ^= sum1;
        sum1 ^= sum2;
    }
    
    /* Additional pattern with immediate reuse */
    p1 = arr1;
    int32_t val = *p1;  /* Load with base register */
    p1 = p1 + 1;        /* Update base register */
    val += *p1;         /* Another load with updated base */
    
    *out = sum1 + sum2 + val;
}

/* Main test driver */
int main(void) {
    int32_t result1 = 0, result2 = 0, result3 = 0, result4 = 0;
    volatile int32_t volatile_array[ARRAY_SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i;
        volatile_array[i] = i * 2;
    }
    
    for (int i = 0; i < STRUCT_COUNT; i++) {
        struct_array[i].field1 = i;
        struct_array[i].field2 = i * 2;
        struct_array[i].field3 = i * 3;
        struct_array[i].field4 = i * 4;
    }
    
    /* Test 1: Noinline forward traversal */
    test_noinline_forward(int_array, ARRAY_SIZE, &result1);
    
    /* Test 2: Inline backward traversal */
    result2 = test_inline_backward(int_array, ARRAY_SIZE);
    
    /* Test 3: Mixed volatile/non-volatile */
    test_volatile_mixed(volatile_array, int_array, ARRAY_SIZE / 2);
    
    /* Test 4: Structure access */
    result3 = test_struct_access(struct_array, STRUCT_COUNT);
    
    /* Test 5: Complex pattern */
    test_complex_pattern(int_array, int_array + ARRAY_SIZE/2, ARRAY_SIZE/4, &result4);
    
    /* Additional patterns in main to increase coverage */
    {
        /* Direct pointer arithmetic with constant stride */
        int32_t *ptr = int_array;
        int32_t local_sum = 0;
        
        for (int i = 0; i < 16; i++) {
            local_sum += *ptr;
            ptr = ptr + 4;  /* Stride of 4 */
        }
        
        result1 += local_sum;
    }
    
    /* Nested loop with pointer reset */
    {
        DataStruct *sptr = struct_array;
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                result3 += sptr->field1;
                sptr = sptr + 1;  /* Inner loop pointer update */
            }
            sptr = struct_array;  /* Reset pointer */
        }
    }
    
    /* Final aggregation to prevent dead code elimination */
    int32_t final_result = result1 + result2 + result3 + result4;
    
    /* Use the result */
    printf("Result: %d\n", final_result);
    
    return (final_result > 0) ? 0 : 1;
}

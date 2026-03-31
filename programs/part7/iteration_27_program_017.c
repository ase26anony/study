/* auto-inc-dec-test.c */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define ARRAY_SIZE 1024
#define STRUCT_COUNT 256

/* Structure for nested access patterns */
typedef struct {
    int32_t field1;
    int32_t field2;
    int32_t field3;
    int32_t field4;
} TestStruct;

/* Global arrays for predictable addressing */
static int32_t int_array[ARRAY_SIZE];
static TestStruct struct_array[STRUCT_COUNT];

/* ========== INLINEABLE HELPERS ========== */

/* Forward traversal with ptr++ pattern - likely to be inlined */
static int32_t sum_with_ptr_increment(int32_t* arr, size_t n) {
    int32_t sum = 0;
    int32_t* ptr = arr;
    int32_t* end = arr + n;
    
    /* Pattern: ptr++ in loop condition */
    while (ptr < end) {
        sum += *ptr++;
    }
    return sum;
}

/* Backward traversal with ptr-- pattern */
static int32_t sum_with_ptr_decrement(int32_t* arr, size_t n) {
    int32_t sum = 0;
    int32_t* ptr = arr + n - 1;
    
    /* Pattern: ptr-- in loop */
    for (size_t i = 0; i < n; i++) {
        sum += *ptr--;
    }
    return sum;
}

/* Mixed base register update pattern */
static void process_struct_fields(TestStruct* arr, size_t n, int32_t* results) {
    TestStruct* ptr = arr;
    
    /* Pattern: base register update with constant stride */
    for (size_t i = 0; i < n; i++) {
        /* Multiple accesses to force register pressure */
        results[0] += ptr->field1;
        results[1] += ptr->field2;
        results[2] += ptr->field3;
        results[3] += ptr->field4;
        
        /* Explicit pointer increment - creates separate instruction */
        ptr = ptr + 1;
    }
}

/* Complex addressing with array index */
static int32_t sum_with_mixed_addressing(int32_t* arr, size_t n) {
    int32_t sum = 0;
    int32_t* base_ptr = arr;
    
    /* Pattern: mixed pointer and index arithmetic */
    for (size_t i = 0; i < n; i++) {
        /* This creates: reg0 = base_ptr, reg1_val = i*4 */
        sum += base_ptr[i];
        
        /* Force register update pattern */
        if (i % 8 == 0) {
            /* This might create the mem_insn pattern we want */
            int32_t* temp = &base_ptr[i];
            sum += *temp;
        }
    }
    return sum;
}

/* ========== NOINLINE FUNCTIONS ========== */

/* Volatile pointer walk - won't be optimized away */
__attribute__((noinline)) 
static int32_t volatile_ptr_walk(volatile int32_t* arr, size_t n) {
    volatile int32_t* ptr = arr;
    int32_t sum = 0;
    
    /* Volatile access pattern */
    for (size_t i = 0; i < n; i++) {
        sum += *ptr;
        ptr = ptr + 1;  /* Separate increment instruction */
    }
    return sum;
}

/* Force specific RTL pattern with manual pointer manipulation */
__attribute__((noinline))
static void manual_ptr_arithmetic(int32_t* arr, size_t n, int32_t* out) {
    int32_t* ptr = arr;
    
    /* Pattern designed to create mem_insn with reg1_val = 0 */
    for (size_t i = 0; i < n; i++) {
        /* Step 1: Load address into register */
        int32_t* current = ptr;
        
        /* Step 2: Use with zero offset (reg1_val = 0) */
        out[i] = *current;
        
        /* Step 3: Increment base register */
        ptr = ptr + 1;
        
        /* Interleaved computation to prevent optimization */
        out[i] += i * 7;
    }
}

/* Structure field access with pointer chasing */
__attribute__((noinline))
static int32_t struct_ptr_chasing(TestStruct* arr, size_t n) {
    TestStruct* ptr = arr;
    int32_t sum = 0;
    
    /* Multiple structure field accesses */
    while (ptr < arr + n) {
        /* Access pattern that might decompose to base + offset */
        sum += ptr->field1;
        
        /* Create the specific pattern for uncovered lines */
        TestStruct* next = ptr;
        sum += next->field2;  /* reg0 = next, reg1_val = offsetof(field2) */
        
        /* Pointer increment */
        ptr++;
        
        /* Another access to same pointer */
        sum += ptr->field3;
    }
    
    return sum;
}

/* ========== MAIN TEST FUNCTION ========== */

int main(void) {
    int32_t result = 0;
    
    /* Initialize arrays with predictable values */
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = (int32_t)(i * 3 + 1);
    }
    
    for (size_t i = 0; i < STRUCT_COUNT; i++) {
        struct_array[i].field1 = (int32_t)(i * 4);
        struct_array[i].field2 = (int32_t)(i * 4 + 1);
        struct_array[i].field3 = (int32_t)(i * 4 + 2);
        struct_array[i].field4 = (int32_t)(i * 4 + 3);
    }
    
    /* Test 1: Basic pointer increment (likely creates mem_insn pattern) */
    result += sum_with_ptr_increment(int_array, ARRAY_SIZE);
    
    /* Test 2: Pointer decrement pattern */
    result += sum_with_ptr_decrement(int_array, ARRAY_SIZE / 2);
    
    /* Test 3: Structure field processing */
    int32_t struct_results[4] = {0};
    process_struct_fields(struct_array, STRUCT_COUNT, struct_results);
    for (int i = 0; i < 4; i++) {
        result += struct_results[i];
    }
    
    /* Test 4: Mixed addressing patterns */
    result += sum_with_mixed_addressing(int_array, ARRAY_SIZE);
    
    /* Test 5: Volatile access (inhibits optimization but shows pattern) */
    volatile int32_t* volatile_ptr = int_array;
    result += volatile_ptr_walk(volatile_ptr, ARRAY_SIZE / 4);
    
    /* Test 6: Manual pointer arithmetic (targets specific uncovered lines) */
    int32_t manual_output[ARRAY_SIZE / 8] = {0};
    manual_ptr_arithmetic(int_array, ARRAY_SIZE / 8, manual_output);
    for (size_t i = 0; i < ARRAY_SIZE / 8; i++) {
        result += manual_output[i];
    }
    
    /* Test 7: Structure pointer chasing */
    result += struct_ptr_chasing(struct_array, STRUCT_COUNT / 2);
    
    /* Additional pointer patterns in main to increase coverage */
    {
        /* Double pointer traversal */
        int32_t* p1 = int_array;
        int32_t* p2 = int_array + ARRAY_SIZE / 2;
        
        for (int i = 0; i < 100; i++) {
            result += *p1 + *p2;
            p1++;
            p2--;
        }
        
        /* Pointer comparison as loop condition */
        int32_t* start = int_array;
        int32_t* end = int_array + 50;
        while (start < end) {
            result += *start;
            start++;
        }
    }
    
    /* Prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return result == 0 ? 1 : 0;
}

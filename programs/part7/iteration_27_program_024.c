/* auto-inc-dec-test.c */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define ARRAY_SIZE 1024
#define STRUCT_ARRAY_SIZE 512

/* Structure for complex addressing patterns */
typedef struct {
    int32_t field1;
    int32_t field2;
    int64_t field3;
    volatile int32_t volatile_field;
} TestStruct;

/* Global arrays to ensure they escape analysis */
static int32_t global_array[ARRAY_SIZE];
static TestStruct global_structs[STRUCT_ARRAY_SIZE];

/* Helper function likely to be inlined */
static inline int32_t sum_array_forward(int32_t* arr, size_t size) {
    int32_t sum = 0;
    int32_t* ptr = arr;
    int32_t* end = arr + size;
    
    /* Pattern 1: Simple pointer increment in loop */
    while (ptr < end) {
        sum += *ptr;
        ptr = ptr + 1;  /* Base register update with constant stride */
    }
    return sum;
}

/* Helper function likely to be inlined */
static inline int32_t sum_array_backward(int32_t* arr, size_t size) {
    int32_t sum = 0;
    int32_t* ptr = arr + size - 1;
    int32_t* start = arr;
    
    /* Pattern 2: Pointer decrement in loop */
    while (ptr >= start) {
        sum += *ptr;
        ptr = ptr - 1;  /* Base register decrement */
    }
    return sum;
}

/* noinline to prevent optimization across function boundaries */
__attribute__((noinline))
static int64_t process_struct_array(TestStruct* structs, size_t count) {
    int64_t total = 0;
    TestStruct* ptr = structs;
    TestStruct* end = structs + count;
    
    /* Pattern 3: Mixed field access with pointer arithmetic */
    while (ptr < end) {
        /* Access different fields with constant offsets */
        total += ptr->field1;
        total += ptr->field2;
        total += ptr->field3;
        
        ptr = ptr + 1;  /* Base register update by structure size */
    }
    return total;
}

/* Volatile pointer walk - inhibits some optimizations */
__attribute__((noinline))
static int32_t volatile_walk(volatile int32_t* arr, size_t size) {
    volatile int32_t* ptr = arr;
    volatile int32_t* end = arr + size;
    int32_t sum = 0;
    
    /* Pattern 4: Volatile pointer with increment */
    while (ptr < end) {
        sum += *ptr;
        ptr = ptr + 1;  /* Volatile pointer update */
    }
    return sum;
}

/* Complex pattern with nested addressing */
__attribute__((noinline))
static int32_t nested_access_pattern(void) {
    int32_t sum = 0;
    
    /* Pattern 5: Array of structures with field access */
    for (size_t i = 0; i < STRUCT_ARRAY_SIZE; ++i) {
        /* Direct index access */
        sum += global_structs[i].field1;
        
        /* Pointer arithmetic version */
        TestStruct* ptr = &global_structs[i];
        sum += ptr->field2;  /* reg0 = ptr, offset = field2 offset */
        
        /* Followed by pointer increment */
        ptr = ptr + 1;  /* Creates base register update pattern */
    }
    
    return sum;
}

/* Mixed volatile and non-volatile access */
__attribute__((noinline))
static int32_t mixed_volatile_access(void) {
    int32_t sum = 0;
    TestStruct* ptr = global_structs;
    
    /* Pattern 6: Interleaved volatile and regular accesses */
    for (size_t i = 0; i < STRUCT_ARRAY_SIZE; ++i) {
        /* Non-volatile access */
        sum += ptr->field1;
        
        /* Volatile access to inhibit certain optimizations */
        sum += ptr->volatile_field;
        
        /* Pointer increment - should create mem_insn pattern */
        ptr = ptr + 1;
    }
    
    return sum;
}

/* Function with multiple pointer variables */
__attribute__((noinline))
static int64_t multiple_pointers(void) {
    int64_t total = 0;
    int32_t* arr_ptr = global_array;
    TestStruct* struct_ptr = global_structs;
    
    /* Pattern 7: Two independent pointer walks */
    for (size_t i = 0; i < ARRAY_SIZE / 2; ++i) {
        /* First pointer access */
        total += *arr_ptr;
        arr_ptr = arr_ptr + 1;  /* Base register update */
        
        /* Second pointer access (different type/size) */
        total += struct_ptr->field3;
        struct_ptr = struct_ptr + 1;  /* Different stride */
    }
    
    return total;
}

/* Main test driver */
int main(void) {
    int64_t total = 0;
    
    /* Initialize arrays with pattern */
    for (size_t i = 0; i < ARRAY_SIZE; ++i) {
        global_array[i] = (int32_t)(i * 3 + 1);
    }
    
    for (size_t i = 0; i < STRUCT_ARRAY_SIZE; ++i) {
        global_structs[i].field1 = (int32_t)(i * 2);
        global_structs[i].field2 = (int32_t)(i * 2 + 1);
        global_structs[i].field3 = (int64_t)(i * 100);
        global_structs[i].volatile_field = (int32_t)(i * 5);
    }
    
    /* Test 1: Forward pointer walk (likely inlined) */
    total += sum_array_forward(global_array, ARRAY_SIZE);
    
    /* Test 2: Backward pointer walk (likely inlined) */
    total += sum_array_backward(global_array, ARRAY_SIZE);
    
    /* Test 3: Structure array processing (noinline) */
    total += process_struct_array(global_structs, STRUCT_ARRAY_SIZE);
    
    /* Test 4: Volatile walk (noinline) */
    total += volatile_walk(global_array, ARRAY_SIZE);
    
    /* Test 5: Nested access pattern (noinline) */
    total += nested_access_pattern();
    
    /* Test 6: Mixed volatile access (noinline) */
    total += mixed_volatile_access();
    
    /* Test 7: Multiple pointers (noinline) */
    total += multiple_pointers();
    
    /* Additional live value pressure */
    int32_t* temp_ptr = global_array;
    for (int i = 0; i < 100; ++i) {
        total += *temp_ptr;
        temp_ptr = temp_ptr + 1;  /* More base register updates */
    }
    
    /* Prevent dead code elimination */
    printf("Result: %lld\n", (long long)total);
    
    return total > 0 ? 0 : 1;
}

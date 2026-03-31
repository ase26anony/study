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
int32_t global_array[ARRAY_SIZE];
TestStruct global_structs[STRUCT_ARRAY_SIZE];

/* Helper functions with different attributes */

/* Likely to be inlined */
static int32_t sum_array_forward(int32_t* arr, size_t size) {
    int32_t sum = 0;
    int32_t* ptr = arr;
    int32_t* end = arr + size;
    
    /* Pattern 1: while loop with pointer comparison */
    while (ptr < end) {
        sum += *ptr;
        ptr = ptr + 1;  /* Base register update with constant stride */
    }
    return sum;
}

/* Likely to be inlined */
static int32_t sum_array_backward(int32_t* arr, size_t size) {
    int32_t sum = 0;
    int32_t* ptr = arr + size - 1;
    int32_t* start = arr;
    
    /* Pattern 2: backward traversal */
    while (ptr >= start) {
        sum += *ptr;
        ptr = ptr - 1;  /* Base register decrement */
    }
    return sum;
}

/* Force no inlining to test pass behavior */
__attribute__((noinline)) 
static int64_t process_structs_noinline(TestStruct* structs, size_t count) {
    int64_t total = 0;
    TestStruct* ptr = structs;
    TestStruct* end = structs + count;
    
    /* Pattern 3: Mixed volatile and non-volatile access */
    while (ptr < end) {
        /* Access regular fields */
        total += ptr->field1;
        total += ptr->field2;
        
        /* Volatile access - inhibits some optimizations but still presents pattern */
        volatile int32_t* volatile_ptr = &ptr->volatile_field;
        total += *volatile_ptr;
        
        ptr = ptr + 1;  /* Base register update */
    }
    return total;
}

/* Likely to be inlined */
static int64_t process_struct_fields(TestStruct* structs, size_t count) {
    int64_t total = 0;
    
    /* Pattern 4: Array indexing with structure field access */
    for (size_t i = 0; i < count; i++) {
        /* Complex addressing: array[i].field */
        total += structs[i].field1;
        total += structs[i].field2;
        
        /* Create register pressure */
        if (i % 2 == 0) {
            total += structs[i].field3;
        }
    }
    
    /* Pattern 5: Pointer walk version */
    TestStruct* ptr = structs;
    for (size_t i = 0; i < count; i++) {
        /* Mixed base register usage */
        int32_t* field_ptr = &ptr->field1;
        total += *field_ptr;
        
        field_ptr = &ptr->field2;  /* Re-assign base register */
        total += *field_ptr;
        
        ptr = ptr + 1;  /* Update base register */
    }
    
    return total;
}

/* Force no inlining */
__attribute__((noinline))
static void volatile_pointer_walk(volatile int32_t* arr, size_t size) {
    volatile int32_t* ptr = arr;
    volatile int32_t* end = arr + size;
    
    /* Pattern 6: Volatile pointer with constant stride */
    while (ptr < end) {
        /* Volatile access creates specific memory patterns */
        *ptr = (*ptr) + 1;
        
        /* Explicit pointer increment - should create reg0 update pattern */
        ptr = ptr + 1;
    }
}

/* Main test driver */
int main(void) {
    int64_t total_result = 0;
    
    /* Initialize arrays */
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        global_array[i] = (int32_t)(i % 100);
    }
    
    for (size_t i = 0; i < STRUCT_ARRAY_SIZE; i++) {
        global_structs[i].field1 = (int32_t)i;
        global_structs[i].field2 = (int32_t)(i * 2);
        global_structs[i].field3 = (int64_t)(i * 3);
        global_structs[i].volatile_field = (int32_t)(i % 50);
    }
    
    /* Test 1: Forward traversal with pointer arithmetic */
    total_result += sum_array_forward(global_array, ARRAY_SIZE);
    
    /* Test 2: Backward traversal */
    total_result += sum_array_backward(global_array, ARRAY_SIZE / 2);
    
    /* Test 3: Mixed index and pointer access on structures */
    total_result += process_struct_fields(global_structs, STRUCT_ARRAY_SIZE / 2);
    
    /* Test 4: Noinline function with volatile mix */
    total_result += process_structs_noinline(
        &global_structs[STRUCT_ARRAY_SIZE / 2], 
        STRUCT_ARRAY_SIZE / 2
    );
    
    /* Test 5: Volatile pointer walk */
    volatile_pointer_walk(global_array, ARRAY_SIZE / 4);
    
    /* Test 6: Additional patterns with different strides */
    {
        /* Pattern with pointer increment in expression */
        int32_t* ptr = global_array;
        int32_t sum = 0;
        for (int i = 0; i < 100; i++) {
            /* Integrated increment in access */
            sum += *ptr++;
        }
        total_result += sum;
        
        /* Pattern with pre-increment */
        ptr = global_array + 100;
        for (int i = 0; i < 100; i++) {
            sum += *(++ptr);
        }
        total_result += sum;
    }
    
    /* Test 7: Nested structure access with pointer chain */
    {
        TestStruct* struct_ptr = global_structs;
        for (int i = 0; i < 100; i++) {
            /* Chain of accesses to create complex addressing */
            int32_t* field1_ptr = &struct_ptr->field1;
            int32_t* field2_ptr = &struct_ptr->field2;
            
            total_result += *field1_ptr;
            total_result += *field2_ptr;
            
            /* Update base register */
            struct_ptr = struct_ptr + 1;
        }
    }
    
    /* Prevent dead code elimination */
    printf("Total result: %lld\n", (long long)total_result);
    
    return 0;
}

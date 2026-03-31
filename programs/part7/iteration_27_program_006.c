#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define ARRAY_SIZE 1024
#define STRUCT_COUNT 512

// Structure for complex addressing patterns
typedef struct {
    int32_t field1;
    int32_t field2;
    int64_t field3;
    volatile int32_t volatile_field;
} TestStruct;

// Global arrays to ensure they escape analysis
static int32_t global_array[ARRAY_SIZE];
static TestStruct global_structs[STRUCT_COUNT];

// NOINLINE functions to prevent optimization across boundaries
__attribute__((noinline)) 
static void process_forward_noinline(int32_t* arr, size_t size) {
    int32_t sum = 0;
    int32_t* ptr = arr;
    int32_t* end = arr + size;
    
    // Pattern 1: Simple pointer increment in loop
    while (ptr < end) {
        sum += *ptr;
        ptr = ptr + 1;  // Explicit assignment to force base register update
    }
    
    // Use the result to prevent dead code elimination
    global_array[0] = sum;
}

__attribute__((noinline))
static void process_backward_noinline(int32_t* arr, size_t size) {
    int32_t sum = 0;
    int32_t* ptr = arr + size - 1;
    int32_t* start = arr;
    
    // Pattern 2: Pointer decrement in loop
    while (ptr >= start) {
        sum += *ptr;
        ptr = ptr - 1;  // Explicit assignment
    }
    
    global_array[1] = sum;
}

// Static function likely to be inlined
static int64_t process_struct_fields_inline(TestStruct* structs, size_t count) {
    int64_t total = 0;
    TestStruct* ptr = structs;
    TestStruct* end = structs + count;
    
    // Pattern 3: Mixed volatile and non-volatile access with constant stride
    while (ptr < end) {
        // Access regular fields
        total += ptr->field1;
        total += ptr->field2;
        
        // Access volatile field - inhibits some optimizations but still shows pattern
        int32_t volatile_val = ptr->volatile_field;
        total += volatile_val;
        
        ptr = ptr + 1;  // Base register update with constant stride
    }
    
    return total;
}

// Function with complex addressing patterns
__attribute__((noinline))
static void process_nested_access(TestStruct* structs, size_t count) {
    int64_t sum = 0;
    
    // Pattern 4: Array indexing with constant stride
    for (size_t i = 0; i < count; i++) {
        // This creates base + offset addressing
        sum += structs[i].field1;
        sum += structs[i].field3;
    }
    
    // Pattern 5: Pointer walk with mixed operations
    TestStruct* ptr = structs;
    for (size_t i = 0; i < count; i++) {
        // Force base register load then use
        TestStruct* current = ptr;
        sum += current->field2;
        
        // Update base register
        ptr = ptr + 1;
    }
    
    global_structs[0].field3 = sum;
}

// Function with volatile pointer manipulation
__attribute__((noinline))
static void process_volatile_walk(volatile int32_t* arr, size_t size) {
    volatile int32_t* volatile_ptr = arr;
    int32_t sum = 0;
    
    // Pattern 6: Volatile pointer with increment
    for (size_t i = 0; i < size; i++) {
        sum += *volatile_ptr;
        volatile_ptr = volatile_ptr + 1;  // Volatile pointer update
    }
    
    // Store through volatile to ensure side effects
    *(volatile int32_t*)&global_array[2] = sum;
}

// Main test function combining all patterns
static void run_auto_inc_dec_tests(void) {
    // Initialize arrays
    for (int i = 0; i < ARRAY_SIZE; i++) {
        global_array[i] = i * 3 + 1;
    }
    
    for (int i = 0; i < STRUCT_COUNT; i++) {
        global_structs[i].field1 = i * 2;
        global_structs[i].field2 = i * 3;
        global_structs[i].field3 = i * 5;
        global_structs[i].volatile_field = i * 7;
    }
    
    // Test 1: Forward processing with noinline boundary
    process_forward_noinline(global_array, ARRAY_SIZE);
    
    // Test 2: Backward processing with noinline boundary  
    process_backward_noinline(global_array, ARRAY_SIZE / 2);
    
    // Test 3: Inlined struct processing (likely to be inlined at -O3)
    int64_t struct_sum = process_struct_fields_inline(global_structs, STRUCT_COUNT);
    global_structs[1].field3 = struct_sum;
    
    // Test 4: Complex nested access patterns
    process_nested_access(global_structs + 10, STRUCT_COUNT - 20);
    
    // Test 5: Volatile pointer walk
    process_volatile_walk((volatile int32_t*)global_array, ARRAY_SIZE);
    
    // Test 6: Mixed patterns in same basic block
    {
        int32_t* ptr = global_array + 100;
        int32_t sum = 0;
        
        // Multiple accesses with base register updates
        sum += *ptr; ptr = ptr + 1;
        sum += *ptr; ptr = ptr + 1;
        sum += *ptr; ptr = ptr + 1;
        sum += *ptr;
        
        global_array[3] = sum;
    }
    
    // Test 7: Loop with pointer comparison and increment
    {
        int32_t* ptr = global_array + 200;
        int32_t* end = ptr + 50;
        int32_t sum = 0;
        
        while (ptr < end) {
            // This should create mem_insn with reg0 = ptr, reg1_val = 0
            int32_t val = *ptr;
            sum += val;
            
            // Base register update
            ptr = ptr + 1;
        }
        
        global_array[4] = sum;
    }
    
    // Test 8: Structure field access via pointer arithmetic
    {
        TestStruct* sptr = global_structs;
        int64_t sum = 0;
        
        for (int i = 0; i < 100; i++) {
            // Access field1 with offset 0 from base
            sum += sptr->field1;
            
            // Access field2 with constant offset
            sum += sptr->field2;
            
            // Update base register
            sptr = sptr + 1;
        }
        
        global_structs[2].field3 = sum;
    }
}

int main(void) {
    printf("Starting auto-inc-dec pattern tests...\n");
    
    // Run tests multiple times to increase coverage probability
    for (int iteration = 0; iteration < 3; iteration++) {
        run_auto_inc_dec_tests();
        
        // Simple arithmetic to create register pressure
        int32_t temp = 0;
        for (int i = 0; i < 100; i++) {
            temp += global_array[i % ARRAY_SIZE];
        }
        global_array[5] = temp;
    }
    
    // Aggregate and print results to prevent dead code elimination
    int64_t final_sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_sum += global_array[i];
    }
    for (int i = 0; i < STRUCT_COUNT; i++) {
        final_sum += global_structs[i].field1;
        final_sum += global_structs[i].field3;
    }
    
    printf("Final checksum: %ld\n", (long)final_sum);
    printf("Test completed.\n");
    
    return 0;
}

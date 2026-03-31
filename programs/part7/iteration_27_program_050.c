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
    volatile int32_t volatile_field;
} test_struct;

/* Global arrays to ensure they escape analysis */
int32_t global_array[ARRAY_SIZE];
test_struct global_structs[STRUCT_COUNT];

/* ========== INLINEABLE FUNCTIONS ========== */

/* Function A: Forward traversal with ptr++ */
static int32_t sum_array_forward(int32_t* arr, size_t size) {
    int32_t* ptr = arr;
    int32_t* end = arr + size;
    int32_t sum = 0;
    
    /* Pattern: ptr++ in loop condition */
    while (ptr < end) {
        sum += *ptr++;
    }
    
    /* Additional pattern: mixed base register update */
    int32_t* p = arr;
    int32_t val1 = *p;
    p = p + 1;  /* Force base register update */
    int32_t val2 = *p;
    
    return sum + val1 + val2;
}

/* Function B: Backward traversal with ptr-- */
static int32_t sum_array_backward(int32_t* arr, size_t size) {
    int32_t* ptr = arr + size - 1;
    int32_t sum = 0;
    
    /* Pattern: ptr-- in loop */
    for (int32_t i = 0; i < size; i++) {
        sum += *ptr--;
    }
    
    return sum;
}

/* Function C: Structure field access with constant stride */
static int32_t sum_struct_fields(test_struct* structs, size_t count) {
    test_struct* ptr = structs;
    int32_t sum = 0;
    
    /* Access different fields with constant offsets */
    for (size_t i = 0; i < count; i++) {
        /* Pattern: array[i].field - decomposes to base + constant */
        sum += ptr->field1;
        sum += ptr->field2;
        
        /* Volatile access mixed in */
        sum += ptr->volatile_field;
        
        ptr = ptr + 1;  /* Constant stride of sizeof(test_struct) */
    }
    
    return sum;
}

/* ========== NOINLINE FUNCTIONS ========== */

/* Function D: Volatile pointer traversal - won't be inlined */
__attribute__((noinline))
static int32_t volatile_traversal(volatile int32_t* arr, size_t size) {
    volatile int32_t* ptr = arr;
    int32_t sum = 0;
    
    /* Volatile access pattern */
    for (size_t i = 0; i < size; i++) {
        sum += *ptr;
        ptr = ptr + 1;  /* Separate increment */
    }
    
    /* Mixed pattern within same function */
    volatile int32_t* p = arr;
    int32_t temp = *p;
    p = p + 1;
    temp += *p;
    
    return sum + temp;
}

/* Function E: Complex nested access - won't be inlined */
__attribute__((noinline))
static int32_t complex_nested_access(test_struct* structs, size_t count) {
    int32_t sum = 0;
    
    /* Multiple addressing patterns */
    for (size_t i = 0; i < count; i++) {
        /* Direct array access */
        sum += structs[i].field1;
        
        /* Pointer arithmetic */
        test_struct* ptr = &structs[i];
        sum += ptr->field2;
        
        /* Pointer walk with constant offset */
        int32_t* field_ptr = &ptr->field3;
        sum += *field_ptr;
        field_ptr = field_ptr + 1;  /* Would point to next field if contiguous */
    }
    
    return sum;
}

/* Function F: Mixed volatile and non-volatile */
__attribute__((noinline))
static int32_t mixed_volatile_access(int32_t* arr, volatile int32_t* varr, size_t size) {
    int32_t* ptr = arr;
    volatile int32_t* vptr = varr;
    int32_t sum = 0;
    
    /* Interleaved access patterns */
    for (size_t i = 0; i < size; i += 2) {
        /* Non-volatile with auto-increment pattern */
        sum += *ptr++;
        
        /* Volatile access */
        sum += *vptr;
        vptr = vptr + 1;
        
        /* Another non-volatile */
        sum += *ptr++;
    }
    
    return sum;
}

/* ========== MAIN TEST FUNCTION ========== */

int main(void) {
    int32_t result = 0;
    
    /* Initialize arrays */
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        global_array[i] = (int32_t)(i % 100);
    }
    
    for (size_t i = 0; i < STRUCT_COUNT; i++) {
        global_structs[i].field1 = (int32_t)i;
        global_structs[i].field2 = (int32_t)(i * 2);
        global_structs[i].field3 = (int32_t)(i * 3);
        global_structs[i].volatile_field = (int32_t)(i * 4);
    }
    
    /* Create volatile alias */
    volatile int32_t* volatile_array = (volatile int32_t*)global_array;
    
    /* Test 1: Inlineable forward traversal */
    result += sum_array_forward(global_array, ARRAY_SIZE);
    
    /* Test 2: Inlineable backward traversal */
    result += sum_array_backward(global_array, ARRAY_SIZE);
    
    /* Test 3: Inlineable structure access */
    result += sum_struct_fields(global_structs, STRUCT_COUNT);
    
    /* Test 4: Non-inline volatile traversal */
    result += volatile_traversal(volatile_array, ARRAY_SIZE);
    
    /* Test 5: Non-inline complex nested */
    result += complex_nested_access(global_structs, STRUCT_COUNT);
    
    /* Test 6: Mixed volatile/non-volatile */
    result += mixed_volatile_access(global_array, volatile_array, ARRAY_SIZE);
    
    /* Additional patterns to create register pressure */
    {
        /* Multiple pointer variables in same scope */
        int32_t* p1 = global_array;
        int32_t* p2 = global_array + ARRAY_SIZE/2;
        int32_t* p3 = global_array + ARRAY_SIZE/4;
        
        /* Pattern: base register update followed by use */
        int32_t* base = global_array;
        int32_t val1 = *base;      /* mem_insn with reg1_val = 0 */
        base = base + 1;           /* Potential increment to find */
        int32_t val2 = *base;      /* Another access */
        
        result += val1 + val2;
        
        /* Pointer comparison loop */
        while (p1 < p2) {
            result += *p1++;
            result += *p3++;
        }
    }
    
    /* Structure array with pointer arithmetic */
    {
        test_struct* sptr = global_structs;
        for (size_t i = 0; i < STRUCT_COUNT; i++) {
            /* Access via pointer with constant offset */
            result += sptr->field1;
            
            /* Explicit pointer arithmetic */
            int32_t* field_ptr = &sptr->field2;
            result += *field_ptr;
            
            /* Move to next structure */
            sptr = sptr + 1;
        }
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return 0;
}

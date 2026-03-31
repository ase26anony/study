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

/* Global arrays to ensure they're not optimized away */
static int32_t global_array[ARRAY_SIZE];
static TestStruct global_structs[STRUCT_ARRAY_SIZE];

/* Helper function likely to be inlined */
static __attribute__((always_inline)) 
inline int32_t sum_array_inline(int32_t* arr, size_t size) {
    int32_t sum = 0;
    int32_t* ptr = arr;
    int32_t* end = arr + size;
    
    /* Pattern 1: Simple pointer increment in loop - should create reg0 + 0 pattern */
    while (ptr < end) {
        sum += *ptr;  /* Memory access with base register */
        ptr = ptr + 1; /* Base register update - potential for auto-inc */
    }
    return sum;
}

/* Function that won't be inlined - forces RTL generation at function boundaries */
__attribute__((noinline)) 
static int32_t sum_array_noinline(int32_t* arr, size_t size) {
    int32_t sum = 0;
    
    /* Pattern 2: Mixed pointer arithmetic with constant stride */
    for (size_t i = 0; i < size; i++) {
        int32_t* p = &arr[i];  /* Base register assignment */
        sum += *p;             /* Memory access with reg0 + 0 offset */
        /* No explicit increment here - let loop increment handle it */
    }
    return sum;
}

/* Function with backward traversal */
__attribute__((noinline))
static int32_t reverse_sum_array(int32_t* arr, size_t size) {
    int32_t sum = 0;
    int32_t* ptr = arr + size - 1;
    
    /* Pattern 3: Pointer decrement pattern */
    while (ptr >= arr) {
        sum += *ptr;
        ptr = ptr - 1;  /* Potential for auto-dec */
    }
    return sum;
}

/* Function with volatile pointer */
__attribute__((noinline))
static void process_with_volatile(volatile int32_t* arr, size_t size) {
    volatile int32_t* vptr = arr;
    volatile int32_t* vend = arr + size;
    
    /* Pattern 4: Volatile access with pointer increment */
    while (vptr < vend) {
        (void)*vptr;  /* Volatile read */
        vptr = vptr + 1;
    }
}

/* Function accessing structure fields */
__attribute__((noinline))
static int64_t sum_struct_fields(TestStruct* structs, size_t size) {
    int64_t total = 0;
    TestStruct* ptr = structs;
    TestStruct* end = structs + size;
    
    /* Pattern 5: Structure access with constant offset */
    while (ptr < end) {
        /* Multiple memory accesses with same base pointer */
        total += ptr->field1;  /* Base + 0 offset */
        total += ptr->field2;  /* Base + 4 offset */
        total += ptr->field3;  /* Base + 8 offset */
        
        /* Volatile access to inhibit some optimizations but still show pattern */
        (void)ptr->volatile_field;
        
        ptr = ptr + 1;  /* Base register update by structure size */
    }
    return total;
}

/* Complex pattern: Nested structure in array */
__attribute__((noinline))
static int32_t complex_access_pattern(void) {
    int32_t sum = 0;
    
    /* Pattern 6: Multiple interleaved pointer walks */
    int32_t* ptr1 = global_array;
    int32_t* ptr2 = global_array + ARRAY_SIZE/2;
    
    for (int i = 0; i < ARRAY_SIZE/2; i++) {
        /* Two independent base registers being updated */
        sum += *ptr1;  /* First memory access */
        sum += *ptr2;  /* Second memory access */
        
        ptr1 = ptr1 + 1;  /* First base update */
        ptr2 = ptr2 + 1;  /* Second base update */
    }
    
    return sum;
}

/* Function that combines multiple patterns */
static int32_t combined_patterns(void) {
    int32_t result = 0;
    
    /* Mix different access patterns in same basic block */
    int32_t* p = global_array;
    
    /* Sequence that should create mem_insn with reg0 + 0 */
    result += *p;        /* Memory access */
    p = p + 1;           /* Base register update */
    result += *p;        /* Another access */
    p = p + 1;           /* Another update */
    
    return result;
}

/* Main test driver */
int main(void) {
    int32_t total = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        global_array[i] = i % 100;
    }
    
    for (int i = 0; i < STRUCT_ARRAY_SIZE; i++) {
        global_structs[i].field1 = i;
        global_structs[i].field2 = i * 2;
        global_structs[i].field3 = i * 3LL;
        global_structs[i].volatile_field = i * 4;
    }
    
    /* Test 1: Inlined function with pointer walk */
    total += sum_array_inline(global_array, ARRAY_SIZE);
    
    /* Test 2: Non-inlined function */
    total += sum_array_noinline(global_array, ARRAY_SIZE);
    
    /* Test 3: Reverse traversal */
    total += reverse_sum_array(global_array, ARRAY_SIZE);
    
    /* Test 4: Volatile access pattern */
    process_with_volatile(global_array, ARRAY_SIZE);
    
    /* Test 5: Structure access pattern */
    total += sum_struct_fields(global_structs, STRUCT_ARRAY_SIZE);
    
    /* Test 6: Complex pattern */
    total += complex_access_pattern();
    
    /* Test 7: Combined patterns in basic block */
    total += combined_patterns();
    
    /* Additional patterns to increase register pressure */
    {
        /* Multiple pointer variables in same scope */
        int32_t* p1 = global_array;
        int32_t* p2 = global_array + 100;
        int32_t* p3 = global_array + 200;
        
        for (int i = 0; i < 100; i++) {
            total += *p1 + *p2 + *p3;
            p1 = p1 + 1;
            p2 = p2 + 1;
            p3 = p3 + 1;
        }
    }
    
    /* Array indexing that should decompose to base + offset */
    {
        TestStruct* sp = global_structs;
        for (int i = 0; i < 100; i++) {
            /* This should create addressing like: base + (i * sizeof(TestStruct)) + field_offset */
            total += sp[i].field1;  /* May decompose to temporary base register */
        }
    }
    
    printf("Result: %d\n", total);
    
    /* Prevent dead code elimination */
    if (total > 0) {
        return 0;
    } else {
        return 1;
    }
}

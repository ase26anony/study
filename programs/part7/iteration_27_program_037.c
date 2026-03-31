#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

#define ARRAY_SIZE 1024
#define STRUCT_COUNT 512

/* Simple structure for nested access */
struct DataPoint {
    int value;
    int timestamp;
    char tag;
    int metadata;
};

/* Global arrays for testing */
static int int_array[ARRAY_SIZE];
static struct DataPoint struct_array[STRUCT_COUNT];

/* ====== Inline-friendly functions ====== */

/* Function A: Forward traversal with ptr++ */
static int sum_array_forward(const int* arr, int size) {
    const int* ptr = arr;
    const int* end = arr + size;
    int sum = 0;
    
    /* Pattern: *ptr++ with constant stride */
    while (ptr < end) {
        sum += *ptr++;
    }
    return sum;
}

/* Function B: Reverse traversal with ptr-- */
static int sum_array_reverse(const int* arr, int size) {
    const int* ptr = arr + size - 1;
    const int* start = arr;
    int sum = 0;
    
    /* Pattern: *ptr-- with constant stride */
    while (ptr >= start) {
        sum += *ptr--;
    }
    return sum;
}

/* Function C: Structure field access with mixed patterns */
static int process_struct_array(struct DataPoint* arr, int count) {
    struct DataPoint* ptr = arr;
    int total = 0;
    
    /* Mixed base register updates */
    for (int i = 0; i < count; i++) {
        /* Access via pointer with constant offset */
        total += ptr->value;
        
        /* Update base register */
        ptr = ptr + 1;  /* This creates reg0 update followed by mem access */
    }
    
    /* Alternative pattern with integrated increment */
    ptr = arr;
    for (int i = 0; i < count; i++) {
        /* This should generate mem_insn with reg1_val = 0 */
        total += (ptr++)->timestamp;
    }
    
    return total;
}

/* Function D: Volatile pointer walk */
static int volatile_walk(volatile int* arr, int size) {
    volatile int* ptr = arr;
    int sum = 0;
    
    /* Volatile access still presents the pattern */
    for (int i = 0; i < size; i++) {
        sum += *ptr;
        ptr = ptr + 1;  /* Separate increment to force pattern */
    }
    
    return sum;
}

/* ====== Noinline functions to preserve patterns ====== */

/* Force no inlining to test pass behavior at function boundaries */
__attribute__((noinline))
static int noinline_sum_forward(int* arr, int size) {
    int* ptr = arr;
    int sum = 0;
    
    /* Explicit sequence to create mem_insn pattern */
    for (int i = 0; i < size; i++) {
        /* This should create: mem_insn.mem_loc = address_of_x */
        /* with reg0 = ptr and reg1_val = 0 */
        sum += *ptr;
        
        /* Update base register - this is what find_inc() looks for */
        ptr = ptr + 1;
    }
    
    return sum;
}

__attribute__((noinline))
static int noinline_struct_walk(struct DataPoint* arr, int count) {
    struct DataPoint* ptr = arr;
    int result = 0;
    
    /* Complex addressing: array[i].field */
    for (int i = 0; i < count; i++) {
        /* Multiple field accesses with same base */
        result += ptr[i].value;
        result -= ptr[i].timestamp;
        
        /* This creates opportunities for auto-inc-dec */
        if (i % 2 == 0) {
            result += (ptr + i)->metadata;
        }
    }
    
    return result;
}

/* ====== Main test driver ====== */

int main(void) {
    int total = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i % 100;
    }
    
    for (int i = 0; i < STRUCT_COUNT; i++) {
        struct_array[i].value = i;
        struct_array[i].timestamp = i * 2;
        struct_array[i].tag = 'A' + (i % 26);
        struct_array[i].metadata = i * 3;
    }
    
    /* Test 1: Forward traversal (likely inlined) */
    total += sum_array_forward(int_array, ARRAY_SIZE);
    
    /* Test 2: Reverse traversal (likely inlined) */
    total += sum_array_reverse(int_array, ARRAY_SIZE / 2);
    
    /* Test 3: Structure access with mixed patterns */
    total += process_struct_array(struct_array, STRUCT_COUNT / 2);
    
    /* Test 4: Volatile access pattern */
    total += volatile_walk(int_array, ARRAY_SIZE / 4);
    
    /* Test 5: Noinline forward sum */
    total += noinline_sum_forward(int_array, ARRAY_SIZE);
    
    /* Test 6: Noinline structure walk */
    total += noinline_struct_walk(struct_array, STRUCT_COUNT / 4);
    
    /* Test 7: Nested loop with pointer arithmetic */
    {
        int* ptr = int_array;
        int* end = int_array + ARRAY_SIZE;
        
        /* while (ptr < end) pattern */
        while (ptr < end) {
            total += *ptr;
            
            /* Force register pressure */
            int temp = *ptr * 2;
            total += temp;
            
            ptr = ptr + 1;  /* Base register update */
        }
    }
    
    /* Test 8: Combined array/structure access */
    {
        struct DataPoint* sptr = struct_array;
        for (int i = 0; i < 100; i++) {
            /* Access different fields to create complex addressing */
            total += sptr->value;
            total += sptr->timestamp;
            
            /* Update base with constant stride */
            sptr = sptr + 1;
            
            /* Interleave with int array access */
            total += int_array[i];
        }
    }
    
    /* Test 9: Pointer comparison as loop condition */
    {
        char* cptr = (char*)struct_array;
        char* cend = cptr + sizeof(struct DataPoint) * (STRUCT_COUNT / 8);
        
        while (cptr < cend) {
            total += *cptr;
            cptr = cptr + sizeof(int);  /* Skip by int size */
        }
    }
    
    /* Test 10: Multiple increments in same block */
    {
        int* p1 = int_array;
        int* p2 = int_array + ARRAY_SIZE / 2;
        
        for (int i = 0; i < 50; i++) {
            /* Multiple memory accesses with base register updates */
            total += *p1;
            p1 = p1 + 1;
            
            total += *p2;
            p2 = p2 - 1;  /* Negative stride */
        }
    }
    
    printf("Total result: %d\n", total);
    
    /* Use result to prevent optimization */
    if (total > 0) {
        return 0;
    } else {
        return 1;
    }
}

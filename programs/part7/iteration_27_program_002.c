#include <stdio.h>
#include <stddef.h>

#define ARRAY_SIZE 1024
#define STRUCT_COUNT 512

/* Structure for nested access patterns */
struct data_item {
    int value;
    int tag;
    int payload[4];
};

/* Global arrays to ensure they escape analysis */
int int_array[ARRAY_SIZE];
struct data_item struct_array[STRUCT_COUNT];

/* ====== INLINE CANDIDATES ====== */

/* Forward traversal with pointer increment - likely to inline */
static int sum_forward(int* arr, int n) {
    int sum = 0;
    int* ptr = arr;
    int* end = arr + n;
    
    /* Pattern: ptr++ in loop condition */
    while (ptr < end) {
        sum += *ptr++;
    }
    return sum;
}

/* Backward traversal with pointer decrement - likely to inline */
static int sum_backward(int* arr, int n) {
    int sum = 0;
    int* ptr = arr + n - 1;
    
    /* Pattern: *ptr-- with explicit decrement */
    for (int i = 0; i < n; i++) {
        sum += *ptr;
        ptr = ptr - 1;  /* Separate decrement to create reg update pattern */
    }
    return sum;
}

/* Structure field access with mixed patterns */
static int sum_struct_tags(struct data_item* items, int n) {
    int sum = 0;
    struct data_item* ptr = items;
    
    /* Access field with constant offset from base pointer */
    for (int i = 0; i < n; i++) {
        sum += ptr->tag;          /* Base + constant offset */
        sum += ptr->payload[0];   /* Different constant offset */
        ptr = ptr + 1;            /* Update base register */
    }
    return sum;
}

/* ====== NOINLINE FUNCTIONS ====== */

/* Force no-inline to test pass behavior at function boundaries */
__attribute__((noinline)) 
static int volatile_walk(volatile int* arr, int n) {
    int sum = 0;
    volatile int* ptr = arr;
    
    /* Volatile pointer walk - inhibits some optimizations */
    for (int i = 0; i < n; i++) {
        sum += *ptr;
        ptr = ptr + 1;  /* Separate increment for volatile */
    }
    return sum;
}

__attribute__((noinline))
static int mixed_base_update(int* arr, int n) {
    int sum = 0;
    
    /* Explicit base register update pattern */
    for (int i = 0; i < n; i++) {
        int* p = &arr[i];      /* Base register assignment */
        sum += *p;             /* Memory access with reg0 = p, offset 0 */
        /* Following instruction updates the base register */
        p = p + 1;             /* This creates the pattern for find_inc */
    }
    return sum;
}

/* Complex nested structure access */
__attribute__((noinline))
static int nested_structure_access(struct data_item* items, int n) {
    int sum = 0;
    
    /* Multiple addressing modes in one loop */
    for (int i = 0; i < n; i++) {
        /* Array index then field access */
        sum += items[i].value;
        
        /* Pointer walk with constant stride */
        struct data_item* ptr = &items[i];
        sum += ptr->payload[2];  /* Constant offset from base */
    }
    
    return sum;
}

/* ====== MAIN TEST DRIVER ====== */

int main(void) {
    int total = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i % 100;
    }
    
    for (int i = 0; i < STRUCT_COUNT; i++) {
        struct_array[i].value = i;
        struct_array[i].tag = i * 2;
        for (int j = 0; j < 4; j++) {
            struct_array[i].payload[j] = i + j;
        }
    }
    
    /* Test 1: Forward pointer traversal (likely inlined) */
    total += sum_forward(int_array, ARRAY_SIZE);
    
    /* Test 2: Backward traversal with separate decrement */
    total += sum_backward(int_array, ARRAY_SIZE / 2);
    
    /* Test 3: Structure field access patterns */
    total += sum_struct_tags(struct_array, STRUCT_COUNT / 2);
    
    /* Test 4: Volatile access pattern */
    total += volatile_walk(int_array, ARRAY_SIZE / 4);
    
    /* Test 5: Explicit base register update */
    total += mixed_base_update(int_array, ARRAY_SIZE / 2);
    
    /* Test 6: Nested structure patterns */
    total += nested_structure_access(struct_array, STRUCT_COUNT / 4);
    
    /* Additional mixed pattern to increase register pressure */
    {
        int* ptr1 = int_array;
        int* ptr2 = int_array + ARRAY_SIZE / 2;
        
        /* Two parallel pointer walks */
        for (int i = 0; i < 100; i++) {
            total += *ptr1 + *ptr2;
            ptr1 = ptr1 + 1;  /* Separate increment */
            ptr2 = ptr2 + 1;  /* Separate increment */
        }
    }
    
    /* Structure pointer with constant stride */
    {
        struct data_item* sptr = struct_array;
        for (int i = 0; i < 50; i++) {
            /* Multiple accesses with same base */
            total += sptr->value;
            total += sptr->tag;
            sptr = sptr + 1;  /* Update base register */
        }
    }
    
    /* Print result to prevent elimination */
    printf("Total: %d\n", total);
    
    return 0;
}

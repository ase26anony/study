#include <stdio.h>
#include <stddef.h>

#define SIZE 256
#define STRUCT_SIZE 100

/* Structure for nested access patterns */
struct Data {
    int value;
    int tag;
    char payload[8];
};

/* Global arrays to ensure they escape analysis */
int int_array[SIZE];
struct Data struct_array[STRUCT_SIZE];

/* Helper likely to be inlined */
static int sum_with_pointer_increment(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    int *end = arr + n;
    
    /* Pattern 1: while loop with pointer comparison */
    while (ptr < end) {
        sum += *ptr;
        ptr = ptr + 1;  /* Base register update with constant stride */
    }
    return sum;
}

/* Helper likely to be inlined */
static int reverse_with_pointer_decrement(int *arr, int n) {
    int sum = 0;
    int *ptr = arr + n - 1;
    
    /* Pattern 2: backward traversal */
    for (int i = 0; i < n; i++) {
        sum += *ptr;
        ptr = ptr - 1;  /* Base register decrement */
    }
    return sum;
}

/* noinline to test pass behavior across function boundaries */
__attribute__((noinline)) 
int process_struct_array_with_mixed_access(struct Data *arr, int n) {
    int total = 0;
    struct Data *current = arr;
    
    /* Pattern 3: Structure field access with pointer arithmetic */
    for (int i = 0; i < n; i++) {
        /* Access different fields creating complex addressing */
        total += current->value;
        total += current->tag;
        
        /* Mixed: update pointer then use it */
        current = current + 1;  /* Base register update by structure size */
    }
    return total;
}

/* Volatile pointer walk - inhibits some optimizations but still shows pattern */
__attribute__((noinline))
int volatile_pointer_walk(volatile int *arr, int n) {
    int sum = 0;
    volatile int *ptr = arr;
    
    /* Pattern 4: Volatile access with predictable stride */
    for (int i = 0; i < n; i++) {
        sum += *ptr;  /* Volatile load */
        ptr = ptr + 1;  /* Still creates base+offset pattern */
    }
    return sum;
}

/* Combined pointer/index access pattern */
static int mixed_array_access(int *arr, int n) {
    int sum = 0;
    int *base_ptr = arr;
    int index = 0;
    
    /* Pattern 5: Mixed pointer and index arithmetic */
    while (index < n) {
        /* Create pattern: base register used, then updated */
        int *access_ptr = base_ptr + index;  /* Base + scaled index */
        sum += *access_ptr;
        
        /* Update index, creating potential for reg+0 pattern */
        index = index + 1;
        
        /* Also update base pointer occasionally */
        if (index % 16 == 0) {
            base_ptr = arr + (index / 2);  /* Non-linear but still shows pattern */
        }
    }
    return sum;
}

/* Complex nested structure access */
__attribute__((noinline))
int nested_structure_access(struct Data *arr, int n) {
    int total = 0;
    
    /* Pattern 6: Array of structures with field access */
    for (int i = 0; i < n; i++) {
        /* Multiple addressing modes within loop */
        struct Data *item = &arr[i];  /* Base + scaled offset */
        total += item->value;
        
        /* Pointer arithmetic on character array within structure */
        char *payload_ptr = item->payload;
        for (int j = 0; j < 8; j++) {
            total += *payload_ptr;
            payload_ptr = payload_ptr + 1;  /* Inner loop pointer increment */
        }
    }
    return total;
}

/* Main driver with register pressure */
int main() {
    int result = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        int_array[i] = i * 3 + 1;
    }
    
    for (int i = 0; i < STRUCT_SIZE; i++) {
        struct_array[i].value = i * 2;
        struct_array[i].tag = i % 10;
        for (int j = 0; j < 8; j++) {
            struct_array[i].payload[j] = (i + j) % 256;
        }
    }
    
    /* Test 1: Simple pointer increment (likely creates reg+0 pattern) */
    result += sum_with_pointer_increment(int_array, SIZE);
    
    /* Test 2: Pointer decrement pattern */
    result += reverse_with_pointer_decrement(int_array, SIZE / 2);
    
    /* Test 3: Structure access with mixed patterns */
    result += process_struct_array_with_mixed_access(struct_array, STRUCT_SIZE / 2);
    
    /* Test 4: Volatile access pattern */
    result += volatile_pointer_walk(int_array, SIZE / 4);
    
    /* Test 5: Mixed pointer/index arithmetic */
    result += mixed_array_access(int_array + 64, SIZE - 64);
    
    /* Test 6: Nested structure access */
    result += nested_structure_access(struct_array, STRUCT_SIZE / 4);
    
    /* Add some register pressure between calls */
    int temp = result;
    for (int i = 0; i < 100; i++) {
        temp = temp * 1103515245 + 12345;  /* Simple LCG to create live values */
    }
    result ^= temp;  /* Ensure values are live */
    
    printf("Final result: %d\n", result);
    return result != 0 ? 0 : 1;
}

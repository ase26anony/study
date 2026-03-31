/* Test program for GCC expr.cc constant bounds initialization coverage */
#include <stdio.h>

/* For count <= 2 path with !MEM_P(target) */
struct SmallReg {
    int a;
    int b;
};

/* For count > 2 path with MEM_P(target) */
struct PackedStruct {
    int a:7;
    int b:9;
    int c:16;
} __attribute__((packed));

/* Static initialization - MEM_P(target) true */
static int static_array[100] = {[10 ... 90] = 99};

/* Multi-dimensional array with constant bounds */
int md_array[3][4] = {[0 ... 1][2 ... 3] = 5};

/* Struct with array member */
struct WithArray {
    int x;
    int arr[5];
};

/* Enum for constant bounds */
enum { LOWER = 2, UPPER = 5 };

int main(void) {
    /* 1. Register target with count <= 2 - should trigger !MEM_P(target) path */
    register struct SmallReg reg_target = {[0 ... 1] = 42};
    
    /* 2. Automatic array with constant range (count > 2, MEM_P(target) true) */
    int auto_array[20] = {[LOWER ... UPPER] = 7};
    
    /* 3. Volatile array to ensure MEM_P(target) */
    volatile int volatile_array[10] = {[1 ... 3] = 11};
    
    /* 4. Packed struct array - tests TYPE_SIZE constant fitting */
    struct PackedStruct packed_array[4] = {[0 ... 2] = {1, 2, 3}};
    
    /* 5. Nested block with different context */
    {
        /* Single element range (count = 1) */
        int single[5] = {[2] = 100};
        
        /* Two element range (count = 2) */
        int double_range[10] = {[3 ... 4] = 200};
        
        /* Use values to prevent elimination */
        printf("Single: %d, Double: %d\n", single[2], double_range[3]);
    }
    
    /* 6. Struct with array member initialization */
    struct WithArray s = {.arr = {[1 ... 3] = 77}};
    
    /* 7. Compound literal assignment */
    int *ptr = (int[5]){[0 ... 4] = 55};
    
    /* 8. Multi-dimensional with complex range */
    int complex_md[4][5] = {[0 ... 2][1 ... 3] = 33};
    
    /* Prevent dead code elimination */
    int sum = 0;
    sum += reg_target.a + reg_target.b;
    sum += static_array[10] + static_array[90];
    sum += auto_array[LOWER] + auto_array[UPPER];
    sum += volatile_array[1];
    sum += packed_array[0].a;
    sum += s.arr[1];
    sum += ptr[0];
    sum += complex_md[0][1];
    sum += md_array[0][2];
    
    printf("Sum: %d\n", sum);
    
    /* 9. Conditional initialization with constant condition */
    if (1) {  /* Always true */
        int cond_array[8] = {[2 ... 6] = 888};
        printf("Cond array[3]: %d\n", cond_array[3]);
    }
    
    /* 10. Switch with constant case */
    switch (3) {
        case 3: {
            int switch_array[7] = {[1 ... 5] = 999};
            printf("Switch array[2]: %d\n", switch_array[2]);
            break;
        }
    }
    
    return 0;
}

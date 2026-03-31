/* Test for expr.cc lines 7691-7700 - constant-bounded memory operations */

#include <stdio.h>
#include <string.h>

/* Test 1: MEM target with count <= 2 */
static int test_mem_small_count(void) {
    /* Initialize first 2 elements of an array - count = 2 */
    int arr1[10] = {0};
    int arr2[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    /* Copy 2 elements - constant bounds */
    arr1[0] = arr2[0];
    arr1[1] = arr2[1];
    
    /* Also test single element copy */
    int single = arr2[5];
    arr1[5] = single;
    
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += arr1[i];
    }
    return sum;
}

/* Test 2: MEM target with count > 2 but small total size */
static int test_mem_small_total_size(void) {
    /* 10 chars = 10 bytes total - should trigger TYPE_SIZE * count path */
    char buffer1[10];
    char buffer2[10] = "123456789";
    
    /* Copy all 10 chars - constant bounds */
    for (int i = 0; i < 10; i++) {
        buffer1[i] = buffer2[i];
    }
    
    /* Also test with short type */
    short shorts1[5];
    short shorts2[5] = {10, 20, 30, 40, 50};
    
    for (int i = 0; i < 5; i++) {
        shorts1[i] = shorts2[i];
    }
    
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += buffer1[i];
    }
    for (int i = 0; i < 5; i++) {
        sum += shorts1[i];
    }
    return sum;
}

/* Test 3: Non-MEM target (register operations) */
static int test_non_mem_target(void) {
    /* Bit-field extraction into register */
    struct bitfield {
        unsigned int a : 4;
        unsigned int b : 4;
        unsigned int c : 4;
        unsigned int d : 4;
    } bf = {1, 2, 3, 4};
    
    /* Extract multiple bit-fields - these go into registers */
    unsigned int reg1 = bf.a;
    unsigned int reg2 = bf.b;
    unsigned int reg3 = bf.c;
    unsigned int reg4 = bf.d;
    
    /* Pack values into a larger integer */
    unsigned int packed = (reg4 << 12) | (reg3 << 8) | (reg2 << 4) | reg1;
    
    return (int)packed;
}

/* Test 4: Array initialization with compound literal */
static int test_compound_literal(void) {
    /* Constant-sized initialization */
    int arr[4];
    
    /* Copy from compound literal - constant bounds */
    int *src = (int[]){10, 20, 30, 40};
    for (int i = 0; i < 4; i++) {
        arr[i] = src[i];
    }
    
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += arr[i];
    }
    return sum;
}

/* Test 5: Structure copy with constant size */
static int test_struct_copy(void) {
    struct small {
        char a;
        char b;
        char c;
    } s1 = {'x', 'y', 'z'};
    
    struct small s2;
    
    /* Copy entire small struct - constant bounds */
    s2 = s1;
    
    return s2.a + s2.b + s2.c;
}

/* Test 6: Mixed operations in loop with constant iteration */
static int test_constant_loop(void) {
    int result = 0;
    
    /* Loop with constant iteration count - may get unrolled */
    for (int i = 0; i < 3; i++) {
        int temp[2] = {i * 10, i * 10 + 1};
        result += temp[0] + temp[1];
    }
    
    return result;
}

/* Test 7: Using enum for constant bounds */
static int test_enum_bounds(void) {
    enum { SIZE = 4 };
    int arr[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i * 100;
    }
    
    int sum = 0;
    for (int i = 0; i < SIZE; i++) {
        sum += arr[i];
    }
    return sum;
}

/* Test 8: Pointer arithmetic with constant offsets */
static int test_pointer_arithmetic(void) {
    int data[8] = {1, 2, 4, 8, 16, 32, 64, 128};
    int copy[4];
    
    /* Copy slice using pointer arithmetic - constant bounds */
    int *src = &data[2];  /* Start at index 2 */
    for (int i = 0; i < 4; i++) {
        copy[i] = src[i];  /* Copy 4 elements */
    }
    
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += copy[i];
    }
    return sum;
}

int main(void) {
    int total = 0;
    
    total += test_mem_small_count();        /* MEM target, count <= 2 */
    total += test_mem_small_total_size();   /* MEM target, small total size */
    total += test_non_mem_target();         /* Non-MEM target */
    total += test_compound_literal();       /* Compound literal copy */
    total += test_struct_copy();            /* Small struct copy */
    total += test_constant_loop();          /* Constant loop unrolling */
    total += test_enum_bounds();            /* Enum-based bounds */
    total += test_pointer_arithmetic();     /* Pointer arithmetic slice */
    
    printf("Result: %d\n", total);
    return 0;
}

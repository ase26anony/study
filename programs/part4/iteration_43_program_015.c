/* Test for GCC expr.cc lines 7691-7700 - constant-bounded memory operations */

#include <stdio.h>
#include <string.h>

/* Test 1: Non-MEM target - bitfield extraction into register */
/* This should trigger !MEM_P(target) path */
static int test_bitfield_extraction(void) {
    struct S {
        unsigned int a : 3;
        unsigned int b : 5;
        unsigned int c : 8;
    } s = {2, 10, 100};
    
    /* Extracting multiple bitfields - compiler may use constant bounds */
    unsigned int result = 0;
    result |= s.a;          /* count = 1, non-MEM target */
    result |= (s.b << 3);   /* count = 1, non-MEM target */
    result |= (s.c << 8);   /* count = 1, non-MEM target */
    
    return result;  /* Should be 2 | (10 << 3) | (100 << 8) = 25610 */
}

/* Test 2: MEM target with count <= 2 */
/* Initialize first 2 elements of array - triggers count <= 2 */
static int test_small_array_init(void) {
    int arr[10];
    
    /* Constant bounds: lo=0, hi=1, count=2 */
    arr[0] = 42;    /* count = 1 */
    arr[1] = 43;    /* count = 1 */
    
    /* Also test with compound literal for constant initialization */
    int arr2[2] = {100, 200};  /* count = 2 */
    
    return arr[0] + arr[1] + arr2[0] + arr2[1];  /* 42+43+100+200 = 385 */
}

/* Test 3: MEM target with count > 2 but small total size */
/* char array with 10 elements - TYPE_SIZE=8 bits, count=10, total=80 bits */
static int test_char_array_init(void) {
    char buffer[10];
    
    /* Initialize with constant values - compiler knows bounds are 0..9 */
    for (int i = 0; i < 10; i++) {
        buffer[i] = (char)(i + 'A');  /* 'A' through 'J' */
    }
    
    /* Also test direct constant initialization */
    char data[5] = {1, 2, 3, 4, 5};  /* count=5, TYPE_SIZE=8, total=40 bits */
    
    int sum = 0;
    for (int i = 0; i < 10; i++) sum += buffer[i];
    for (int i = 0; i < 5; i++) sum += data[i];
    
    return sum;  /* Sum of ASCII values A-J + 1+2+3+4+5 */
}

/* Test 4: Structure copy with constant size */
/* Structure small enough to trigger the condition */
static int test_struct_copy(void) {
    struct Point {
        short x;    /* 16 bits */
        short y;    /* 16 bits */
        char z;     /* 8 bits */
    };  /* Total: 40 bits */
    
    struct Point p1 = {100, 200, 50};
    struct Point p2;
    
    /* Constant-sized copy: count=3 elements? Or treated as single block? */
    /* Let's try array of structs to force element-wise thinking */
    struct Point points[2] = {{1, 2, 3}, {4, 5, 6}};
    struct Point copy[2];
    
    /* This should have constant bounds: lo=0, hi=1, count=2 */
    for (int i = 0; i < 2; i++) {
        copy[i] = points[i];
    }
    
    return p1.x + p2.x + copy[0].x + copy[1].x;  /* 100+0+1+4 = 105 */
}

/* Test 5: Mixed types with constant indices */
static int test_mixed_types(void) {
    /* Array of different integer types with constant indices */
    short shorts[3] = {10, 20, 30};      /* TYPE_SIZE=16, count=3, total=48 */
    int ints[2] = {100, 200};            /* TYPE_SIZE=32, count=2, total=64 */
    char chars[8] = "ABCDEFG";           /* TYPE_SIZE=8, count=8, total=64 */
    
    /* Access with constant indices */
    int result = shorts[0] + shorts[2];  /* 10 + 30 = 40 */
    result += ints[1];                   /* + 200 = 240 */
    result += chars[3];                  /* + 'D' (68) = 308 */
    
    /* Constant slice copy using memcpy with constant size */
    char dest[4];
    /* Compiler knows this copies 4 chars = 32 bits */
    __builtin_memcpy(dest, chars + 2, 4);
    
    result += dest[0];  /* + 'C' (67) = 375 */
    
    return result;
}

/* Test 6: Using enums for constant bounds */
static int test_enum_bounds(void) {
    enum { START = 0, END = 3, COUNT = END - START + 1 };
    
    int array[10];
    
    /* Constant bounds from enum */
    for (int i = START; i <= END; i++) {
        array[i] = i * 10;  /* lo=0, hi=3, count=4 */
    }
    
    /* Also test with switch to force constant index analysis */
    int val = 0;
    switch (array[2]) {
        case 20: val = 1; break;
        default: val = 0; break;
    }
    
    return array[0] + array[3] + val;  /* 0 + 30 + 1 = 31 */
}

/* Test 7: Pointer arithmetic with constant offsets */
static int test_pointer_arithmetic(void) {
    int data[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int *ptr = data;
    
    /* Constant offset accesses */
    int a = ptr[2];   /* index 2 */
    int b = ptr[5];   /* index 5 */
    
    /* Constant bounds copy using pointer arithmetic */
    int copy[3];
    for (int i = 0; i < 3; i++) {
        copy[i] = ptr[i + 1];  /* lo=1, hi=3, count=3 */
    }
    
    return a + b + copy[0] + copy[2];  /* 2 + 5 + 1 + 3 = 11 */
}

int main(void) {
    int total = 0;
    
    total += test_bitfield_extraction();  /* 25610 */
    total += test_small_array_init();     /* + 385 = 25995 */
    total += test_char_array_init();      /* + 755 = 26750 */
    total += test_struct_copy();          /* + 105 = 26855 */
    total += test_mixed_types();          /* + 375 = 27230 */
    total += test_enum_bounds();          /* + 31 = 27261 */
    total += test_pointer_arithmetic();   /* + 11 = 27272 */
    
    printf("Result: %d\n", total);
    
    /* Verify expected total */
    if (total == 27272) {
        printf("All tests passed!\n");
        return 0;
    } else {
        printf("Unexpected result: %d\n", total);
        return 1;
    }
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global variable for variant 3 */
volatile int global_cond = 0;

/* Variant 1: Integer condition modified in the then block */
int variant1_modify_in_then(int x, int y) {
    int result;
    /* Simple if-else pattern that if-conversion would like to convert */
    if (x > 0) {
        result = y + 10;
        x = 5;  /* Modifies the condition variable x */
    } else {
        result = y - 10;
    }
    /* Use modified x to prevent dead store elimination */
    return result + (x % 2);
}

/* Variant 1b: Integer condition modified in the else block */
int variant1_modify_in_else(int x, int y) {
    int result;
    if (x <= 0) {
        result = y + 20;
    } else {
        result = y - 20;
        x = -1;  /* Modifies condition variable in else block */
    }
    return result + (x % 3);
}

/* Variant 2: Pointer comparison where pointer is modified */
int variant2_pointer_modify(char *ptr, int offset) {
    char buffer[100];
    char *result_ptr;
    
    /* Pattern: if (ptr != NULL) { ... } else { ... } */
    if (ptr != NULL) {
        result_ptr = ptr + offset;
        ptr = NULL;  /* Modifies the condition pointer */
    } else {
        result_ptr = buffer;
    }
    
    /* Use both to prevent optimization */
    return (int)(*result_ptr) + (ptr == NULL ? 1 : 0);
}

/* Variant 3: Global variable condition modified inside block */
int variant3_global_modify(int value) {
    int result;
    /* Use volatile global as condition */
    if (global_cond > 0) {
        result = value * 2;
        global_cond = -1;  /* Modifies global condition */
    } else {
        result = value / 2;
        global_cond = 1;   /* Also modifies in else */
    }
    return result;
}

/* Variant 4: Multiple modifications in complex then block */
int variant4_multiple_modifications(int a, int b, int c) {
    int result;
    /* if-conversion candidate: same variable assigned in both arms */
    if (a + b > c) {
        result = a * b;
        a = c - b;      /* First modification of a (used in condition) */
        b = a + 1;      /* Modification of b (also used in condition) */
        a = b * 2;      /* Second modification of a */
    } else {
        result = a + b + c;
    }
    /* Use all modified variables */
    return result + a + b;
}

/* Variant 5: Nested condition with modification */
int variant5_nested_condition_modify(int x, int y, int z) {
    int result;
    /* Outer if that's a candidate for if-conversion */
    if (x > y) {
        result = z * 3;
        /* Inner if that shouldn't prevent outer if-conversion attempt */
        if (z > 0) {
            x = y;  /* Modifies outer condition variable */
        }
    } else {
        result = z / 3;
    }
    return result + x;
}

/* Variant 6: Loop with if inside - if-conversion might try to convert the if */
int variant6_loop_with_modifying_if(int n, int seed) {
    int sum = 0;
    int i;
    for (i = 0; i < n; i++) {
        int temp = seed + i;
        /* This if inside loop is a candidate for if-conversion */
        if (temp % 2 == 0) {
            sum += temp * 2;
            seed = i;  /* Modifies variable used in condition */
        } else {
            sum += temp;
        }
    }
    return sum;
}

/* Main function that exercises all variants */
int main(int argc, char **argv) {
    int total = 0;
    int seed;
    
    /* Use argv for input-dependent values to prevent constant folding */
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = 42;  /* Default seed */
    }
    
    /* Initialize global */
    global_cond = seed % 3;
    
    /* Test all variants with input-dependent values */
    total += variant1_modify_in_then(seed, seed + 1);
    total += variant1_modify_in_else(seed - 5, seed + 2);
    
    char test_char = 'A' + (seed % 26);
    total += variant2_pointer_modify(&test_char, seed % 10);
    
    total += variant3_global_modify(seed);
    total += variant4_multiple_modifications(seed, seed + 3, seed + 7);
    total += variant5_nested_condition_modify(seed, seed - 2, seed + 4);
    total += variant6_loop_with_modifying_if(10, seed);
    
    /* Print result to prevent optimization */
    printf("Total result: %d\n", total);
    
    return total % 256;
}

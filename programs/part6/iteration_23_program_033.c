/* Main test suite to exercise if-conversion with condition modification */
#include <stdio.h>
#include <stdlib.h>

/* Global variable for one of the test cases */
volatile int global_cond = 0;

/* Test 1: Integer condition modified in the 'then' block */
int test1_modify_condition_in_then(int x, int y) {
    int result;
    
    /* Simple if-else pattern that if-conversion would like to convert */
    if (x > 0) {
        result = y * 2;
        x = 0;  /* MODIFIES CONDITION VARIABLE - should trigger modified_in_p */
    } else {
        result = y / 2;
    }
    
    /* Use modified x to prevent dead store elimination */
    return result + x;
}

/* Test 2: Pointer condition modified in the 'then' block */
int test2_modify_pointer_condition(char *ptr, int threshold) {
    int sum = 0;
    
    /* Pattern: if (ptr != NULL) { ... } */
    if (ptr != NULL) {
        sum = *ptr + 10;
        ptr = NULL;  /* MODIFIES POINTER CONDITION - should trigger modified_in_p */
    } else {
        sum = threshold;
    }
    
    /* Use ptr to prevent optimization */
    return sum + (ptr == NULL ? 0 : 1);
}

/* Test 3: Global variable condition modified in conditional block */
int test3_modify_global_condition(int value) {
    int output;
    
    /* Use global variable in condition */
    if (global_cond > 5) {
        output = value * 3;
        global_cond = 0;  /* MODIFIES GLOBAL CONDITION - should trigger modified_in_p */
    } else {
        output = value + 5;
    }
    
    return output;
}

/* Test 4: Multiple modifications of condition variable */
int test4_multiple_modifications(int a, int b) {
    int res;
    
    /* Attractive for if-conversion: both arms write to 'res' */
    if (a < b) {
        res = a * b;
        a = b;      /* First modification */
        a += 1;     /* Second modification - still in 'then' block */
    } else {
        res = a - b;
    }
    
    return res + a;  /* Use modified 'a' */
}

/* Test 5: Condition variable modified through pointer */
int test5_indirect_modification(int *cond_ptr, int x, int y) {
    int result;
    
    /* Condition depends on dereferenced pointer */
    if (*cond_ptr > 10) {
        result = x + y;
        *cond_ptr = 5;  /* MODIFIES CONDITION THROUGH POINTER - should trigger modified_in_p */
    } else {
        result = x - y;
    }
    
    return result;
}

/* Test 6: Complex condition with modification */
int test6_complex_condition_mod(int a, int b, int c) {
    int val;
    
    /* More complex condition that's still if-convertible */
    if ((a > b) && (b < c)) {
        val = a * c;
        a = b;  /* MODIFIES PART OF CONDITION (a) */
    } else {
        val = b * c;
    }
    
    return val + a;
}

/* Test 7: Nested condition variable usage with modification */
int test7_nested_condition_usage(int x, int y, int z) {
    int output;
    
    /* Pattern that looks good for if-conversion */
    if (x != y) {
        output = z * 2;
        x = y;  /* Makes condition false - modification in 'then' block */
        
        /* Additional operation that doesn't affect coverage */
        output += 1;
    } else {
        output = z / 2;
    }
    
    /* Use modified x to prevent optimization */
    if (x == y) {
        output += 100;
    }
    
    return output;
}

/* Test 8: Condition variable modified in both branches */
int test8_modify_in_both_branches(int flag, int value) {
    int result;
    
    if (flag) {
        result = value * 10;
        flag = 0;  /* Modification in 'then' block */
    } else {
        result = value + 10;
        flag = 1;  /* Modification in 'else' block - also should be checked */
    }
    
    return result + flag;
}

/* Main function that runs all tests with input-dependent values */
int main(int argc, char *argv[]) {
    int seed = 0;
    int total = 0;
    
    /* Use command line argument or default seed */
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Initialize test values with input-dependent or volatile values */
    volatile int v1 = seed;
    volatile int v2 = seed + 1;
    volatile int v3 = seed + 2;
    char buffer[10] = "test";
    char *ptr = buffer;
    
    /* Set global condition */
    global_cond = seed % 10;
    
    /* Run all tests and accumulate results */
    total += test1_modify_condition_in_then(v1, v2);
    total += test2_modify_pointer_condition(ptr, v3);
    total += test3_modify_global_condition(v1);
    
    int cond_var = v2 * 2;
    total += test4_multiple_modifications(v1, v2);
    total += test5_indirect_modification(&cond_var, v2, v3);
    total += test6_complex_condition_mod(v1, v2, v3);
    total += test7_nested_condition_usage(v1, v2, v3);
    total += test8_modify_in_both_branches(v1 % 2, v2);
    
    /* Print result to prevent optimization */
    printf("Total result: %d\n", total);
    
    return total != 0 ? 0 : 1;
}

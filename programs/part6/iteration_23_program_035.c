/* Main test driver that calls all test functions */
#include <stdio.h>
#include <stdlib.h>

/* Forward declarations for test functions */
int test1_modify_condition_in_then(int x, int y);
int test2_modify_pointer_in_then(int *ptr, int threshold);
int test3_global_condition_modification(int val);
int test4_compound_condition_modification(int a, int b, int c);
int test5_volatile_condition_modification(volatile int v);
int test6_nested_condition_modification(int x, int y);
int test7_array_index_condition_modification(int idx, int *arr);
int test8_bitwise_condition_modification(int x, int mask);

/* Global variable for test3 */
int global_cond = 0;

int main(int argc, char *argv[]) {
    int seed = 0;
    int result = 0;
    
    /* Use command line argument as seed to prevent constant folding */
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Test 1: Simple integer condition modified in then block */
    int x = seed + 1;
    int y = seed + 2;
    result += test1_modify_condition_in_then(x, y);
    
    /* Test 2: Pointer comparison with modification */
    int arr[3] = {seed, seed + 10, seed + 20};
    result += test2_modify_pointer_in_then(arr, 15);
    
    /* Test 3: Global variable condition */
    global_cond = seed % 10;
    result += test3_global_condition_modification(seed);
    
    /* Test 4: Compound condition */
    result += test4_compound_condition_modification(seed, seed + 1, seed + 2);
    
    /* Test 5: Volatile condition */
    volatile int v = seed;
    result += test5_volatile_condition_modification(v);
    
    /* Test 6: Nested modification */
    result += test6_nested_condition_modification(seed, seed + 5);
    
    /* Test 7: Array index condition */
    int arr2[5] = {seed, seed+1, seed+2, seed+3, seed+4};
    result += test7_array_index_condition_modification(2, arr2);
    
    /* Test 8: Bitwise condition */
    result += test8_bitwise_condition_modification(seed, 0x0F);
    
    printf("Total result: %d\n", result);
    return result;
}

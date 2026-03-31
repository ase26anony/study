/* Test case 1: Integer condition modified in then block */
#include <stdio.h>
#include <stdlib.h>

int test1_modify_in_then(int x, int y) {
    int result;
    /* Simple if-else pattern that if-conversion would consider */
    if (x > 0) {
        result = y * 2;
        x = 5;  /* Modifies the condition variable x */
    } else {
        result = y + 10;
    }
    return result + x;  /* Use x to prevent dead store elimination */
}

int test1_modify_in_else(int x, int y) {
    int result;
    /* Another pattern with modification in else block */
    if (x <= 0) {
        result = y - 5;
    } else {
        result = y * 3;
        x = -1;  /* Modifies condition variable in else block */
    }
    return result * (x + 1);  /* Use x to prevent optimization */
}

int main(int argc, char **argv) {
    volatile int seed = argc > 1 ? atoi(argv[1]) : 42;
    int x = seed % 10 - 5;  /* Range -5 to 4 */
    int y = seed % 20 + 1;  /* Range 1 to 20 */
    
    int sum = 0;
    sum += test1_modify_in_then(x, y);
    sum += test1_modify_in_else(x, y);
    
    printf("Test1 result: %d\n", sum);
    return sum % 256;
}

/* test1.c - Integer condition modified in then block */
#include <stdio.h>

int test1(int x, int y) {
    int result;
    
    /* Simple if-else pattern that if-conversion would like to convert */
    if (x > 0) {
        /* This modifies the condition variable x */
        x = 5;  // This should trigger modified_in_p check
        result = y + 10;
    } else {
        result = y - 10;
    }
    
    /* Use both x and result to prevent optimization */
    return result + (x % 2);
}

/* Another variant with different operation */
int test1b(int a, int b) {
    int res;
    
    if (a != 0) {
        a = a * 2;  // Modifies condition variable
        res = b * 3;
    } else {
        res = b / 2;
    }
    
    return res + a;
}

int main_test1(int argc, char **argv) {
    volatile int seed = argc;
    int x = seed;
    int y = seed * 2;
    
    int sum = 0;
    sum += test1(x, y);
    sum += test1b(x, y);
    
    printf("Test1 result: %d\n", sum);
    return sum;
}

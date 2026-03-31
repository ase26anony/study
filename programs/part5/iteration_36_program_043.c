#include <stdio.h>

/* Declarations from the test files */
int compute1(int x);
int compute2(int x);
int compute3(int x);
void helper_function(void);

int main() {
    int result1 = compute1(5);
    int result2 = compute2(10);
    int result3 = compute3(15);
    
    printf("Result 1: %d\n", result1);
    printf("Result 2: %d\n", result2);
    printf("Result 3: %d\n", result3);
    
    helper_function();
    
    return 0;
}

#include <stdio.h>

extern int compute1(int x);
extern int compute2(int x);
extern int compute3(int x);

int main() {
    int result1 = compute1(5);
    int result2 = compute2(10);
    int result3 = compute3(15);
    
    printf("Results: %d, %d, %d\n", result1, result2, result3);
    printf("Sum: %d\n", result1 + result2 + result3);
    
    return 0;
}

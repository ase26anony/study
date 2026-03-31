#include <stdio.h>

// Declarations from the test files
int compute1(int x);
int compute2(int x);
int compute3(int x);
float compute4(float x);

int main() {
    int result1 = compute1(5);
    int result2 = compute2(10);
    int result3 = compute3(15);
    float result4 = compute4(2.5f);
    
    printf("Result1: %d\n", result1);
    printf("Result2: %d\n", result2);
    printf("Result3: %d\n", result3);
    printf("Result4: %.2f\n", result4);
    
    return 0;
}

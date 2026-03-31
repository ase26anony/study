/* main.c - Main program that calls functions from other files */
#include <stdio.h>

/* Declarations from other source files */
int compute1(int x);
int compute2(int x);
int compute3(int x);
float compute4(float x);

int main() {
    int result1 = compute1(5);
    int result2 = compute2(10);
    int result3 = compute3(15);
    float result4 = compute4(3.14f);
    
    printf("Results:\n");
    printf("compute1(5) = %d\n", result1);
    printf("compute2(10) = %d\n", result2);
    printf("compute3(15) = %d\n", result3);
    printf("compute4(3.14) = %.2f\n", result4);
    
    return 0;
}

/* coverage_source.c - Simple program to generate GCOV data */
#include <stdio.h>

int main() {
    int i;
    int sum = 0;
    
    for (i = 0; i < 10; i++) {
        sum += i;
    }
    
    printf("Sum: %d\n", sum);
    
    if (sum > 0) {
        printf("Positive sum\n");
    } else {
        printf("Zero sum\n");
    }
    
    return 0;
}

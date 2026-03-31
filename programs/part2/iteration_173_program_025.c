/* Simple test program to generate GCOV data */
#include <stdio.h>

int main() {
    int i;
    for (i = 0; i < 10; i++) {
        printf("Iteration %d\n", i);
    }
    return 0;
}

#include <stdio.h>

int main() {
    int i;
    for (i = 0; i < 10; i++) {
        printf("Iteration %d\n", i);
    }
    
    if (i > 5) {
        printf("i is greater than 5\n");
    }
    
    return 0;
}

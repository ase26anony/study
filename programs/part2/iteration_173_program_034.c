#include <stdio.h>

int main() {
    int i;
    for (i = 0; i < 10; i++) {
        printf("Iteration: %d\n", i);
    }
    
    if (i > 5) {
        printf("i is greater than 5\n");
    } else {
        printf("i is 5 or less\n");
    }
    
    return 0;
}

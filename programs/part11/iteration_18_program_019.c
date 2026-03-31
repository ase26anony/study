#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    int x = 0;
    
    if (argc > 1) {
        x = atoi(argv[1]);
    }
    
    for (int i = 0; i < x; i++) {
        printf("Iteration %d\n", i);
    }
    
    if (x > 10) {
        printf("Large value: %d\n", x);
    } else {
        printf("Small value: %d\n", x);
    }
    
    return 0;
}

#include <stdio.h>

int main() {
    int i;
    for (i = 0; i < 10; i++) {
        printf("Iteration %d\n", i);
    }
    
    if (i == 10) {
        printf("Loop completed successfully\n");
    } else {
        printf("Unexpected loop termination\n");
    }
    
    return 0;
}

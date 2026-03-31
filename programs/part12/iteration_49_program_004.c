/* Simple test program for gcov-dump testing */
#include <stdio.h>

void helper_function(int x) {
    if (x > 0) {
        printf("Positive: %d\n", x);
    } else if (x < 0) {
        printf("Negative: %d\n", x);
    } else {
        printf("Zero\n");
    }
}

int main() {
    int i;
    
    /* Multiple branches for coverage data */
    for (i = -2; i <= 2; i++) {
        helper_function(i);
    }
    
    /* Another conditional */
    if (i > 0) {
        printf("Loop completed\n");
    }
    
    return 0;
}

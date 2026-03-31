/* Simple test program for gcov-dump testing */
#include <stdio.h>

int helper_function(int x) {
    if (x > 0) {
        return x * 2;
    } else {
        return x - 1;
    }
}

int main() {
    int result = 0;
    
    /* Generate some coverage data */
    for (int i = 0; i < 5; i++) {
        result += helper_function(i);
    }
    
    /* Another branch */
    if (result > 10) {
        printf("Result is %d\n", result);
    } else {
        printf("Small result: %d\n", result);
    }
    
    return 0;
}

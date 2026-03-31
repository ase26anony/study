/* Simple test program for gcov-dump testing */
#include <stdio.h>

int helper(int x) {
    if (x > 0) {
        return x * 2;
    } else {
        return x + 5;
    }
}

int main() {
    int i;
    for (i = -2; i <= 2; i++) {
        printf("helper(%d) = %d\n", i, helper(i));
    }
    return 0;
}

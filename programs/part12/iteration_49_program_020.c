/* Simple test program for gcov-dump testing */
#include <stdio.h>

int helper(int x) {
    if (x > 0) {
        return x * 2;
    } else {
        return x - 1;
    }
}

int main() {
    int i;
    for (i = 0; i < 3; i++) {
        printf("Value: %d -> %d\n", i, helper(i));
    }
    return 0;
}

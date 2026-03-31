#include <stdio.h>

int helper(int x) {
    if (x > 0) {
        return x * 2;
    } else {
        return x - 1;
    }
}

int main() {
    int result = 0;
    
    // Generate some coverage data
    for (int i = 0; i < 5; i++) {
        result += helper(i);
    }
    
    printf("Result: %d\n", result);
    return 0;
}

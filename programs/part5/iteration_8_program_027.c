#include <stdio.h>

int main() {
    int trigger = 1;
    int a = 10, b = 0;
    // Predefine variable for candidate instruction
    int candidate_var = 5;
    
    // Make the branch condition less predictable
    if (trigger > 0) {
        goto TARGET_LABEL;
    }
    
    // Code that might or might not execute
    a = 20;
    b = a * 2;
    
TARGET_LABEL:
    // More complex candidate instruction that's harder to optimize away
    candidate_var = candidate_var + (a > 5 ? 1 : 2);
    
    printf("%d\n", candidate_var);
    return 0;
}

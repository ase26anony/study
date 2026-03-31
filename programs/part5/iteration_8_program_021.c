int main() {
    int trigger = 1;
    int a = 10, b = 0;
    int candidate_var = 5;
    
    // Hoisted: candidate_var = candidate_var + 1;
    candidate_var = 6;  // Even better: constant propagation
    
    if (trigger) goto TARGET_LABEL;  // Still jumps, but target is empty
    
    // Dead code eliminated
TARGET_LABEL:
    printf("%d\n", candidate_var);  // Prints 6
    return 0;
}

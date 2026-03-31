int main() {
    int trigger = 1;
    int a = 10, b = 0;
    int candidate_var = 5;
    
    // Hoisted: candidate_var = candidate_var + 1;
    candidate_var = 6;  // Even better: constant propagation
    
    // Simplified control flow
    goto TARGET_LABEL;  // Or just remove the if entirely
    
    // Dead code eliminated
    // a = 20;  // Removed
    
TARGET_LABEL:
    printf("%d\n", candidate_var);  // candidate_var is already 6
    return 0;
}

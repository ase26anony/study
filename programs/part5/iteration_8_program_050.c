int main() {
    volatile int trigger = 1;  // Prevent compile-time evaluation
    int a = 10, b = 0;
    int candidate_var = 5;
    
    if (trigger) goto TARGET_LABEL;
    
    // Make unreachable code more complex
    for(int i = 0; i < 10; i++) {
        a += i;
    }
    
TARGET_LABEL:
    // Multiple instructions at jump target
    candidate_var = candidate_var + 1;
    candidate_var = candidate_var * 2;
    printf("%d\n", candidate_var);
    
    return 0;
}

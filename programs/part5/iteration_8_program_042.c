int main() {
    int trigger = 1;
    int a = 10, b = 0;
    int candidate_var = 5;
    
    // Hoist the computation if it doesn't have side effects
    candidate_var = candidate_var + 1;
    
    if (trigger) goto TARGET_LABEL;
    a = 20;
TARGET_LABEL:
    printf("%d\n", candidate_var);
    return 0;
}

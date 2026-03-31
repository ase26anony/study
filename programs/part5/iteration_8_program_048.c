// Possible optimized version:
int main() {
    int trigger = 1;
    int a = 10, b = 0;
    int candidate_var = 5;
    
    // Hoist the computation before the branch
    candidate_var = 6;  // Precomputed: 5 + 1
    
    if (trigger) goto TARGET_LABEL;
    a = 20;
    
TARGET_LABEL:
    printf("%d\n", candidate_var);  // Already has value 6
    return 0;
}

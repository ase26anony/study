// Possible optimized version:
int main() {
    int trigger = 1;
    int a = 10, b = 0;
    int candidate_var = 5;
    
    // Hoist the computation before the branch
    int temp = candidate_var + 1;
    
    if (trigger) goto TARGET_LABEL;
    a = 20;
    
TARGET_LABEL:
    candidate_var = temp;  // Just assign the precomputed value
    printf("%d\n", candidate_var);
    return 0;
}

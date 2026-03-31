int main() {
    int trigger = 1;
    int a = 10, b = 0;
    int candidate_var = 5;
    
    // Speculatively execute the candidate instruction
    candidate_var = candidate_var + 1;  // Moved up
    
    if (trigger) goto TARGET_LABEL;
    a = 20;
TARGET_LABEL:
    printf("%d\n", candidate_var);
    return 0;
}

int main() {
    int trigger = 1;
    int a = 10, b = 0;
    int candidate_var = 5;
    
    // Candidate instruction moved up (speculative execution)
    candidate_var = candidate_var + 1;
    
    if (trigger) goto TARGET_LABEL;
    // Dead code eliminated
    a = 20;
TARGET_LABEL:
    printf("%d\n", candidate_var);
    return 0;
}

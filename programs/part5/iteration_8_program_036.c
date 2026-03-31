int main() {
    int trigger = 1;
    int a = 10, b = 0;
    // Predefine variable for candidate instruction
    int candidate_var = 5;
    
    if (trigger) {
        goto TARGET_LABEL;
    }
    
    // Fall-through path that reaches TARGET_LABEL
    a = 20;
    // No jump here - execution falls through to the label
    
TARGET_LABEL:
    candidate_var = candidate_var + 1; // Now this is on both paths
    printf("%d\n", candidate_var);
    return 0;
}

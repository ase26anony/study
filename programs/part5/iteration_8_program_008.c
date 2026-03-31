int main() {
    int trigger = 1;      // Always true
    int a = 10, b = 0;
    int candidate_var = 5;
    
    if (trigger) goto TARGET_LABEL;  // Always jumps
    
    a = 20;  // Never executed
    
TARGET_LABEL:
    candidate_var = candidate_var + 1;  // candidate_var becomes 6
    printf("%d\n", candidate_var);      // Prints "6"
    return 0;
}

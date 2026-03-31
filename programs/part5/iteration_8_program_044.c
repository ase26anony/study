int main() {
    int trigger = 1;
    int a = 10, b = 0;
    int candidate_var = 5;
    
    // Some computation that doesn't depend on candidate_var
    b = a * 2;
    
    if (trigger) goto TARGET_LABEL;
    
    // More independent computation
    a = a + 5;
    
TARGET_LABEL:
    candidate_var = candidate_var + 1; // Could this be moved up?
    printf("%d\n", candidate_var + b); // Now depends on both
    return 0;
}

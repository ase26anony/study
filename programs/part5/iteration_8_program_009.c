int main() {
    int trigger = 1;
    int a = 10, b = 0;
    int candidate_var = 5;
    
    // Moved candidate instruction before the branch
    candidate_var = candidate_var + 1;
    
    if (trigger) {
        // Jump target becomes empty or eliminated
    } else {
        a = 20;  // This block is now truly dead code
    }
    
    printf("%d\n", candidate_var);
    return 0;
}

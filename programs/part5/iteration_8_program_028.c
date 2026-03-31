int main() {
    int trigger = 1;
    int a = 10, b = 0;
    int candidate_var = 5;
    
    // Hoisted calculation
    candidate_var = 6;  // Compiler might compute this at compile time
    
    printf("%d\n", candidate_var);
    return 0;
}

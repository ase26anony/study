int main() {
    int trigger = rand() % 2;  // Make it truly conditional
    int a = 10, b = 0;
    int candidate_var = 5;
    
    if (trigger) {
        goto TARGET_LABEL;
    }
    
    // Some real code here that might execute
    a = process_something();
    
TARGET_LABEL:
    candidate_var = candidate_var + 1;  // This executes via jump or fall-through
    printf("%d\n", candidate_var);
    return 0;
}

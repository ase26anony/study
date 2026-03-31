volatile int trigger = 1;
int main() {
    int counter = 0;
    int a = 0;
    
    for (int i = 0; i < 100; ++i) {
        // The compiler might try to move a = 1 here as a speculative execution
        // that works correctly whether the branch is taken or not
        int branch_taken = (trigger > i);
        a = 1;  // Candidate for delay slot filling
        
        if (branch_taken) {
            goto target_label;
        }
        // If branch not taken, 'a' was set but not used - harmless
        continue;
        
    target_label:
        counter += a;  // Uses the value set in delay slot
    }
    
    return counter > 50 ? 0 : 1;
}

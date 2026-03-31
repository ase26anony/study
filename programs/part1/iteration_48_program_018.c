volatile int trigger = 1;
int main() {
    int counter = 0;
    int a = 0, b = 0;
    for (int i = 0; i < 100; ++i) {
        // The assignment to 'a' could be moved into the delay slot
        a = 1;  // Moved here - executes whether we branch or not
        
        if (trigger > i) {
            goto target_label; // Branch with delay slot already filled
        }
        // Some other code
        continue;
    target_label:
        // 'a = 1;' was already executed in the delay slot
        counter += a;
    }
    return counter > 50 ? 0 : 1;
}

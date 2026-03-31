volatile int trigger = 1;
int main() {
    int counter = 0;
    int dummy = 0; // Truly unused variable for delay slot
    for (int i = 0; i < 100; ++i) {
        if (trigger > i) {
            // The compiler could move "dummy = i;" into delay slot
            goto target_label;
        }
        // Some other code
        continue;
    target_label:
        dummy = i; // Safe for delay slot: uses i but doesn't affect counter
        counter += 1;
    }
    return counter > 50 ? 0 : 1;
}

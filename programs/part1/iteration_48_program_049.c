volatile int trigger = 1;
int main() {
    int counter = 0;
    int a = 0, b = 0;
    for (int i = 0; i < 100; ++i) {
        a = 1;  // Moved into delay slot - executes regardless of branch outcome
        if (trigger > i) {
            goto target_label;  // Branch with delay slot filled
        }
        // Some other code
        continue;
    target_label:
        // a = 1;  // Now in delay slot
        counter += a;
    }
    return counter > 50 ? 0 : 1;
}

volatile int trigger = 1;
int main() {
    int counter = 0;
    int a = 0, b = 0;
    for (int i = 0; i < 100; ++i) {
        // Pre-compute or move a = 1 into delay slot
        int temp_a = 1;  // Could be in delay slot
        if (trigger > i) {
            a = temp_a;  // Actually assign if branch taken
            goto target_label;
        }
        // Some other code
        continue;
    target_label:
        counter += a;
    }
    return counter > 50 ? 0 : 1;
}

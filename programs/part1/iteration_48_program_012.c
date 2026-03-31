volatile int trigger = 1;
int main() {
    int counter = 0;
    int a = 0, b = 0;
    for (int i = 0; i < 100; ++i) {
        a = 0; // Initialize a before the branch
        if (trigger > i) {
            goto target_label;
        }
        // Some other code
        continue;
    target_label:
        a = 1; // Now safe for delay slot (a already initialized)
        counter += a;
    }
    return counter > 50 ? 0 : 1;
}

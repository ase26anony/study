volatile int trigger = 1;
int main() {
    int counter = 0;
    int a = 0, b = 0;
    for (int i = 0; i < 100; ++i) {
        if (trigger > i) {
            a = 1;  // Moved before the branch
            goto target_label;
        }
        // Some other code that doesn't use 'a'
        continue;
    target_label:
        // Delay slot would execute a = 1; here (already executed)
        counter += a;
    }
    return counter > 50 ? 0 : 1;
}

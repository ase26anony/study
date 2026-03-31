volatile int trigger = 1;
int main() {
    int counter = 0;
    int a = 0, b = 0;
    for (int i = 0; i < 100; ++i) {
        if (trigger > i) {
            a = 1;  // This could go in delay slot
            goto target_label;
        }
        // Some other code
        continue;
    target_label:
        counter += a;  // a is always 1 here
    }
    return counter > 50 ? 0 : 1;
}

volatile int trigger = 1;
int main() {
    int counter = 0;
    int a = 0, b = 0;
    for (int i = 0; i < 100; ++i) {
        a = 1;  // Hoisted into potential delay slot
        if (trigger > i) {
            goto target_label;  // Branch with a=1 already executed
        }
        // Some other code
        continue;
    target_label:
        // a=1 was already executed, so just use it
        counter += a;
    }
    return counter > 50 ? 0 : 1;
}

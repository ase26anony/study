volatile int trigger = 1;
int main() {
    int counter = 0;
    int a = 0, b = 0;
    for (int i = 0; i < 100; ++i) {
        if (trigger > i) {
            // Branch delay slot could execute a = 1; here
            goto target_label;
        }
        // Some other code
        // No continue statement here - allows fall-through
    target_label:
        a = 1; // Good candidate for delay slot
        counter += a;
    }
    return counter > 50 ? 0 : 1;
}

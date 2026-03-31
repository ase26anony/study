volatile int trigger = 1;
int main() {
    int counter = 0;
    int a = 0, b = 0;
    for (int i = 0; i < 100; ++i) {
        if (trigger > i) {
            a = 1;  // This could be moved to delay slot
            counter += a;
        } else {
            // Some other code
        }
    }
    return counter > 50 ? 0 : 1;
}

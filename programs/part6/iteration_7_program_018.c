int a, b, c;
int original_a = a;  // Store original if needed
if (a > 0) {
    b = 10;
    // Don't modify a here
} else {
    b = 20;
}
// If you must update a, do it here with clear intent
a = 5;  // But only if this makes sense for your logic

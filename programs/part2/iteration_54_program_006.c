// Option 1: Clear separation
int x = 5;
if (x > 0) {
    x = 10;  // But be aware x's original value was used in the condition
}

// Option 2: Use a temporary variable if logic is complex
int x = 5;
int original_x = x;
if (original_x > 0) {
    x = 10;  // Now it's clear we're not using the modified value
}

// Option 3: Different variable names for clarity
int x = 5;
if (x > 0) {
    int new_value = 10;  // or perform some calculation
    x = new_value;
}

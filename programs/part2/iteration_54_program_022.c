// Option 1: Use a different variable
int x = 5;
if (x > 0) {
    int y = 10;  // Use different variable for result
}

// Option 2: Rename to clarify intent
int input = 5;
int result = input;
if (input > 0) {
    result = 10;
}

// Option 3: If modification is intentional, add comment
int x = 5;
if (x > 0) {  // Check initial value, then update
    x = 10;   // Update based on condition
}

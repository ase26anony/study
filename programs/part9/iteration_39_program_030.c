// After loop peeling optimization:
if (0 < n) {  // First iteration (i == 0)
    // body for i == 0
    i = 1;
}
while (i < n) {  // Remaining iterations (i > 0)
    // No need to check i == 0 here
    i++;
}

// First iteration (peeled)
if (0 < n) {
    // body for i == 0
    i = 1;
}

// Remaining iterations
while (i < n) {
    // This part never executes because i != 0
    i++;
}

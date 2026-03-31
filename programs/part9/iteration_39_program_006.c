// First iteration (peeled)
if (0 < n) {
    // body of if (i == 0)
    i = 1;
}

// Remaining iterations
while (i < n) {
    // i is never 0 here, so the if condition is always false
    i++;
}

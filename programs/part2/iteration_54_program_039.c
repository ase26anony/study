int x = 5;
if (x > 0) {
    x = 10;  // x is now 10, but the condition was evaluated with x=5
    // If we had an else-if checking x > 5 here, it would behave unexpectedly
}

// Case 1: some_value() returns 5
// Loop executes: i=0: cond=5 > 0 → arr[0]=0, cond=0
// i=1..99: cond=0 > 0 → false, nothing happens

// Case 2: some_value() returns -1
// Loop executes: i=0..99: cond=-1 > 0 → false, nothing happens

// Branch 1 (if cond is true)
val1 = 1
goto merge_point

// Branch 2 (if cond is false)  
val2 = 2
goto merge_point

// Merge point
val = φ(val1, val2)  // PHI node: picks val1 or val2 based on which path was taken

// Later use
if (val == 1) ...

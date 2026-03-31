For each pair of loops (loop, other):
    If they don't share any blocks → skip
    Else if other is completely inside loop → other becomes child of loop
    Else if loop is completely inside other → loop becomes child of other

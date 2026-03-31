Gang 0: i = 0..k
  Worker 0: j = 0..m (using vector lanes)
  Worker 1: j = m+1..n (using vector lanes)
  ...
Gang 1: i = k+1..l
  Worker 0: j = 0..m (using vector lanes)
  Worker 1: j = m+1..n (using vector lanes)
  ...

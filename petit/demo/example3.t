auto real a(100)
inout real b(100,100)
!
! The values computed in one iteration of the L1
! loop is not used in any other iteration of the L1
! loop. However, sharing of the array a prevents us
! from running the L1 loop in parallel
!
! Array exapansion/privatization will allow us to run 
! the L1 loop in parallel
!
integer n, m
for L1 = 1 to n do
  for L2 = 2 to m do
    a(L2) = a(L2-1) + b(L1,L2)
  endfor
endfor

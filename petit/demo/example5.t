auto real a(100)
inout real b(100,100)
!
! In this example, there is a flow dependence carried
! by the outer loop
!
integer n, m
for L1 = 1 to n do
  for L2 = L1+1 to m do
    a(L2) = a(L2-1) + b(L1,L2)
  endfor
endfor

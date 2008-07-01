real a(100),b(100),c(100)
integer n, m
!
! Since m might be in the range n+11 <= m <= n+20,
! we can't kill the flow dependence in this example
!
a(m) = b(n)
for L1 = n to n+10 do
  a(L1) = b(L1)
endfor
for L1 = n to n+20 do
  c(L1) = a(L1)
endfor

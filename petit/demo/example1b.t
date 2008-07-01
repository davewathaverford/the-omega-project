real a(100),b(100),c(100)
!
! We have added the assertion m <= n+10
! to the previous example. 
! This allows us to kill te flow dependence
!
integer n, m
assert(m<=n+10);
a(m) = b(n)
for L1 = n to n+10 do
  a(L1) = b(L1)
endfor
for L1 = n to n+20 do
  c(L1) = a(L1)
endfor

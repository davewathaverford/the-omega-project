real a(100),b(100),c(100)
integer n
!
! Since the first loop overwrites a(n), we
! can determine that there is no flow dependence
! from the first write to a(n) to the read of a(L1)
! in the second loop
! 
!
!
a(n) = b(n)
for L1 = n to n+10 do
  a(L1) = b(L1)
endfor
for L1 = n to n+20 do
  c(L1) = a(L1)
endfor

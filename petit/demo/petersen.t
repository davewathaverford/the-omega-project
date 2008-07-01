integer n
real a(1:100,1:100), b(1:100)
real s
!
! It is difficult to expose, or even
! see, the parallelism in this example.
! All loops carry flow dependences.
!
! To expose the parallelism, we must
! use expansion to break storage depemdences
! and use the our unified reordering framework
!
!
for i = 2,n do
  s = 0
  for l = 1,i-1 do
    s = s + a(l,i) * b(l)
  endfor
  b(i) = b(i) - s
endfor

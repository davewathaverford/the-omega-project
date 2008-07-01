real a(101)
!
! This show an example of a linearized array 
! reference. This causes problems for techniques
! such as Banerjee's equalities.
! 
! However, we are able to accurately compute the
! dependence difference for this example, despite the
! linearized reference
!
for i = 1 to 8 do
for j = 1 to 10 do
a(10*i+j) = a(10*(i+2)+j)+7
endfor
endfor

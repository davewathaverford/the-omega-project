common real trigs(144)
common real g(96)
common real w(96)
common integer lot

auto integer i, j, k, l, ibase, jbase, kc
auto real c1, c2, c3, s1, s2, s3
!
! This example shows off our ability to recognize induction variables.
! The original code is immediately below, in comments.
! Below that, in the source file, is another copy of the code.
! However, our induction variable recognition system recognizes the
! induction variables and transforms the code appropriately.
!

! ibase = 3
! jbase = 7
! kc = 5
! do kb = 3, 31, 2 {
!     c1 = trigs(kb)
!     s1 = trigs(kb+1)
!     i = ibase
!     j = jbase
!     do l = 1, lot {
!         w(j)   = g(i) 
!         w(j+1) = g(i+1) + g(i+33) + g(i+65)
!         w(j+2) = c1 * (g(i)+g(i+32)+g(i+64))
!         w(j+3) = s1 * (g(i+1)+g(i+33)+g(i+65))
!         i = i+96
!         j = j+96
!     }
!     ibase = ibase+2
!     jbase = jbase+6
!     kc = kc+4
! }
ibase = 3
jbase = 7
kc = 5
do kb = 3, 31, 2 {
    c1 = trigs(kb)
    s1 = trigs(kb+1)
    i = ibase
    j = jbase
    do l = 1, lot {
        w(j)   = g(i) 
        w(j+1) = g(i+1) + g(i+33) + g(i+65)
        w(j+2) = c1 * (g(i)+g(i+32)+g(i+64))
        w(j+3) = s1 * (g(i+1)+g(i+33)+g(i+65))
        i = i+96
        j = j+96
    }
    ibase = ibase+2
    jbase = jbase+6
    kc = kc+4
}

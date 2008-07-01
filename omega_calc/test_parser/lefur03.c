
#include <stdio.h>
#include "basic/util.h"

template int max(int, int);
template int min(int, int);

void Exit(int foo) {
  exit(foo);
  }

int main() {

int t1,t2,t3,t4,t5,t6;





for(t1 = 0; t1 <= 3; t1++) {
  for(t2 = max(2*t1-3,0); t2 <= min(t1+1,3); t2++) {
    for(t3 = t1; t3 <= min(-t2+2*t1+1,3*t2+2,3); t3++) {
      for(t4 = max(max(int_div(5*t3+3*t2-6+5,6),int_div(5*t1-t2-4+2,3)),max(t2,0)); t4 <= min(int_div(5*t3+3*t2+7,6),int_div(5*t1-t2+4,3),3); t4++) {
        for(t5 = max(max(max(int_div(1000*t2-2+2,3),250*t4+1),max(int_div(2000*t1-1000*t2-999+2,3),400*t4-200*t2-199)),int_div(1000*t3-1+2,3)); 
		t5 <= min(min(min(400*t4-200*t2+400,500*t1+499),min(int_div(1000*t3+998,3),int_div(1000*t4+1001,3))),1000); t5++) {
          for(t6 = max(max(int_div(t5+1000*t2+1,2),t5),max(1000*t1-t5,-2*t5+1000*t4+2)); t6 <= min(min(int_div(t5+1000*t2+999,2),2*t5+1),min(1000*t1-t5+999,-2*t5+1000*t4+1001)); t6++) {
	    printf("%d,%d,%d,%d,%d,%d\n",
		    t1,t2,t3,t4,t5,t6);
            }
          }
        }
      }
    }
  }
}


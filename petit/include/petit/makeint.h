/* makeint.h,v 1.1.1.2 1992/07/10 02:40:48 davew Exp */
#ifndef Already_Included_MakeInternal
#define Already_Included_MakeInternal

namespace omega {

extern node *Free_Node;
#define Node_List_Size 100
typedef struct Node_List{
    struct Node_List *Next_Node_List;
    unsigned int NNodes;
    node Nodes[Node_List_Size];
    }Node_List;
extern Node_List *Node_List_Head;

}

#endif

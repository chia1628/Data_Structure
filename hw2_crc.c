#include <stdio.h>
#include <stdlib.h>

struct node {
    int poly;
    struct node* next;
};

typedef struct node NODE;

// Insert a new node with poly_index after node P
NODE* INSERT(NODE* P, int poly_index){
    NODE* new_node;
    new_node = (NODE*)malloc(sizeof(NODE));
    printf("In INSERT function\n");
    new_node -> poly = poly_index;
    new_node -> next = P -> next;
    P -> next = new_node;
    return P;
}

// Delete the node P_delete which comes after P_front
NODE* DELETE(NODE* P_front, NODE* P_delete){
    P_front -> next = P_delete -> next;
    free(P_delete);
    return P_front;
}

// Update the polynomial indices in G by adding 'number' to each
NODE* UPDATE_G(NODE* G, int number){
    NODE* tmp;
    tmp = G;
    for(int i=0; i<3; i++){
        tmp -> poly = tmp -> poly + number;
        tmp = tmp -> next;
    }
    return G;
}

// Restore the original polynomial indices in G_ORI
NODE* ORIGINAL_G(NODE* G_ORI){
    NODE* tmp;
    tmp = G_ORI;
    int poly_ori[3] = {4, 1, 0};
    for(int i=0; i<3; i++){
        tmp -> poly = poly_ori[i];
        tmp = tmp -> next;
    }
    return G_ORI;
}

void PRINT(NODE* G){
    NODE* tmp;
    tmp = G;
    for(int i=0; i<3; i++){
        printf("%d ", tmp -> poly);
        tmp = tmp -> next;
    }
    printf("\n");
    printf("PRINT_G done\n");
}

int main(){
    NODE *G, *G_ORI, *D;
    char input_data[3];
    scanf("%s", input_data);
    G = (NODE*)malloc(sizeof(NODE));
    G_ORI = (NODE*)malloc(sizeof(NODE));
    D = (NODE*)malloc(sizeof(NODE));
    G_ORI = ORIGINAL_G(G_ORI);
    PRINT(G_ORI);
    printf("G_ORI initialized\n");
    D = INSERT(D, input_data[0]-'0');
    PRINT(D);
    return 0;
}

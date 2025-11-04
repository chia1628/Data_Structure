#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct node {
    int poly;
    struct node* next;
};

typedef struct node NODE;

// Insert a new node with poly_index after node P
NODE* INSERT_after(NODE* P, int poly_index){
    NODE* new_node;
    new_node = (NODE*)malloc(sizeof(NODE));
    printf("In INSERT_after function\n");
    new_node -> poly = poly_index;
    new_node -> next = P -> next;
    P -> next = new_node;
    return new_node;
}

// Insert a new node with poly_index before node P
NODE* INSERT_before(NODE* P, int poly_index){
    NODE* new_node;
    new_node = (NODE*)malloc(sizeof(NODE));
    printf("In INSERT_before function\n");
    new_node -> poly = poly_index;
    new_node -> next = P;
    return new_node;
}

// Delete the node P_delete which comes after P_front
NODE* DELETE(NODE* P_front, NODE* P_delete){
    if(P_front == NULL){
        return P_delete -> next;
    }
    P_front -> next = P_delete -> next;
    free(P_delete);
    return P_front;
}

// Update the polynomial indices in G by adding 'number' to each
NODE* UPDATE(NODE* LIST, int number){
    NODE* tmp;
    tmp = LIST;
    while(tmp != NULL){
        tmp -> poly = tmp -> poly + number;
        tmp = tmp -> next;
    }
    return LIST;
}

void PRINT(NODE* LIST){
    NODE* tmp;
    tmp = LIST;
    while(tmp != NULL){
        printf("%d ", tmp -> poly);
        tmp = tmp -> next;
    }
}

NODE* COMPARE(NODE* G, NODE* D){
    NODE* pG;
    NODE* pD, *pD_prev = NULL;
    NODE* NEW_HEAD = NULL;
    pG = G;
    pD = D;
    while(pG != NULL){
        if(pD == NULL){
            INSERT_after(pD_prev, pG->poly);
            pG = pG->next;
        }
        else if(pG -> poly == pD -> poly){
            pG = pG -> next;
            printf("Deleting %d\n", pD -> poly);
            pD = DELETE(pD_prev, pD);
            pD_prev = NULL;
        }
        else if(pG -> poly < pD -> poly){
            printf("%d < %d\n", pG -> poly, pD -> poly);
            if(NEW_HEAD==NULL){
                NEW_HEAD = pD;
            }
            pD_prev = pD;
            pD = pD -> next;
        }
        else{ // pG -> poly > pD -> poly
            pD = INSERT_before(pD, pG -> poly);
            if(NEW_HEAD==NULL){
                NEW_HEAD = pD;
                pD_prev  = pD;
            }
            else{
                pD_prev -> next = pD;
            }
            //pD = pD -> next;
            pG = pG -> next;
        }
        printf("Current NEW_HEAD: ");
        PRINT(NEW_HEAD);
        printf("Current D: ");
        PRINT(D);
        printf("Current G: ");
        PRINT(pG);
    }
    printf("\n");
    return NEW_HEAD;
}

void HexToBin(int *ascii, char *binary_string) {
    long i = 0;

    // Iterate through the hex string
    while (ascii[i]) {
        // Convert the character to uppercase for consistent processing
        switch (ascii[i]) {
            case 0: strcat(binary_string, "0000"); break;
            case 1: strcat(binary_string, "0001"); break;
            case 2: strcat(binary_string, "0010"); break;
            case 3: strcat(binary_string, "0011"); break;
            case 4: strcat(binary_string, "0100"); break;
            case 5: strcat(binary_string, "0101"); break;
            case 6: strcat(binary_string, "0110"); break;
            case 7: strcat(binary_string, "0111"); break;
            case 8: strcat(binary_string, "1000"); break;
            case 9: strcat(binary_string, "1001"); break;
            default:
                return; // Stop on invalid input
        }
        i++;
    }
}

int hex_to_byte_value(char* hex, int* ascii) {
    int i = 0;
    int index = 0;
    while (hex[i]) {
        int value = hex[i];;
        printf("%d", value);
        printf("\n");
        ascii[index] = value/16;
        printf("%d", ascii[index]);
        printf("\n");
        index++;
        value = value%16;
        ascii[index] = value;
        printf("%d", ascii[index]);
        printf("\n");
        index++;
        i++;
    }
    return 0; // Invalid character defaults to 0
}

int main(){
    NODE *G_ORI, *P, *D, *G_new;
    char input_data[3];
    int ascii[5];
    char binary_string[16] = {0};
    int poly_ori[3] = {4, 1, 0};
    scanf("%s", input_data);
    printf("Input data: %s\n", input_data);
    D = (NODE*)malloc(sizeof(NODE));
    D -> next = NULL;
    G_ORI = (NODE*)malloc(sizeof(NODE));
    G_ORI -> next = NULL; 
    P = G_ORI;
    for (int i=0; i<3; i++){
        printf("currenct data: %d\n", poly_ori[i]);
        P = INSERT_after(P, poly_ori[i]);
        printf("currenct data: %d\n", P->poly);
    }
    G_ORI = G_ORI -> next; // Move to the first actual node
    PRINT(G_ORI);
    printf("G_ORI initialized\n");
    
    for (int i=0; i<8; i++){
        printf("%c", binary_string[i]);
    }
    hex_to_byte_value(input_data, ascii);
    // for(int i=0; i<4; i++){
    //     printf("%d", ascii[i]);
    // }
    // printf("\n");
    HexToBin(ascii, binary_string);
    for(int i=0; i<16; i++){
        printf("%c", binary_string[i]);
    }
    printf("\n");
    P = D;
    for(int i=0; i<16; i++){
        printf("%c ", binary_string[i]);
        if(binary_string[i] =='1'){
            printf("%d ", 15-i);
            P = INSERT_after(P, 15-i);
        }
    }
    D  = D  -> next; // Move to the first actual node
    printf("D before COMPARE: \n");
    PRINT(D);
    printf("D before UPDATE: \n");
    PRINT(D);
    printf("\n");
    UPDATE(D, 4);
    printf("D AFTER UPDATE: \n");
    PRINT(D);
    printf("\n");
    while(D->poly > 3){
        int shift = D->poly - 4;
        NODE *G_new;
        G_new = (NODE*)malloc(sizeof(NODE));
        G_new -> next = NULL;
        P = G_new;
        for (int i=0; i<3; i++){
            printf("currenct data: %d\n", poly_ori[i]);
            P = INSERT_after(P, poly_ori[i]+shift);
            printf("currenct data: %d\n", P->poly);
        }
        G_new = G_new -> next; // Move to the first actual node
        PRINT(G_new);
        printf("G_ORI initialized\n");
        printf("G_new before COMPARE: \n");
        PRINT(G_new);
        printf("\n");
        printf("D before COMPARE: \n");
        PRINT(D);
        D = COMPARE(G_new, D);
        printf("D AFTER COMPARE: \n");
        PRINT(D);
        printf("\n");
    }
    
    free(G_ORI);
    free(D);
    return 0;
}

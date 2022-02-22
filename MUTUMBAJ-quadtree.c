/***************************************************************************
*   Written by JAMES MUTUMBA                                              *
*                                                                         *
*   This program reads a database containing black pixels from a file,    *
*   builds a 2 dimensional array representing this information, and then
*   builds a Quad tree to represent the nodes at which the black pixels
     exist.
*   ***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define Black 0
#define White 1
#define Mixed 2

/* Structure of node required for the Quad tree containing the 4 children
   NW,NE,SW,SE, the colour, coordinates and size of the node*/
typedef struct node {
        struct node* Child[4];
        int Colour;
        int XPos;
        int YPos;
        int Size;
} Node;

/*Function prototypes*/
bool PowerofTwo(int width);
int **ReadFile(char *file_name, Node* node);
void SetNode(Node* TreeNode, int PosX, int PosY, int Size, int Colour);
int CheckColour (int** Arr, int Top, int Bottom, int Left, int Right);
void BuildQuadTree(Node *node,int **Arr);
int board_bounds(int row, int col,int width);

/*Boolean Function that checks whether the input is a power of 2, returns true(1) or false(0)*/
bool PowerofTwo(int width){

        if( width>=1) {
                return width && (!( width & (width-1) ) );
        }
        else{
                fprintf(stderr, "Incorrect image width specified,must be greater than 1 and a power of 2\n");
                exit(EXIT_FAILURE);
        }
}

/*Function that reads input from a txt file. It takes in input from the
   file specified at command line; It takes in the size, coordinates and
   number of black pixels, error checks them, builds a 2d array and then sets a
   root node and finally returns the static 2d array*/
int **ReadFile(char *file_name, Node* node){

        int input, i, j, width, NumberofBlack,Power,row,col;
        int CountX = 0, CountY = 0;
        int *DataArr = NULL;
        static int **InitArray;
        size_t DataArrSize = 0;
        FILE *fp;

        fp = fopen(file_name, "r");

        if(fp == NULL) {
                fprintf(stderr, "Error reading file, please specify a valid file\n");
                exit(EXIT_FAILURE);
        }

        while (fscanf(fp, "%d", &input) == 1) {
                int *tempArr = realloc(DataArr, (DataArrSize + 1) * sizeof(int));

                if (tempArr != NULL) {

                        DataArr = tempArr;
                        DataArr[DataArrSize++] = input;
                }
        }
        width = DataArr[0];

        Power = PowerofTwo(width);
        if(Power != 1) {
                fprintf(stderr, "Specified width is not a power of 2\n");
                exit(EXIT_FAILURE);
        }

        NumberofBlack = DataArr[1];

        for(i = 2; i < DataArrSize; i++) {
                if (i % 2 == 0) {
                        CountX++;
                }

                else{
                        CountY++;
                }
        }

        if ((CountX != NumberofBlack) || (CountY != NumberofBlack)) {
                fprintf(stderr, "Number of X and Y coordinates do not match the input number of black nodes, try again");
                exit(EXIT_FAILURE);
        }

        if (NumberofBlack>(width*width)) {
                fprintf(stderr, " Number of black pixels bigger than specified width\n");
                exit(EXIT_FAILURE);
        }
        if(NumberofBlack == 0) {
                printf("No black pixels in this input file");
                exit(EXIT_SUCCESS);
        }

        if(NumberofBlack == (width*width)) {
                printf("Black terminal node at position (0,0) with width %i\n", width);
                exit(EXIT_SUCCESS);
        }

        InitArray = (int**) malloc(width*sizeof(int*));
        for(i = 0; i < width; i++)
        {
                InitArray[i] = (int*) malloc(width*sizeof(int));
                for ( j = 0; j < width; j++ )
                {
                        InitArray[i][j] = White;
                }
        }

        for(i = 1; i < DataArrSize/2; i++)
        {
                if (!(board_bounds(DataArr[2 * i],DataArr[2 * i + 1],width))) {
                        fprintf(stderr, "%i pixel off image bounds, please provide correct input\n",i);
                        exit(EXIT_FAILURE);
                }
                InitArray[DataArr[2 * i]][DataArr[2 * i + 1]] = Black;
        }
        free(DataArr);
        SetNode(node,0,0,width,Mixed);
        fclose(fp);

        return (InitArray);
}

/*Function that sets up the parameters of the  root node and child nodes*/
void SetNode(Node* TreeNode, int PosX, int PosY, int Size, int Colour)
{
        int k;

        TreeNode->XPos=PosX;
        TreeNode->YPos=PosY;
        TreeNode->Size= Size;
        TreeNode->Colour= Colour;

        for (k = 0; k < 4; k++)
                TreeNode->Child[k] = (Node*) malloc(sizeof(Node));
}

/*Function that checks the colour on a specific part of the 2d array,
   it takes position arguments and counts the number of blacks and whites in the
   region, it then returns black or white if uniform colour or mixed if both colours*/
int CheckColour (int** Arr, int Top, int Bottom, int Left, int Right)
{
        int i,j,BlackCount=0, WhiteCount=0;

        for ( j = Top; j <= Bottom; j++ )
        {
                for ( i = Left; i <= Right; i++ )
                {
                        if (Arr[i][j]==White) {
                                WhiteCount++;
                        }
                        else if (Arr[i][j]==Black) {
                                BlackCount++;
                        }
                }
        }
        if (WhiteCount==0) {
                return Black;
        }
        if (BlackCount==0) {
                return White;
        }
        return Mixed;
}

/*Function that builds a Quad tree. It takes in a root node, and uses the static InitArray that was
   created from the database. Uses 4 cases to represent the children, checks the colour with CheckColour() in each
   quadrant and recursion to divide the quadrants further until a uniform colour is reached and
   it prints out nodes which are solely black */
void BuildQuadTree(Node *node,int **Arr)
{
        int x, Colour, top, bottom, left, right,size;

        for (x=0; x< 4; x++) {
                switch(x) {
                case 0:
                        top = node->YPos;
                        bottom = node->YPos+(node->Size/2)-1;
                        left = node->XPos;
                        right = node->XPos+(node->Size/2)-1;
                        size = (node->Size)/2;

                        Colour= CheckColour(Arr,top,bottom,left,right);

                        SetNode(node->Child[0],left,top,size,Colour);
                        if (Colour==Black)
                                printf("Black terminal node at Position (%i,%i) with width %i\n", left,top, size);
                        if (Colour==Mixed)
                                BuildQuadTree(node->Child[0],Arr);
                        break;

                case 1:

                        top = node->YPos;
                        bottom = node->YPos+(node->Size/2)-1;
                        left = node->XPos+(node->Size/2);
                        right = node->XPos+node->Size-1;
                        size = (node->Size)/2;

                        Colour= CheckColour(Arr,top,bottom,left,right);

                        SetNode(node->Child[1],left,top,size,Colour);
                        if (Colour==Black)
                                printf("Black terminal node at Position (%i,%i) with width %i\n", left,top, size);
                        if (Colour==Mixed)
                                BuildQuadTree(node->Child[1],Arr);
                        break;
                case 2:
                        top = node->YPos+(node->Size/2);
                        bottom = node->YPos+(node->Size)-1;
                        left = node->XPos;
                        right = node->XPos+(node->Size/2)-1;
                        size = (node->Size)/2;

                        Colour = CheckColour(Arr,top,bottom,left,right);

                        SetNode(node->Child[2],left,top,size,Colour);
                        if (Colour==Black)
                                printf("Black terminal node at Position (%i,%i) with width %i\n", left,top, size);
                        if (Colour==Mixed)
                                BuildQuadTree(node->Child[2],Arr);
                        break;

                case 3:

                        top = node->YPos+(node->Size/2);
                        bottom = node->YPos+(node->Size)-1;
                        left = node->XPos+(node->Size/2);
                        right = node->XPos+node->Size-1;
                        size = (node->Size)/2;

                        Colour = CheckColour(Arr,top,bottom,left,right);

                        SetNode(node->Child[3],left,top,size,Colour);
                        if (Colour==Black)
                                printf("Black terminal node at Position (%i,%i) with width %i\n", left,top, size);
                        if (Colour==Mixed)
                                BuildQuadTree(node->Child[3],Arr);
                        break;

                }
        }
}

/*Function that checks input coordinates are inside the image boundary
   It takes in coordinates and checks them against the created image size boundary*/
int board_bounds(int row, int col,int width){
        if ((row < 0 || col < 0 || row >= width || col >= width))
                return 0;
        else
                return 1;
}

/*Driver function which calls all the relevant functions*/
int main(int argc, char *argv[]){
        int **Arr;
        Node Root;
        if (argc != 2) {
                fprintf(stderr, "Usage ./quadtree input.txt\n");
                exit(EXIT_FAILURE);
        }
        Arr = ReadFile(argv[1],&Root);
        BuildQuadTree(&Root,Arr);
}

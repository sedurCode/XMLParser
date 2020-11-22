#include "lxml.h"
int main()
{
    XMLDocument doc;
    if (XMLDocument_load(&doc, "./test.xml"))
    {   
        printf("Num Children of Root: %d \n", doc.root->children.size);
        XMLNode* main_node = XMLNode_child(doc.root, 0);
        printf("Main node: %s => %s \n", main_node->tag, main_node->inner_text);

        XMLNode* another_node = XMLNode_child(doc.root, 1);
        printf("Another node: %s => %s \n", another_node->tag, another_node->inner_text);
        int c = 0;
        while( c++ < 1000000 ); // you can use sleep but for this you dont need #import
        {//Wait for some time to print before we free
        } 
        XMLDocument_free(&doc);
    }
    return 0;
}
#include "lxml.h"
int main()
{
    XMLDocument doc;
    if (XMLDocument_load(&doc, "./test.xml"))
    {   
        printf("XML version: %s \n", doc.version);
        printf("XML encoding: %s \n", doc.encoding);
        printf("Num Children of Root: %d \n", doc.root->children.size);
        XMLNode* main_node = XMLNode_child(doc.root, 0);
        //printf("Main node: %s => %s \n", main_node->tag, main_node->inner_text);

        XMLNodeList* fields = XMLNode_children(main_node, "field");


        for (int i = 0; i < fields->size; i++)
        {
            XMLNode* field = XMLNodeList_at(fields, i);
            XMLAttribute* attribute = XMLNode_attribute(field, "type");
            attribute->value = "";
            printf("   %s => %s \n", XMLNode_attribute_value(field, "name"), XMLNode_attribute_value(field, "type"));
        }

        XMLDocument_write(&doc, "./out.xml", 4);

        int c = 0;
        while( c++ < 1000000 ) // you can use sleep but for this you dont need #import
        {//Wait for some time to print before we free
        } 
        XMLDocument_free(&doc);
    }
    return 0;
}
#include "lxml.h"

int main()
{
    XMLDocument doc;
    if (XMLDocument_load(&doc, "./test.xml"))
    {
        XMLNode node = *doc.root;
        printf("Attributes:\n");
        for(int i = 0; i < node.attributes.size; i++)
        {
            XMLAttribute attribute = node.attributes.data[i];
            printf("   %s => \"%s\" \n", attribute.key, attribute.value);
        }

        printf("%s: %s \n", node.tag, node.inner_text);

        XMLDocument_free(&doc);
    }
    return 0;
}
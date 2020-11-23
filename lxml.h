#ifndef LITTLE_XML_H
#define LITTLE_XML_H

// Includes 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Deffinitions
// #define DEBUG
#ifdef DEBUG
#ifndef DEBUG_PRINT
#define DEBUG_PRINT printf
#endif
#else
#ifndef DEBUG_PRINT
#define DEBUG_PRINT 
#endif
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

int ends_with(const char* haystack, const char* needle)
{
    int h_len = strlen(haystack);
    int n_len = strlen(needle);
    if (h_len < n_len)
    {
        return FALSE;
    }

    for (int i = 0; i < n_len; i++)
    {
        if (haystack[h_len - n_len + i] != needle[i])
        {
            return FALSE;
        }
    }
    return TRUE;
}

struct _XMLNodeList
{
    int heap_size;
    int size;
    struct _XMLNode** data;
};
typedef struct _XMLNodeList XMLNodeList;

struct _XMLAttribute
{
    char* key;
    char* value;
};
typedef struct _XMLAttribute XMLAttribute;

struct _XMLAttributeList
{
    int heap_size;
    int size;
    XMLAttribute* data;
};
typedef struct _XMLAttributeList XMLAttributeList;

struct _XMLNode
{
    char* tag;
    char* inner_text;
    struct _XMLNode* parent;
    XMLAttributeList attributes;
    XMLNodeList children;
};
typedef struct _XMLNode XMLNode;

struct _XMLDocument
{
    char* version;
    char* encoding;
    XMLNode* root;
};
typedef struct _XMLDocument XMLDocument;

// Forward declaration

int XMLDocument_load(XMLDocument* doc, const char* path);
int XMLDocument_write(XMLDocument* doc, const char* path, int indent);
void XMLDocument_free(XMLDocument* doc);
XMLNode* XMLNode_new(XMLNode* parent);
void XMLNode_free(XMLNode* node);
// XMLAttribute* XMLAttribute_new(XMLNode* parent);
void XMLAttribute_free(XMLAttribute* attribute);
void XMLAttributeList_init(XMLAttributeList* list);
void XMLAttributeList_add(XMLAttributeList* list, XMLAttribute* attribute);
void XMLNodeList_init(XMLNodeList* list);
void XMLNodeList_free(XMLNodeList* list);
void XMLNodeList_add(XMLNodeList* list, XMLNode* node);
XMLNode* XMLNode_child(XMLNode* parent, int index);
char* XMLNode_attribute_value(XMLNode* node, char* key);
XMLNode* XMLNodeList_at(XMLNodeList* list, int index);
XMLNodeList* XMLNode_children(XMLNode* parent, const char* tag);
XMLAttribute* XMLNode_attribute(XMLNode* node, char* key);


// Implementations

enum _TagType
{
    TAG_START,
    TAG_INLINE
};
typedef enum _TagType TagType;

/** static void parse_attributes(char* buffer, int* i, char* lex, int* lexi, XMLNode* current_node)
 * 
 */
static TagType parse_attributes(char* buffer, int* i, char* lex, int* lexi, XMLNode* current_node)
{
    XMLAttribute currentAttribute = {0, 0};
    // Read the beginning of the tag of the node into the buffer
    while (buffer[(*i)] != '>')
    {
        lex[(*lexi)++] = buffer[(*i)++];
        // If we have it a patch of whitespace and we have not written a tag yet, lex buffer now has the tag
        if(buffer[(*i)] == ' ' && !current_node->tag)
        {
            lex[(*lexi)]= '\0';
            // Create a new string with the same content as what we just read and assign to the tag of the node
            current_node->tag = strdup(lex);
            DEBUG_PRINT("Tag of new node is %s \n", current_node->tag);
            // Reset index to lex buffer
            (*lexi) = 0;
            (*i)++; 
            continue;
        }
        // unusally ignore spaces
        if(lex[(*lexi)-1] == ' ')
        {
            (*lexi)--;
        }
        // If we hit an equals, we have the attribute key in the buffer
        if (buffer[(*i)] == '=')
        {
            lex[(*lexi)]= '\0';
            currentAttribute.key = strdup(lex);
            (*lexi) = 0;
            continue;
        }
        // attribute value
        if (buffer[(*i)] == '"')
        {
            if (!currentAttribute.key)
            {
                fprintf(stderr, "Value  %s has no key at node %s \n", lex, current_node->tag);
                return TAG_START;// FALSE;
            }
            (*lexi) = 0;
            (*i)++;
            while(buffer[(*i)] != '"')
            {
                lex[(*lexi)++] = buffer[(*i)++];
            }
            lex[(*lexi)]= '\0';
            currentAttribute.value = strdup(lex);
            XMLAttributeList_add(&current_node->attributes, &currentAttribute);
            // Reset current attribute placeholder to empty
            currentAttribute.key = NULL;
            currentAttribute.value = NULL;
            (*lexi) = 0;
            (*i)++;
            continue;
        }
        // Inline node handling
        if (buffer[(*i)-1] == '/' && buffer[(*i)] == '>')
        {
            lex[(*lexi)]= '\0';
            if(!current_node->tag)
            {
                current_node->tag = strdup(lex);
            }
            (*i)++;
            return TAG_INLINE;
        }
    }
    return TAG_START;
}

/** bool XMLDocument_load(XMLDocument* doc, const char* path)
 * 
 */
int XMLDocument_load(XMLDocument* doc, const char* path)
{
    DEBUG_PRINT("opening file %s \n", path);
    FILE* file = fopen(path, "r");
    if (!file)
    {
        fprintf(stderr, "Failed to open file '%s' \n", path);
        return FALSE;
    }

    // Find size of file
    fseek(file, 0, SEEK_END);
    int size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    // Create a buffer to fill with the data of the document
    char* buffer = (char*) malloc(sizeof(char) * size + 1);
    // Read the document into the buffer
    fread(buffer, 1, size, file);
    fclose(file);
    buffer[size] = '\0';

    doc->root = XMLNode_new(NULL);

    // Lexical Analysis
    char lex[256];
    int lexi = 0;
    int i = 0;

    XMLNode* current_node = doc->root;

    // While loop that parses the document into new nodes
    while(buffer[i] != '\0')
    {
        if (buffer[i] == '<')
        {
            DEBUG_PRINT("Enterring new tag region\n");
            // null terminate anything in the lex buffer
            lex[lexi] = '\0'; 
            // If content has been written into lex buffer
            if (lexi > 0)
            {
                DEBUG_PRINT("Lex is not empty\n");
                if (!current_node)
                {
                    fprintf(stderr, "text outside of document\n");
                    return FALSE;
                }
                // Allocate a copy of the lex buffer contents to the inner text of the current node
                current_node->inner_text = strdup(lex);
                DEBUG_PRINT("Contents of lex: %s \n", lex);
                DEBUG_PRINT("Contents of lext coppied to inner text\n");
                lexi = 0;
            } else {
                DEBUG_PRINT("Lex is empty \n");
            }
            // End of node
            if(buffer[i+1] == '/')
            {
                DEBUG_PRINT("Entering end node region\n");
                i += 2; // move on to the text of the tag
                while (buffer[i] != '>')
                {
                    lex[lexi++] = buffer[i++];
                }
                lex[lexi]= '\0';
                lexi = 0;
                if (!current_node)
                {
                    fprintf(stderr, "Invalid XML document: end tag at root\n");
                    return FALSE;
                }
                // If these buffers are not the same then we have a problem
                if (strcmp(current_node->tag, lex))
                {
                    fprintf(stderr, "Mismatched tags (%s != %s) \n", current_node->tag, lex);
                    return FALSE;   
                }
                // If we hit the end tag, return to parent and continue reading
                current_node = current_node->parent;
                i++;
                continue;
            } else {DEBUG_PRINT("Not end node\n");}

            // handle comments
            if (buffer[i + 1] == '!')
            {
                while (buffer[i] != ' ' && buffer[i] != '>')
                {
                    lex[lexi++] = buffer[i++];
                }
                lex[lexi] = '\0';
                if (!strcmp(lex, "<!--"))
                {
                    lex[lexi] = '\0';
                    while(!ends_with(lex, "-->"))
                    {
                        lex[lexi++] = buffer[i++];
                        lex[lexi] = '\0';
                    }
                    continue;
                }
            }

            // handle declaration tags
            if (buffer[i + 1] == '?')
            {
                while (buffer[i] != ' ' && buffer[i] != '>')
                {
                    lex[lexi++] = buffer[i++];
                }
                lex[lexi] = '\0';
                //Handle xml version spec declaration
                if (!strcmp(lex, "<?xml"))
                {
                    lexi = 0;
                    XMLNode* desc = XMLNode_new(NULL);
                    parse_attributes(buffer, &i, lex, &lexi, desc);
                    doc->version = XMLNode_attribute_value(desc, "version");
                    doc->encoding = XMLNode_attribute_value(desc, "encoding");
                    continue;
                }
            }

            // We are at a new node, so prepare current_node for the new node
            DEBUG_PRINT("Parent node of new node is %s \n", current_node->tag);
            // Parent is the last node
            current_node = XMLNode_new(current_node);
            // Progress document pointer
            i++;
            // Parse attributes
            if(parse_attributes(buffer, &i, lex, &lexi, current_node) == TAG_INLINE)
            {
                current_node = current_node->parent;
                i++;
                continue;
            }
            // Reset index to lex buffer
            lex[lexi]= '\0';
            if (!current_node->tag)
            {
                // Create a new string with the same content as what we just read and assign to the tag of the node
                current_node->tag = strdup(lex);
                DEBUG_PRINT("Tag of new node is %s \n", current_node->tag);
            }
            lexi = 0; 
            i++; // Move on to the body
            continue;
        } else {
            // If we arent in a tag field, fill lex buffer with inner text content
            lex[lexi++] = buffer[i++];
        }
    }

    // Free the initial buffer
    free(buffer);
    // if we succeeded, return true
    return TRUE;
} 

static void node_out(FILE* file, XMLNode* node, int indent, int times)
{
    // For all child nodes
    for (int i = 0; i < node->children.size; i++)
    {
        // Get the node
        XMLNode* child = XMLNode_child(node, i);
        //Apply spacing
        if (times>0)
        {
            fprintf(file, "%*s", indent * times, " ");
        }
        // Write out tag
        fprintf(file, "<%s", child->tag);
        // Write out non-null attributes of node
        for (int j = 0; j < child->attributes.size; j++)
        {
            XMLAttribute attribute = child->attributes.data[j];
            if((!attribute.value) || (!strcmp(attribute.value, "")))
            {
                continue;
            }
            fprintf(file, " %s=\"%s\"", attribute.key, attribute.value);
        }
        // If simple, make a one-line tag
        if ((child->children.size == 0)&&(!child->inner_text))
        {
            fprintf(file, " />\n");
        } else {
            // If node has children, start writing those on a new line.
            // If node has no children but does that innertext, write the innertext on this line.
            if(child->children.size == 0)
            {
                fprintf(file, ">");
            } else {
                fprintf(file, ">\n");
            }
            // If node has no children but does have innertext
            // Write the innertext and end the node on this line
            if((child->children.size == 0)&&(child->inner_text))
            {
                fprintf(file, "%s</%s>\n", child->inner_text, child->tag);
            } else {
                // If there are children, enter the children and begin recursive writeout call
                if(child->children.size > 0)
                {
                    node_out(file, child, indent, times + 1);
                }
                // If we have innertext
                if(child->inner_text)
                {
                    // Apply indent
                    if (times > 0)
                    {
                        fprintf(file, "%*s", indent * times, " ");
                    }
                    // Writeout innertext and newline the closing tag
                    fprintf(file, "%s\n", child->inner_text);
                }
                // Writeout indent
                if (times > 0)
                {
                    fprintf(file, "%*s", indent * times, " ");
                }
                // Closing tag after some combination of children and potentially inner text
                fprintf(file, "</%s>\n", child->tag);
            }
        }
    }
}

/** int XMLDocument_write(XMLDocument* doc, const char* path, int indent)
 * 
 * 
 */
int XMLDocument_write(XMLDocument* doc, const char* path, int indent)
{
    FILE* file = fopen(path, "w");
    if (!file)
    {
        fprintf(stderr, "Failed to open file '%s' \n", path);
        return FALSE;
    }
    // Write out XML header
    fprintf(file, "<?xml version=\"%s\" encoding=\"%s\" ?>\n", 
    (doc->version) ? doc->version : "1.0", 
    (doc->encoding) ? doc->encoding : "UTF-8"
    );

    node_out(file, doc->root, indent, 0);

    // close file
    fclose(file);
    return TRUE;
}

void XMLDocument_free(XMLDocument* doc)
{
    XMLNode_free(doc->root);
}

/** XMLNode* XMLNode_new(XMLNode* parent)
 * Allocates a new node with a pointer to the partent node and null contents
 * Args: Parent: Pointer to parent node
 */ 
XMLNode* XMLNode_new(XMLNode* parent)
{
    XMLNode* node = (XMLNode*) malloc(sizeof(XMLNode));
    node->parent = parent;
    node->tag = NULL;
    node->inner_text = NULL;
    XMLAttributeList_init(&node->attributes);
    XMLNodeList_init(&node->children);
    if(parent)
    {
        XMLNodeList_add(&parent->children, node);
    }
    return node;
}

/** void XMLNode_free(XMLNode* node)
 * Checks and frees the contnets of the tag and inner text of a node,
 * before freeing the node itself. 
 */ 
void XMLNode_free(XMLNode* node)
{
    DEBUG_PRINT("Entered free of node %s \n", node->tag);
    DEBUG_PRINT("Freeing children of node %s \n", node->tag);
    XMLNodeList_free(&node->children);
    if(node->tag)
    {
        free(node->tag);
    }
    if(node->inner_text)
    {
        free(node->inner_text);
    }
    free(node);
}

/** void XMLAttributeList_init(XMLAttributeList* list)
 */ 
void XMLAttributeList_init(XMLAttributeList* list)
{
    list->heap_size = 1;
    list->size = 0;
    list->data = (XMLAttribute*) malloc(sizeof(XMLAttribute) * list->heap_size);
}

/** void XMLAttributeList_add(XMLAttributeList* list, XMLAttribute* attribute)
 */ 
void XMLAttributeList_add(XMLAttributeList* list, XMLAttribute* attribute)
{
    // ensure that our list size does not go beyond the heap have made available
    while(list->size >= list->heap_size)
    {
        list->heap_size *= 2;
        list->data = (XMLAttribute*) realloc(list->data, sizeof(XMLAttribute) * list->heap_size);
    }
    list->data[list->size++] = *attribute;
}

/** void XMLAttributeList_free(XMLAttributeList* list)
 */ 
void XMLAttributeList_free(XMLAttributeList* list)
{

}

/** void XMLNodeList_init(XMLNodeList* list)
 */ 
void XMLNodeList_init(XMLNodeList* list)
{
    list->heap_size = 1;
    list->size = 0;
    list->data = (XMLNode**) malloc(sizeof(XMLNode*) * list->heap_size);
}

/** void XMLNodeList_free(XMLNodeList* list);
 * 
 * 
 */
void XMLNodeList_free(XMLNodeList* list)
{
    if (list->data)
    {
        for (int index = 0; index < list->size; index++)
        {
            XMLNode_free(list->data[index]);
        }
    }
    list->size = 0;
    list->heap_size = 0;
}

/** void XMLNodeList_add(XMLNodeList* list, XMLNode* node); 
 * 
 * 
 */
void XMLNodeList_add(XMLNodeList* list, XMLNode* node)
{
    // ensure that our list size does not go beyond the heap have made available
    while(list->size >= list->heap_size)
    {
        list->heap_size *= 2;
        list->data = (XMLNode**) realloc(list->data, sizeof(XMLNode*) * list->heap_size);
    }
    list->data[list->size++] = node;
}

/** XMLNode* XMLNode_child(XMLNode* parent, int index)
 * 
 * 
 */
XMLNode* XMLNode_child(XMLNode* parent, int index)
{
   return parent->children.data[index]; 
}

/** char* XMLNode_attribute_value(XMLNode* node, char* key)
 * 
 */ 
char* XMLNode_attribute_value(XMLNode* node, char* key)
{
    for (int i = 0; i < node->attributes.size; i++)
    {
        XMLAttribute tAttrib = node->attributes.data[i];
        if(!strcmp(tAttrib.key, key))
        {
            return tAttrib.value;
        }
    }
    return NULL;
}

/** XMLNode* XMLNodeList_at(XMLNodeList* list, int index);
 * Get a node at a point in the node list
 */
XMLNode* XMLNodeList_at(XMLNodeList* list, int index)
{
    return list->data[index];
}

/** XMLNodeList* XMLNode_children(XMLNode* node)
 * returns the nodelist of child nodes from an XMLNode
 * Should be 'child by name'
 */
XMLNodeList* XMLNode_children(XMLNode* parent, const char* tag)
{
    XMLNodeList* list = (XMLNodeList*) malloc(sizeof(XMLNodeList));
    XMLNodeList_init(list);
    for (int i = 0; i < parent->children.size; i++)
    {
        XMLNode* child = XMLNode_child(parent, i);
        if (!strcmp(child->tag, tag))
        {
            XMLNodeList_add(list, child);
        }
    }
    return list;
}

/** XMLAttribute* XMLNode_attribute(XMLNode* node, char* key)
 * 
 * 
 */ 
XMLAttribute* XMLNode_attribute(XMLNode* node, char* key)
{
    for (int i = 0; i < node->attributes.size; i++)
    {
        XMLAttribute* tAttrib = &node->attributes.data[i];
        if(!strcmp(tAttrib->key, key))
        {
            return tAttrib;
        }
    }
    return NULL;
}

#endif // LITTLE_XML_H
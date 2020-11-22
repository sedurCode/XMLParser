#ifndef LITTLE_XML_H
#define LITTLE_XML_H

// Includes 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Deffinitions
#define DEBUG
#ifdef DEBUG
#ifndef DEBUG_PRINT
#define DEBUG_PRINT printf
#endif
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

struct _XMLNode
{
    char* tag;
    char* inner_text;
    struct _XMLNode* parent;
};
typedef struct _XMLNode XMLNode;

struct _XMLDocument
{
    XMLNode* root;
};
typedef struct _XMLDocument XMLDocument;

// Forward declaration

int XMLDocument_load(XMLDocument* doc, const char* path);
void XMLDocument_free(XMLDocument* doc);
XMLNode* XMLNode_new(XMLNode* parent);
void XMLNode_free(XMLNode* node);

// Implementations

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

    XMLNode* current_node = NULL;

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

            // We are at a new node, so prepare current_node for the new node
            if (!current_node)
            {
                DEBUG_PRINT("Current node is root\n");
                // If at the bottom, parent is root
                current_node = doc->root;
            } else {
                DEBUG_PRINT("Parent node of new node is %s \n", current_node->tag);
                // Parent is the last node
                current_node = XMLNode_new(current_node);
            }
            // Progress document pointer
            i++;
            // Read the tag of the node into the buffer
            while (buffer[i] != '>')
            {
                lex[lexi++] = buffer[i++];
            }
            lex[lexi]= '\0';
            // Create a new string with the same content as what we just read and assign to the tag of the node
            current_node->tag = strdup(lex);
            DEBUG_PRINT("Tag of new node is %s \n", current_node->tag);
            // Reset index to lex buffer
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
    return node;
}

/** void XMLNode_free(XMLNode* node)
 * Checks and frees the contnets of the tag and inner text of a node,
 * before freeing the node itself. 
 */ 
void XMLNode_free(XMLNode* node)
{
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

#endif // LITTLE_XML_H
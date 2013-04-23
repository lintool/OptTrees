#ifndef STRUCT_H_GUARD
#define STRUCT_H_GUARD

#include<stdlib.h>

typedef struct Struct Struct;

/**
 * Similar to the Object implementation, but in C
 * and using structs.
 */
struct Struct {
  Struct* right; // Right node
  Struct* left; // Left node
  int fid; // Feature id
  unsigned long id; // Node id
  float threshold; // Threshold/Regression value
};

/**
 * Creates a new node
 *
 * @param id Node id
 * @param fid Feature id that the node conditions on
 * @param threshold Threshold/Regression value
 */
Struct* createNode(unsigned long id, int fid, float threshold) {
  Struct* node = (Struct*) malloc(sizeof(Struct));
  node->id = id;
  node->fid = fid;
  node->threshold = threshold;
  node->left = 0;
  node->right = 0;

  return node;
}

// Destructor
void destroyTree(Struct* node) {
  if(node->right != 0) {
    destroyTree(node->right);
    free(node->right);
    node->right = 0;
  }
  if(node->left != 0) {
    destroyTree(node->left);
    free(node->left);
    node->left = 0;
  }
}

/**
 * Inserts a new node to the left or right of "node"
 *
 * @param node Parent node
 * @param id Node id
 * @param leftChild Whether to insert the current node to the left of parent node or not
 * @param featureId Feature id
 * @param threshold Threshold/Regression value
 * @return A new node
 */
Struct* addNode(Struct* node, unsigned long id, int leftChild, int featureId, float threshold) {
  if(leftChild) {
    node->left = createNode(id, featureId, threshold);
    return node->left;
  } else {
    node->right = createNode(id, featureId, threshold);
    return node->right;
  }
}

/**
 * Traverses the tree from the specified node and returns a pointer to the
 * terminal node for the given feature vector.
 *
 * @param pThis Pointer to the current node
 * @param featureVector Feature vector
 * @return Pointer to a terminal node.
 */
Struct* getLeaf(Struct* pThis, float* featureVector) {
  if(!pThis->left && !pThis->right) {
    return pThis;
  }
  if(featureVector[pThis->fid] <= pThis->threshold) {
    return getLeaf(pThis->left, featureVector);
  } else {
    return getLeaf(pThis->right, featureVector);
  }
}

#endif

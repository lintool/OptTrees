#ifndef STRUCT_PLUS_H_GUARD
#define STRUCT_PLUS_H_GUARD

#include<stdlib.h>

typedef struct StructPlus StructPlus;

/**
 * Similar to Struct, however the memory layout is done
 * manually.
 */
struct StructPlus {
  StructPlus* right;
  StructPlus* left;
  unsigned long id;
  int fid;
  float threshold;
};

/**
 * Creates an array of nodes.
 *
 * @param size Initial number of nodes in the tree
 * @return Pointer to array of nodes
 */
StructPlus* createNodes(long size) {
  StructPlus* tree = (StructPlus*) calloc(size, sizeof(StructPlus));
  return tree;
}

/**
 * Removes empty nodes from the array, and re-organizes the array
 *
 * @param array New array
 * @param old Old array
 * @param index Next available node in the array
 * @return Updated index
 */
long compressNodes(StructPlus* array, StructPlus* old, long index) {
  // Copy information from the old node to the new node
  array[index].id = old->id;
  array[index].fid = old->fid;
  array[index].threshold = old->threshold;

  // If the current node has left and right subtrees, repeat this process
  if(old->right || old->left) {
    long pindex = index;
    index = compressNodes(array, old->left, index + 1);
    array[pindex].left = &array[pindex + 1];

    array[pindex].right = &array[index + 1];
    index = compressNodes(array, old->right, index + 1);
  } else {
    array[index].right = 0;
    array[index].left = 0;
  }

  return index;
}

/**
 * Counts the number of nodes in the tree
 *
 * @param root Root of the tree
 * @return Number of nodes in the tree
 */
long countNodes(StructPlus* root) {
  long count = 1;
  if(!root->left && !root->right) {
    return count;
  }
  count += countNodes(root->left);
  count += countNodes(root->right);
  return count;
}

/**
 * Re-organizes a tree by removing empty nodes from the memory layout
 *
 * @param root Root of the tree
 * @return New tree structure that is more compact
 */
StructPlus* compress(StructPlus* root) {
  long validNodes = countNodes(root);
  StructPlus* tree = (StructPlus*) calloc(validNodes, sizeof(StructPlus));
  long index = compressNodes(tree, root, 0);
  return tree;
}

void destroyTree(StructPlus* tree) {
  free(tree);
  tree = 0;
}

/**
 * Sets the root of the tree
 */
void setRoot(StructPlus* tree, unsigned long id, int featureId, float threshold) {
  tree[0].id = id;
  tree[0].fid = featureId;
  tree[0].threshold = threshold;
  tree[0].left = 0;
  tree[0].right = 0;
}

/**
 * Inserts a new node to the left or right of the parent node
 *
 * @param tree Tree structure
 * @param pindex Index of the parent node
 * @param index Index of the child node
 * @param id Node id
 * @param leftChild Left or right subtree
 * @param featureId Feature id
 * @param threshold Threshold
 */
void addNode(StructPlus* tree, unsigned long pindex,
             unsigned long index, unsigned long id,
            int leftChild, int featureId, float threshold) {
  tree[index].id = id;
  tree[index].fid = featureId;
  tree[index].threshold = threshold;
  tree[index].left = 0;
  tree[index].right = 0;
  if(leftChild) {
    tree[pindex].left = &tree[index];
  } else {
    tree[pindex].right = &tree[index];
  }
}

/**
 * Traverses a tree and finds a terminal node using the
 * input feature vector.
 *
 * @param pThis Tree structure
 * @param featureVector Test instance
 * @param Pointer to a terminal node
 */
StructPlus* getLeaf(StructPlus* pThis, float* featureVector) {
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

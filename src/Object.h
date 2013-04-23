#ifndef Object_H_GUARD
#define Object_H_GUARD

#include <math.h>
#include <stdlib.h>

/*
 * An instance of the Object class represents a node in the
 * regression tree, which comprises left and right pointers,
 * feature id of the feature that this node conditions on,
 * and a threshold.
 *
 * For terminal nodes, "threshold" is used to hold the regression
 * value associated with the node.
 */
class Object {
 public:
  Object* right;
  Object* left;
  int fid;
  float threshold;

 public:
  unsigned long id; // Node id
  ~Object();
  Object(unsigned long id, int fid, float threshold); // Constructor

  /*
   * Inserts a new node to the left or right of a node
   *
   * @param id Id of the new node
   * @param left Inserts the node to the left of this node if set to 1, right otherwise
   * @param featureId Fid of the new node
   * @param threshold Threshold/Regression value of the new node
   * @return A new node
   */
  Object* addNode(unsigned long id, bool left, int featureId, float threshold);

  /*
   * Traverses the tree starting from this node using the input feature vector
   *
   * @param featureVector Input feature vector
   * @param featureVectorSize Length of the feature vector
   * @return Regression value of the terminal node
   */
  float getLeaf(float* featureVector, int featureVectorSize);
};

Object::~Object() {
  if(right != 0) {
    right->~Object();
    delete right;
  }
  if(left != 0) {
    left->~Object();
    delete left;
  }
}

Object::Object(unsigned long id, int fid, float threshold) {
  this-> id = id;
  this-> fid = fid;
  this->threshold = threshold;
  this->left = 0;
  this->right = 0;
}

Object* Object::addNode(unsigned long id, bool left, int featureId, float threshold) {
  if(left) {
    this->left = new Object(id, featureId, threshold);
    return this->left;
  } else {
    this->right = new Object(id, featureId, threshold);
    return this->right;
  }
}

float Object::getLeaf(float* featureVector, int featureVectorSize) {
  if(!left && !right) {
    return this->threshold;
  }
  if(featureVector[abs(fid)] <= threshold) {
    return left->getLeaf(featureVector, featureVectorSize);
  } else {
    return right->getLeaf(featureVector, featureVectorSize);
  }
}

#endif

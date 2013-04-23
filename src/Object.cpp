#include <iostream>
#include <fstream>
#include <string>
#include <sys/time.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include "Object.h"
#include "ParseCommandLine.h"

using namespace std;

/**
 * Driver that evaluates test instances using the Object
 * implementation. Use the following command to run this
 * driver:
 *
 * ./Object -ensemble <ensemble-path> -instances <test-instances-path> \
 *          -maxLeaves <max-number-of-leaves> [-print]
 *
 */

int main(int argc, char** args) {
  if(!isPresentCL(argc, args, (char*) "-ensemble") ||
     !isPresentCL(argc, args, (char*) "-instances") ||
     !isPresentCL(argc, args, (char*) "-maxLeaves")) {
    return -1;
  }

  char* configFile = getValueCL(argc, args, (char*) "-ensemble");
  char* featureFile = getValueCL(argc, args, (char*) "-instances");
  int maxNumberOfLeaves = atoi(getValueCL(argc, args, (char*) "-maxLeaves"));
  int printScores = isPresentCL(argc, args, (char*) "-print");

  // Open ensemble file
  ifstream file (configFile);

  // Read ensemble
  int numberOfTrees;
  file >> numberOfTrees;

  // Array of pointers to tree roots, one per tree in the ensemble
  Object** root = new Object*[numberOfTrees];

  // Number of nodes in a tree does not exceed (maxLeaves * 2)
  int treeSize = 2 * maxNumberOfLeaves;

  for(int t = 0; t < numberOfTrees; t++) {
    long size;
    file >> size;

    Object** pointers = new Object*[treeSize];
    for(long i = 0 ; i < treeSize; i++) {
      pointers[i] = 0;
    }

    // There are three types of nodes in the ensemble file:
    //   root
    //   node (intermediate node)
    //   leaf (terminal node)
    // "end" indicates the end of a tree
    string type;
    file >> type;
    int curIndex = 0;

    while(type != "end") {
      //read node id
      long id;
      file >> id;

      // A "root" node contains a feature id and a threshold
      if(type == "root") {
        int fid;
        float threshold;
        file >> fid;
        file >> threshold;
        root[t] = new Object(id, fid, threshold);
        // Set the root pointer
        pointers[curIndex++] = root[t];
      } else if(type == "node") {
        int fid;
        long pid;
        float threshold;
        bool left = false;
        string token;
        file >> pid; // Id of the parent node
        file >> fid; // Feature id
        file >> token; // Whether it's a left or right child
        if(token == "1") {
          left = true;
        }
        file >> threshold; // Threshold/Regression value

        // Find the parent node, based in parent id
        int parentIndex = 0;
        for(parentIndex = 0; parentIndex < treeSize; parentIndex++) {
          if(pointers[parentIndex]->id == pid) {
            break;
          }
        }
        // Add the new node
        if(pointers[pid]->fid >= 0) {
          pointers[curIndex++] = pointers[parentIndex]->addNode(id, left, fid, threshold);
        }
      } else if(type == "leaf") {
        long pid;
        bool left = false;
        string token;
        float value;
        file >> pid;
        file >> token;
        if(token == "1") {
          left = true;
        }
        file >> value;

        int parentIndex = 0;
        for(parentIndex = 0; parentIndex < treeSize; parentIndex++) {
          if(pointers[parentIndex]->id == pid) {
            break;
          }
        }
        if(pointers[pid]->fid >= 0) {
          pointers[curIndex++] = pointers[parentIndex]->addNode(id, left, 0, value);
        }
      }
      file >> type;
    }
    delete(pointers);
  }
  file.close();

  // Read instances (SVM Light format)
  int numberOfInstances, numberOfFeatures;
  ifstream ffile (featureFile);
  ffile >> numberOfInstances;
  ffile >> numberOfFeatures;
  float** features = new float*[numberOfInstances];
  int id = 0;
  for(int i = 0; i < numberOfInstances; i++) {
    features[i] = new float[numberOfFeatures];
    string token;
    char text[20];
    ffile >> token;
    ffile >> token;

    int index = 0;
    while(index < numberOfFeatures) {
      ffile >> token;
      sscanf(token.c_str(), "%[^:]:%f", text, &features[i][index++]);
    }
  }
  ffile.close();

  // Compute scores for instances using the ensemble and
  // measure elapsed time
  float score = 0;
  float sum = 0; // Dummy value just so gcc wouldn't optimize the loop out
  struct timeval start, end;

  gettimeofday(&start, NULL);
  for(int i = 0; i < numberOfInstances; i++) {
    score = 0;
    for(int t = 0; t < numberOfTrees; t++) {
      score += root[t]->getLeaf(features[i], numberOfFeatures);
    }
    if(printScores) {
      cout << score << endl;
    }
    sum += score;
  }
  gettimeofday(&end, NULL);

  cout << "Time per instance (ns): " <<
    (((end.tv_sec * 1000000 + end.tv_usec) -
      (start.tv_sec * 1000000 + start.tv_usec))*1000/((float) numberOfInstances)) << endl;
  cout << "Ignore this number: " << sum << endl;

  delete(root);
  for(int i = 0; i < numberOfInstances; i++) {
    delete(features[i]);
  }
  delete(features);
  return 0;
}

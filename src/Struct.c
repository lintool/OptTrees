#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>
#include "Struct.h"
#include "ParseCommandLine.h"

/**
 * Driver that evaluates test instances using the Struct
 * implementation. Use the following command to run this driver:
 *
 * ./Struct -ensemble <ensemble-path> -instances <test-instances-path> \
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

  // Read ensemble
  FILE *fp = fopen(configFile, "r");
  int nbTrees;
  fscanf(fp, "%d", &nbTrees);

  // Array of pointers to tree roots, one per tree in the ensemble
  Struct** trees = (Struct**) malloc(nbTrees * sizeof(Struct*));
  int tindex = 0;

  // Number of nodes in a tree does not exceed (maxLeaves * 2)
  int maxTreeSize = 2 * maxNumberOfLeaves;

  for(tindex = 0; tindex < nbTrees; tindex++) {
    long treeSize;
    fscanf(fp, "%ld", &treeSize);

    Struct** pointers = (Struct**) malloc(maxTreeSize * sizeof(Struct*));
    char text[20];
    long line = 0;
    for(line = 0; line < maxTreeSize; line++) pointers[line] = 0;

    // There are three types of nodes in the ensemble file:
    //   root
    //   node (intermediate node)
    //   leaf (terminal node)
    // "end" indicates the end of a tree
    int curIndex = 0;
    fscanf(fp, "%s", text);
    while(strcmp(text, "end") != 0) {
      long id;
      fscanf(fp, "%ld", &id);

      // A "root" node contains a feature id and a threshold
      if(strcmp(text, "root") == 0) {
        int fid;
        float threshold;
        fscanf(fp, "%d %f", &fid, &threshold);
        trees[tindex] = createNode(id, fid, threshold);
        // Set the root pointer
        pointers[curIndex++] = trees[tindex];
      } else if(strcmp(text, "node") == 0) {
        int fid;
        long pid;
        float threshold;
        int leftChild = 0;
        // Read Id of the parent node, feature id, subtree (left or right),
        // and threshold
        fscanf(fp, "%ld %d %d %f", &pid, &fid, &leftChild, &threshold);

        // Find the parent node, based in parent id
        int parentIndex = 0;
        for(parentIndex = 0; parentIndex < maxTreeSize; parentIndex++) {
          if(pointers[parentIndex]->id == pid) {
            break;
          }
        }
        // Add the new node
        if(pointers[pid]->fid >= 0) {
          pointers[curIndex++] = addNode(pointers[parentIndex], id, leftChild, fid, threshold);
        }
      } else if(strcmp(text, "leaf") == 0) {
        long pid;
        int leftChild = 0;
        float value;
        fscanf(fp, "%ld %d %f", &pid, &leftChild, &value);

        int parentIndex = 0;
        for(parentIndex = 0; parentIndex < maxTreeSize; parentIndex++) {
          if(pointers[parentIndex]->id == pid) {
            break;
          }
        }
        if(pointers[pid]->fid >= 0) {
          pointers[curIndex++] = addNode(pointers[parentIndex], id, leftChild, 0, value);
        }
      }
      fscanf(fp, "%s", text);
    }
    free(pointers);
  }
  fclose(fp);

  // Read instances (SVM Light format)
  int numberOfFeatures = 0;
  int numberOfInstances = 0;

  fp = fopen(featureFile, "r");
  fscanf(fp, "%d %d", &numberOfInstances, &numberOfFeatures);
  float** features = (float**) malloc(numberOfInstances * sizeof(float*));
  int i = 0;
  for(i = 0; i < numberOfInstances; i++) {
    features[i] = (float*) malloc(numberOfFeatures * sizeof(float));
  }

  float fvalue;
  int fIndex = 0, iIndex = 0;
  int ignore;
  char text[20];
  for(iIndex = 0; iIndex < numberOfInstances; iIndex++) {
    fscanf(fp, "%d %[^:]:%d", &ignore, text, &ignore);
    for(fIndex = 0; fIndex < numberOfFeatures; fIndex++) {
      fscanf(fp, "%[^:]:%f", text, &fvalue);
      features[iIndex][fIndex] = fvalue;
    }
  }

  // Compute scores for instances using the ensemble and
  // measure elapsed time
  int sum = 0; // Dummy value just so gcc wouldn't optimize the loop out
  float score;
  struct timeval start, end;

  gettimeofday(&start, NULL);
  for(iIndex = 0; iIndex < numberOfInstances; iIndex++) {
    score = 0;
    for(tindex = 0; tindex < nbTrees; tindex++) {
      score += getLeaf(trees[tindex], features[iIndex])->threshold;
    }
    if(printScores) {
      printf("%f\n", score);
    }
    sum += score;
  }
  gettimeofday(&end, NULL);

  printf("Time per instance (ns): %5.2f\n",
         (((end.tv_sec * 1000000 + end.tv_usec) -
           (start.tv_sec * 1000000 + start.tv_usec))*1000/((float) numberOfInstances)));
  printf("Ignore this number: %d\n", sum);

  // Free used memory
  for(tindex = 0; tindex < nbTrees; tindex++) {
    destroyTree(trees[tindex]);
  }
  free(trees);
  for(i = 0; i < numberOfInstances; i++) {
    free(features[i]);
  }
  free(features);
  fclose(fp);
  return 0;
}

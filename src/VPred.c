#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>
#include "Struct.h"
#include "VPred.h"
#include "ParseCommandLine.h"

/**
 * Driver that evaluates test instances using the VPred
 * implementation. Use the following command to run this driver:
 *
 * ./VPred -ensemble <ensemble-path> -instances <test-instances-path> \
 *         -maxLeaves <max-number-of-leaves> [-print]
 *
 */

// Function pointers
void (*findLeaf[151])(int* leaves, float* features,
                      int numberOfFeatures, Node* nodes) =
{0, &findLeafDepth1, &findLeafDepth2, &findLeafDepth3, &findLeafDepth4,
 &findLeafDepth5, &findLeafDepth6, &findLeafDepth7, &findLeafDepth8,
 &findLeafDepth9, &findLeafDepth10, &findLeafDepth11, &findLeafDepth12,
 &findLeafDepth13, &findLeafDepth14, &findLeafDepth15, &findLeafDepth16,
 &findLeafDepth17, &findLeafDepth18, &findLeafDepth19, &findLeafDepth20,
 &findLeafDepth21, &findLeafDepth22, &findLeafDepth23, &findLeafDepth24,
 &findLeafDepth25, &findLeafDepth26, &findLeafDepth27, &findLeafDepth28,
 &findLeafDepth29, &findLeafDepth30, &findLeafDepth31, &findLeafDepth32,
 &findLeafDepth33, &findLeafDepth34, &findLeafDepth35, &findLeafDepth36,
 &findLeafDepth37, &findLeafDepth38, &findLeafDepth39, &findLeafDepth40,
 &findLeafDepth41, &findLeafDepth42, &findLeafDepth43, &findLeafDepth44,
 &findLeafDepth45, &findLeafDepth46, &findLeafDepth47, &findLeafDepth48,
 &findLeafDepth49, &findLeafDepth50, &findLeafDepth51, &findLeafDepth52,
 &findLeafDepth53, &findLeafDepth54, &findLeafDepth55, &findLeafDepth56,
 &findLeafDepth57, &findLeafDepth58, &findLeafDepth59, &findLeafDepth60,
 &findLeafDepth61, &findLeafDepth62, &findLeafDepth63, &findLeafDepth64,
 &findLeafDepth65, &findLeafDepth66, &findLeafDepth67, &findLeafDepth68,
 &findLeafDepth69, &findLeafDepth70, &findLeafDepth71, &findLeafDepth72,
 &findLeafDepth73, &findLeafDepth74, &findLeafDepth75, &findLeafDepth76,
 &findLeafDepth77, &findLeafDepth78, &findLeafDepth79, &findLeafDepth80,
 &findLeafDepth81, &findLeafDepth82, &findLeafDepth83, &findLeafDepth84,
 &findLeafDepth85, &findLeafDepth86, &findLeafDepth87, &findLeafDepth88,
 &findLeafDepth89, &findLeafDepth90, &findLeafDepth91, &findLeafDepth92,
 &findLeafDepth93, &findLeafDepth94, &findLeafDepth95, &findLeafDepth96,
 &findLeafDepth97, &findLeafDepth98, &findLeafDepth99, &findLeafDepth100,
 &findLeafDepth101, &findLeafDepth102, &findLeafDepth103, &findLeafDepth104,
 &findLeafDepth105, &findLeafDepth106, &findLeafDepth107, &findLeafDepth108,
 &findLeafDepth109, &findLeafDepth110, &findLeafDepth111, &findLeafDepth112,
 &findLeafDepth113, &findLeafDepth114, &findLeafDepth115, &findLeafDepth116,
 &findLeafDepth117, &findLeafDepth118, &findLeafDepth119, &findLeafDepth120,
 &findLeafDepth121, &findLeafDepth122, &findLeafDepth123, &findLeafDepth124,
 &findLeafDepth125, &findLeafDepth126, &findLeafDepth127, &findLeafDepth128,
 &findLeafDepth129, &findLeafDepth130, &findLeafDepth131, &findLeafDepth132,
 &findLeafDepth133, &findLeafDepth134, &findLeafDepth135, &findLeafDepth136,
 &findLeafDepth137, &findLeafDepth138, &findLeafDepth139, &findLeafDepth140,
 &findLeafDepth141, &findLeafDepth142, &findLeafDepth143, &findLeafDepth144,
 &findLeafDepth145, &findLeafDepth146, &findLeafDepth147, &findLeafDepth148,
 &findLeafDepth149, &findLeafDepth150};

/**
 * Organizes a tree with a breadth-first traversal.
 *
 * @param root Root of the tree
 * @param i Index of the next available node in the new array
 * @param nodes New node array
 * @return Updated index
 */
int createNodes(Struct* root, long i, Node* nodes) {
  // Copy information from old node to current node at index i
  nodes[i].fid = root->fid;
  nodes[i].theta = root->threshold;

  // If left and right subtrees do not exist, create a self-loop
  // and point the node to itself.
  if(!root->left && !root->right) {
    nodes[i].children[0] = i;
    nodes[i].children[1] = i;
  } else {
    //otherwise, first pack the left subtree, then the right subtree
    nodes[i].children[0] = i + 1;
    int last = createNodes(root->left, i + 1, nodes);
    nodes[i].children[1] = last + 1;
    i = createNodes(root->right, last + 1, nodes);
  }
  return i;
}

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

  // Total number of nodes in the ensemble
  long totalNodes = 0;
  // Size of trees in ensemble
  long* nodeSizes = (long*) malloc(nbTrees * sizeof(long));
  // Depth of trees in ensemble
  long* treeDepths = (long*) malloc(nbTrees * sizeof(long));
  // Trees
  Node** nodes = (Node**) malloc(nbTrees * sizeof(Node*));

  int tindex = 0;
  // Number of nodes in a tree does not exceed (maxLeaves * 2)
  int maxTreeSize = 2 * maxNumberOfLeaves;

  for(tindex = 0; tindex < nbTrees; tindex++) {
    fscanf(fp, "%ld", &treeDepths[tindex]);

    // Temporary tree structure
    Struct** pointers = (Struct**) malloc(maxTreeSize * sizeof(Struct*));
    Struct* root;
    char text[20];
    long line = 0;

    fscanf(fp, "%s", text);
    while(strcmp(text, "end") != 0) {
      long id;
      fscanf(fp, "%ld", &id);

      if(strcmp(text, "root") == 0) {
        int fid;
        float threshold;
        fscanf(fp, "%d %f", &fid, &threshold);
        root = createNode(id, fid, threshold);
        pointers[line] = root;
      } else if(strcmp(text, "node") == 0) {
        int fid;
        long pid;
        float threshold;
        int leftChild = 0;
        fscanf(fp, "%ld %d %d %f", &pid, &fid, &leftChild, &threshold);

        // Find the parent node, based in parent id
        int parentIndex = 0;
        for(parentIndex = 0; parentIndex < maxTreeSize; parentIndex++) {
          if(pointers[parentIndex]->id == pid) {
            break;
          }
        }
        pointers[line] = addNode(pointers[parentIndex], id, leftChild, fid, threshold);
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
        addNode(pointers[parentIndex], id, leftChild, 0, value);
      } else if(strcmp(text, "feature") == 0) {
        continue;
      }
      fscanf(fp, "%s", text);
      line++;
    }
    totalNodes += line;
    nodeSizes[tindex] = line;

    // Remove empty nodes and create final tree structure
    nodes[tindex] = (Node*) malloc(line * sizeof(Node));
    createNodes(root, 0, nodes[tindex]);
    // Free temporary structure
    destroyTree(root);
    free(root);
    root = 0;
    free(pointers);
  }
  fclose(fp);

  // Pack all trees into a single array, thereby avoiding two-D arrays.
  Node* all_nodes = (Node*) malloc(totalNodes * sizeof(Node));
  int newIndex = 0;
  for(tindex = 0; tindex < nbTrees; tindex++) {
    int nsize = nodeSizes[tindex];
    nodeSizes[tindex] = newIndex;
    int telement;
    for(telement = 0; telement < nsize; telement++) {
      all_nodes[newIndex].fid = abs(nodes[tindex][telement].fid);
      all_nodes[newIndex].theta = nodes[tindex][telement].theta;
      all_nodes[newIndex].children[0] = nodes[tindex][telement].children[0];
      all_nodes[newIndex].children[1] = nodes[tindex][telement].children[1];
      newIndex++;
    }
  }

  for(tindex = 0; tindex < nbTrees; tindex++) {
    free(nodes[tindex]);
  }
  free(nodes);
  nodes = 0;

  // Read features into a flat array
  fp = fopen(featureFile, "r");
  int numberOfInstances = 0;
  int numberOfFeatures = 0;
  fscanf(fp, "%d %d", &numberOfInstances, &numberOfFeatures);
  int divisibleNumberOfInstances = numberOfInstances;
  while(divisibleNumberOfInstances % V != 0) {
    divisibleNumberOfInstances++;
  }
  float* features = (float*) malloc(divisibleNumberOfInstances * numberOfFeatures * sizeof(float));
  float fvalue;
  int fIndex = 0, iIndex = 0;
  char text[20];
  int ignore;
  for(iIndex = 0; iIndex < numberOfInstances; iIndex++) {
    fscanf(fp, "%d %[^:]:%d", &ignore, text, &ignore);
    for(fIndex = 0; fIndex < numberOfFeatures; fIndex++) {
      fscanf(fp, "%[^:]:%f", text, &fvalue);
      features[iIndex*numberOfFeatures+fIndex] = fvalue;
    }
  }

  // Compute scores for V instances at a time and measure elapsed time
  int leaf[V];
  float scores[V] = {0};
  int sum = 0; // Dummy value just so gcc wouldn't optimize the loop out
  int j = 0;
  struct timeval start, end;

  gettimeofday(&start, NULL);
  for(iIndex = 0; iIndex < numberOfInstances; iIndex+=V) {
    for(tindex = 0; tindex < nbTrees; tindex++) {
      findLeaf[treeDepths[tindex]](leaf, &features[iIndex * numberOfFeatures],
                                   numberOfFeatures, &all_nodes[nodeSizes[tindex]]);
      for(j = 0; j < V; j++) {
        scores[j]+=all_nodes[nodeSizes[tindex]+leaf[j]].theta;
      }
    }
    for(j = 0; j < V; j++) {
      if(printScores) {
        printf("%f\n", scores[j]);
      }
      sum+=scores[j];
      scores[j] = 0;
    }
  }
  gettimeofday(&end, NULL);

  printf("Time per instance (ns): %5.2f\n",
         (((end.tv_sec * 1000000 + end.tv_usec) -
           (start.tv_sec * 1000000 + start.tv_usec)) * 1000/((float) numberOfInstances)));
  printf("Ignore this number: %d\n", sum);

  // Free used memory
  fclose(fp);
  free(features);
  free(all_nodes);
  free(treeDepths);
  free(nodeSizes);
  return 0;
}

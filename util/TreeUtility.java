import java.io.InputStream;
import java.io.IOException;
import java.io.File;
import java.io.FileInputStream;
import java.util.List;
import java.util.Map;

import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

import com.google.common.base.Preconditions;
import com.google.common.collect.Lists;
import com.google.common.io.InputSupplier;
import com.google.common.io.Files;

import org.xml.sax.SAXException;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

/**
 * Converts jforest trees into a format compatible with OptTrees
 *
 * @author Nima Asadi
 */
public class TreeUtility {
  private static enum Mode {
    TREE, CODEGEN
  }

  public static void convertTree(String treePath, Mode mode) throws Exception {
    Preconditions.checkNotNull(treePath);
    TreeUtility.convertTree(Files.newInputStreamSupplier(new File(treePath)), mode);
  }

  public static void convertTree(InputSupplier<? extends InputStream> treeInputSupplier, Mode mode)
    throws ParserConfigurationException, SAXException, IOException {
    Preconditions.checkNotNull(treeInputSupplier);

    Document dom = DocumentBuilderFactory.newInstance().
      newDocumentBuilder().parse(treeInputSupplier.getInput());
    NodeList nodeList = dom.getDocumentElement().getElementsByTagName("Tree");

    if(nodeList == null) {
      return;
    }

    int nbTrees = nodeList.getLength();

    if(mode == Mode.TREE) {
      System.out.println(nbTrees);
    }
    for(int i = 0, funcId = 0; i < nodeList.getLength(); i++) {
      Element element = (Element) nodeList.item(i);
      String[] splitFeaturesText = element.getElementsByTagName("SplitFeatures").
        item(0).getFirstChild().getNodeValue().split("\\s+");
      String[] leafOutputs = element.getElementsByTagName("LeafOutputs").
        item(0).getFirstChild().getNodeValue().split("\\s+");
      String[] thresholdsText = element.getElementsByTagName("OriginalThresholds").
        item(0).getFirstChild().getNodeValue().split("\\s+");
      String[] leftChildrenText = element.getElementsByTagName("LeftChildren").
        item(0).getFirstChild().getNodeValue().split("\\s+");
      String[] rightChildrenText = element.getElementsByTagName("RightChildren").
        item(0).getFirstChild().getNodeValue().split("\\s+");

      int[] leftChildren = new int[leftChildrenText.length];
      for(int j = 0; j < leftChildren.length; j++) {
        leftChildren[j] = Integer.parseInt(leftChildrenText[j]);
      }

      int[] rightChildren = new int[leftChildrenText.length];
      for(int j = 0; j < rightChildren.length; j++) {
        rightChildren[j] = Integer.parseInt(rightChildrenText[j]);
      }

      int depth = findDepth(leftChildren, rightChildren, 0);
      if(mode == Mode.TREE) {
        printTree(depth, leftChildren, rightChildren, splitFeaturesText,
                  thresholdsText, leafOutputs);
      } else if(mode == Mode.CODEGEN) {
        String funcName = "findLeaf" + funcId++;
        String parameter = "features";
        System.out.println("float " + funcName + "(float* " + parameter + ") {");
        printHardCodedTree(0, leftChildren, rightChildren, splitFeaturesText,
                           thresholdsText, leafOutputs, parameter, depth);
        System.out.println("}");
      }
    }
    if(mode == Mode.CODEGEN) {
      printHardCodedMain("findLeaf", nbTrees);
    }
  }

  private static int findDepth(int[] leftChildren, int[] rightChildren, int node) {
    int ld = 1;
    if(leftChildren[node] >= 0) {
      ld += findDepth(leftChildren, rightChildren, leftChildren[node]);
    }
    int rd = 1;
    if(rightChildren[node] >= 0) {
      rd += findDepth(leftChildren, rightChildren, rightChildren[node]);
    }
    return Math.max(ld, rd);
  }

  private static class TreeNode {
    int id, pid, left;
    String feature, theta, leaf;

    TreeNode(int id, int pid, int left,
             String feature, String theta, String leaf) {
      this.id = id;
      this.pid = pid;
      this.left = left;
      this.feature = feature;
      this.theta = theta;
      this.leaf = leaf;
    }
  }

  private static void printTree(int depth, int[] leftChildren, int[] rightChildren,
                                String[] splitFeatures, String[] thresholds,
                                String[] leafOutputs) {
    int treeSize = (int) Math.pow(2, depth) - 1;
    List<TreeNode> nodes = Lists.newArrayList();
    int id = 0;

    // Print depth and root information
    System.out.println(depth);
    System.out.println("root 0 " + splitFeatures[0] + " " + thresholds[0]);

    if(leftChildren.length > 0) {
      if(leftChildren[0] >= 0) {
        nodes.add(new TreeNode(leftChildren[0], id, 1,
                               splitFeatures[leftChildren[0]],
                               thresholds[leftChildren[0]], "0"));
      } else {
        nodes.add(new TreeNode(leftChildren[0], id, 1,
                               splitFeatures[0],
                               thresholds[0], leafOutputs[~leftChildren[0]]));
      }
    }

    if(rightChildren.length > 0) {
      if(rightChildren[0] >= 0) {
        nodes.add(new TreeNode(rightChildren[0], id, 0,
                               splitFeatures[rightChildren[0]],
                               thresholds[rightChildren[0]], "0"));
      } else {
        nodes.add(new TreeNode(rightChildren[0], id, 0,
                               splitFeatures[0],
                               thresholds[0], leafOutputs[~rightChildren[0]]));
      }
    }

    id++;
    while(!nodes.isEmpty()) {
      TreeNode node = nodes.remove(0);
      if(node.id < 0) {
        if(id >= treeSize) {
          System.out.println("leaf " + id + " " + node.pid +
                             " " + node.left + " " + node.leaf);
        } else {
          System.out.println("node " + id + " " + node.pid + " " +
                             node.feature + " " + node.left + " " + node.leaf);
        }
      } else {
        System.out.println("node " + id + " " + node.pid + " " +
                           node.feature + " " + node.left + " " + node.theta);
        if(leftChildren[node.id] >= 0) {
          nodes.add(new TreeNode(leftChildren[node.id], id, 1,
                                 splitFeatures[leftChildren[node.id]],
                                 thresholds[leftChildren[node.id]], node.leaf));
        } else {
          nodes.add(new TreeNode(leftChildren[node.id], id, 1, node.feature,
                                 node.theta, leafOutputs[~leftChildren[node.id]]));
        }

        if(rightChildren[node.id] >= 0) {
          nodes.add(new TreeNode(rightChildren[node.id], id, 0,
                                 splitFeatures[rightChildren[node.id]],
                                 thresholds[rightChildren[node.id]], node.leaf));
        } else {
          nodes.add(new TreeNode(rightChildren[node.id], id, 0, node.feature,
                                 node.theta, leafOutputs[~rightChildren[node.id]]));
        }
      }
      id++;
    }
    System.out.println("end");
  }

  private static void printHardCodedTree(int node, int[] leftChildren, int[] rightChildren,
                                         String[] splitFeatures, String[] thresholds,
                                         String[] leafOutputs, String parameter, int depth) {
    System.out.println("  if(" + parameter + "[" + splitFeatures[node] +
                       "] <= " + thresholds[node] + ") {" );
    if(leftChildren[node] >= 0) {
      printHardCodedTree(leftChildren[node], leftChildren, rightChildren,
                         splitFeatures, thresholds, leafOutputs, parameter, depth - 1);
    } else {
      System.out.println("  return " + leafOutputs[~leftChildren[node]] + ";");
    }

    System.out.println("  } else {");
    if(rightChildren[node] >= 0) {
      printHardCodedTree(rightChildren[node], leftChildren, rightChildren,
                         splitFeatures, thresholds, leafOutputs, parameter, depth - 1);
    } else {
      System.out.println("  return " + leafOutputs[~rightChildren[node]] + ";");
    }
    System.out.println("  }");
  }

  private static void printHardCodedMain(String funcName, int nbTrees) {
    System.out.print("#include <stdio.h>\n");
    System.out.print("#include <stdlib.h>\n");
    System.out.print("#include <sys/time.h>\n");
    System.out.print("#include <time.h>\n");

    System.out.print("int main(int argc, char** args) {\n");
    System.out.print("  if(argc < 2) {\n");
    System.out.print("    return -1;\n");
    System.out.print("  }\n");
    System.out.print("  char* featureFile = args[1];\n");
    System.out.print("  FILE *fp = fopen(featureFile, \"r\");\n");
    System.out.print("  int numberOfInstances;\n");
    System.out.print("  int numberOfFeatures;\n");
    System.out.print("  fscanf(fp, \"%d %d\", &numberOfInstances, &numberOfFeatures);");
    System.out.print("  float** features = (float**) malloc(numberOfInstances * sizeof(float*));\n");
    System.out.print("  int i = 0;");
    System.out.print("  for(i = 0; i < numberOfInstances; i++) { features[i] =" +
                     "(float*) malloc(numberOfFeatures * sizeof(float)); }\n");
    System.out.print("  float fvalue;\n");
    System.out.print("  int fIndex = 0, iIndex = 0;\n");
    System.out.print("  char text[20];\n");
    System.out.print("  int ignore;\n");
    System.out.print("  for(iIndex = 0; iIndex < numberOfInstances; iIndex++) {\n");
    System.out.print("    fscanf(fp, \"%d %[^:]:%d\", &ignore, text, &ignore);\n");
    System.out.print("    for(fIndex = 0; fIndex < numberOfFeatures; fIndex++) {\n");
    System.out.print("      fscanf(fp, \"%[^:]:%f\", text, &fvalue);\n");
    System.out.print("      features[iIndex][fIndex] = fvalue;\n");
    System.out.print("    }\n");
    System.out.print("  }\n");
    System.out.print("  int sum = 0;\n");
    System.out.print("  float score = 0;\n");
    System.out.print("  struct timeval start, end;\n");
    System.out.print("  gettimeofday(&start, NULL);\n");
    System.out.print("  for(iIndex = 0; iIndex < numberOfInstances; iIndex++) {\n");
    System.out.print("    score = 0;\n");
    for(int i = 0; i < nbTrees; i++) {
      System.out.print("    score += " + funcName + i + "(features[iIndex]);\n");
    }
    System.out.print("    //printf(\"%f\\n\", score);\n");
    System.out.print("    sum += score;\n");
    System.out.print("  }\n");
    System.out.print("  gettimeofday(&end, NULL);\n");
    System.out.print("  printf(\"Time per instance (ns): %5.2f\\n\", (((end.tv_sec * 1000000 + end.tv_usec)" +
                     " - (start.tv_sec * 1000000 + start.tv_usec)) * 1000/((float) numberOfInstances)));\n");
    System.out.print("  printf(\"Ignore this number: %d\\n\", sum);\n");
    System.out.print("  fclose(fp);\n");
    System.out.print("  for(i = 0; i < numberOfInstances; i++) { free(features[i]); }\n");
    System.out.print("  free(features);");
    System.out.print("  return 0;\n");
    System.out.print("}\n");
  }

  public static String argValue(String[] args, String option)
    throws IllegalArgumentException {
    for(int i = 0; i < args.length - 1; i++) {
      if(args[i].equals(option)) {
        return args[i + 1];
      }
    }
    throw new IllegalArgumentException("Option not found: " + option);
  }

  public static void main(String[] args) throws Exception {
    if(args.length < 4) {
      System.out.println("usage:\n -input jforest-ensemble-xml -mode [tree|codegen]");
      return;
    }

    String input = argValue(args, "-input");
    String modeText = argValue(args, "-mode");

    Mode mode = Mode.TREE;
    if(modeText.equals("tree")) {
      mode = Mode.TREE;
    } else if(modeText.equals("codegen")) {
      mode = Mode.CODEGEN;
    }

    TreeUtility.convertTree(input, mode);
  }
}

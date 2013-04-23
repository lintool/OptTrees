Instance File
------------

Every line in the feature file contains a feature vector of the following form:

	<first_line> .=. <Number of instances:integer> <Number of features:integer>
	<line> .=. <relevance: integer> qid:<query id: integer> 1:<value for feature 1: float> 2:<value for feature 2: float> ...

Tree Ensemble File
--------------

To convert a tree ensemble created by jforests into a tree compatible with the OptTrees framework, please use the following java driver under `util/`:

	java TreeUtility -input jforests-ensemble-output-xml -mode [tree|codegen]

By setting `-mode` to `codegen`, the driver will output a hard-coded ensemble with if-else blocks (i.e., the CodeGen implementation). Otherwise, the jforest ensemble will be formatted such that it can be read by OptTrees. Note that, this driver prints the output to stdout.

Evaluating Test Instances
--------------

Given a tree ensemble file and a test instances file, you can use any of the drivers provided in `src/` to compute scores. By default, these drivers only measure the elapsed time to evaluate a single instance and report it in nanoseconds.

	make
	out/<implementation> -ensemble <tree-ensemble-file> -instances <test-instances-file> \
	                     -maxLeaves <max-number-of-leaves-from-jforests> [-print]

Using `-print`, you can print the computed scores.


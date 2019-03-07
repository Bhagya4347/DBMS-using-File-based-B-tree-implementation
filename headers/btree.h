#include<stdio.h>
#include"node.h"	



#ifndef NODE_H_
#define NODE_H_
#endif

#define file_name "stu.bin"
#define init_file "btree.bin"

struct bTree {
	
	int min_degree;
	int root;
	int node_count;
	int next_pos;	
};


 struct bTree * intialize_tree(int t, int mode);
 struct node * create_node(int leaf, struct bTree *bt);

#include<stdio.h>
#include"student.h"
#define T 24

#ifndef BTREE_H_
#define BTREE_H_
#endif

#ifndef NODE_H_
#define NODE_H_
#endif


struct node {
	int key_count;
	int is_leaf;
	int location_in_disk;
	struct student students[2 * T - 1];
	int children[2 * T];
};

//struct node * create_node(int leaf, struct bTree *bt);


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include"/media/sumanth/Secondary/Toy DB using B-Tree/headers/btree.h"
#include "../headers/btree.h"
#include "../headers/color.h"

#define ALLOC(x) (x *)malloc(sizeof(x));

struct node * create_node(int leaf, struct bTree *bt)
{
	struct node *newnode = NULL;
	newnode = ALLOC(struct node);
	newnode->key_count = 0;
	newnode->is_leaf = leaf;
	newnode->location_in_disk = bt->next_pos;
	bt->next_pos++;

	int t;
	t = bt->min_degree;
	// newnode->students = (struct student *)malloc(((2 * t)-1) * sizeof(struct student));
	// newnode->children = (int *)malloc((2 * t) * sizeof(int));
	int i;
	for (i = 0; i < 2 * t; ++i) {
		newnode->children[i] = -1;
	}

	return newnode;
}

struct bTree * intialize_tree(int t, int mode) {
	struct bTree *bt = NULL;
	bt = ALLOC(struct bTree);

	if (mode == 0) {
		bt->next_pos = 0;
		bt->min_degree = t;
		bt->root = 0;
		bt->node_count = 0;

		return bt;
	}	
	FILE *fp = fopen(init_file,"r");
	/*code to read btree head node from meta.dat file */
	fseek(fp, 0, 0);
	fread(bt, sizeof(struct bTree), 1, fp);
	fclose(fp);

	return bt;
} 

void exit_db(struct bTree *bt)
{
	FILE *fp = fopen(init_file, "w");
	fseek(fp, 0, 0);
	fwrite(bt, sizeof(struct bTree), 1, fp);
	fclose(fp);
}

void read_file(struct bTree *bt, struct node *p, int pos) 
{
	FILE *fp = NULL;
	fp = fopen(file_name, "r+");

	if (fp == NULL) {
		printf("failed to open file\n");

		return;
	}

	fseek(fp, pos * sizeof(struct node), 0);
	fread(p, sizeof(struct node), 1, fp);

	// printf("content is successfully read\n");
	// printf("name %s\n", p->students[0].name);

	fclose(fp);
}

void write_file(struct bTree *ptr_tree, struct node *p, int pos, int mod)
{
	// pos = -1; use next_pos {
	if (pos == -1) {
		pos = ptr_tree->next_pos++;
	}

	FILE *fp = NULL;

	fp = (mod == 0) ? fopen(file_name, "w") : fopen(file_name, "r+");

	if (fp == NULL) {
		printf("failed to open file\n");

		return;
	}
	// printf("node is being stored at %ld\n", pos * sizeof(struct node));

	fseek(fp, pos *sizeof(struct node), 0);
	fwrite(p, sizeof(struct node), 1, fp); 
	fclose(fp);
}

void split_child(struct bTree *tree, struct node *x, int i, struct node *y)
{
	struct node *z = create_node(y->is_leaf, tree);
	tree->node_count++;
	int t;
	t = tree->min_degree ;
	z->key_count = t - 1;

	int j;
	for (j = 0; j < t - 1; j++) {
		z->students[j] = y->students[j + t];
	}
	if (!y->is_leaf) {
		for (j = 0; j < t; j++) {
			z->children[j] = y->children[j + t];
		}
	}

	y->key_count = t - 1;
	for (j = x->key_count; j >= i + 1; j--) {
		x->children[j + 1] = x->children[j];
	}
	x->children[i + 1] = z->location_in_disk;

	for (j = x->key_count - 1; j >= i; j--) {
		x->students[j + 1] = x->students[j];
	}

	x->students[i] = y->students[t - 1];

	x->key_count++;

	write_file(tree, x, x->location_in_disk, 1);
	write_file(tree, y, y->location_in_disk, 1);
	write_file(tree, z, z->location_in_disk, 1);
	free(z);

}

void insert_non_full(struct bTree *tree, struct node *node, struct student *record)
{	
	int i;
	i = (node->key_count) - 1; 	
	if (node->is_leaf) {
		while (i >= 0 && (strcmp(node->students[i].enrollment_no, record->enrollment_no) > 0)) {
			node->students[i + 1] = node->students[i];
			i--;
		}

		node->students[i + 1] = *record;
		node->key_count++;
		write_file(tree, node, node->location_in_disk, 1);  
	} else {
		while (i >= 0 && (strcmp(node->students[i].enrollment_no, record->enrollment_no) > 0)) {
			i--;
		}
		struct node *c_i = ALLOC(struct node);

		read_file(tree, c_i, node->children[i + 1]);
		if (c_i->key_count == (2 * tree->min_degree - 1)) {
			split_child(tree, node, i + 1, c_i);
			if ((strcmp(node->students[i + 1].enrollment_no, record->enrollment_no) < 0)) {
				i++;
			}
		}
		read_file(tree, c_i, node->children[i + 1]);
		insert_non_full(tree, c_i, record);
		free(c_i);
	}
}

void insert(struct bTree *tree, struct student *record)
{
	if ((tree->next_pos) == 0) {
		tree->root = tree->next_pos;
		struct node *newnode = create_node(1, tree);
		tree->node_count++;
		newnode->students[0] = *record;
		newnode->key_count++;
		tree->root = newnode->location_in_disk;
		write_file(tree, newnode, newnode->location_in_disk, 0);
		free(newnode);

		return;
	} else {
		struct node *root = ALLOC(struct node);
		read_file(tree, root, tree->root);
		if (root->key_count == (2 * tree->min_degree - 1)) {
			struct node *newroot = create_node(0, tree);

			tree->node_count++;
			newroot->children[0] = tree->root;
			split_child(tree, newroot, 0, root);

			int i = 0;
			if (strcmp(newroot->students[0].enrollment_no, record->enrollment_no) < 0) {
				i++;
			}

			struct node *c_i = ALLOC(struct node);
			read_file(tree, c_i, newroot->children[i]);
			insert_non_full(tree, c_i, record);

			tree->root = newroot->location_in_disk;

			write_file(tree, root, root->location_in_disk, 1);            
			free(newroot);
		} else {
			insert_non_full(tree, root, record);
		}
		free(root);
	}
}

struct student * search_recursive(struct bTree *tree, char *ar, struct node *root)
{
	int i = 0;
	while (i < root->key_count && (strcmp(ar, root->students[i].enrollment_no) > 0)) {
		i++;
	}

	if (i < root->key_count && (strcmp(ar, root->students[i].enrollment_no) == 0)) {
		return &root->students[i];
	} else if (root->is_leaf) {
		return NULL;
	} else {
		struct node *c_i = ALLOC(struct node);
		read_file(tree, c_i, root->children[i]);

		return search_recursive(tree, ar, c_i);
	}
}

struct student * search(struct bTree *tree, char *key)
{
	struct node *root = ALLOC(struct node);
	read_file(tree, root, tree->root);

	return search_recursive(tree, key, root);
}

void update_helper_pk(struct bTree *tree, char *ar, struct node *root, char update_column[], char update_key[])
{
	int i = 0;
	while (i < root->key_count && (strcmp(ar, root->students[i].enrollment_no) > 0)) {
		i++;
	}

	if (i < root->key_count && (strcmp(ar, root->students[i].enrollment_no) == 0)) {
		strcmp(update_column, "name") == 0 ? strcpy(root->students[i].name, update_key) :
			strcmp(update_column, "email") == 0 ? strcpy(root->students[i].email, update_key) :
			strcmp(update_column, "course_code") == 0 ? strcpy(root->students[i].course_code, update_key) : strcpy(root->students[i].gender, update_key);
		// printf("updated\n");
		write_file(tree, root, root->location_in_disk, 1);

		return;
	} else if (root->is_leaf) {
		return;
	} else {
		struct node *c_i = ALLOC(struct node);
		read_file(tree, c_i, root->children[i]);
		update_helper_pk(tree, ar, c_i, update_column, update_key);
	}
}

void update_pk(struct bTree *tree, char key[], char update_column[], char update_key[])
{
	struct node *root = ALLOC(struct node);
	read_file(tree, root, tree->root);

	update_helper_pk(tree, key, root, update_column, update_key);
}

struct student * intialize_student(char *record)
{
	struct student *newnode = NULL;
	newnode = ALLOC(struct student);
	char *copy;

	copy = strtok(record, "(,)");
	copy = strtok(NULL, "(,)");
	if (copy != NULL) {
		strcpy(newnode->enrollment_no, copy);
	}
	// printf(" \nEnrollment number : %s ", newnode->enrollment_no);

	copy = strtok(NULL, "(,)");
	if (copy != NULL) {
		strcpy(newnode->name, copy);
	}
	// printf("\nName : %s ", newnode->name);

	copy = strtok(NULL, "(,)");
	if (copy != NULL) {
		strcpy(newnode->email, copy);
	}
	// printf("\nEmail ID : %s ", newnode->email);

	copy = strtok(NULL, "(,)");
	if (copy != NULL) {
		strcpy(newnode->course_code, copy);
	}
	// printf("\nCourse Code : %s ", newnode->course_code);

	copy = strtok(NULL, "(,)");
	if (copy != NULL) {
		strcpy(newnode->gender, copy);
	}
	// printf("\nGender : %s ", newnode->gender);

	return newnode;	
}

void disp_node(struct node *nod) 
{
	char ar[] = {"|"};
	int i;
	for (i = 0; i < nod->key_count; i++) {
		// printf(UL "%10s%5s%10s%5s%10s%5s%10s%5s%10s\n" RESET, nod->students[i].enrollment_no, ar, nod->students[i].name, ar, nod->students[i].email, ar, nod->students[i].course_code, ar, nod->students[i].gender);
		printf( IT "%20s%10s%20s%10s%20s%10s%20s%10s%5s    %s\n" RESET, nod->students[i].enrollment_no, ar, nod->students[i].name, ar, nod->students[i].email, ar, nod->students[i].course_code, ar, nod->students[i].gender, ar);
	}
}

void check_in_node(struct bTree *tree, struct node *nod, char column_name[], char update_column[], char search_key[], char update_key[], int *rows_updated) 
{
	int i;
	for (i = 0; i < nod->key_count; i++) {
		if (strcmp(column_name, "name") == 0) {
			if (strcmp(nod->students[i].name, search_key) == 0) {
				strcmp(update_column, "name") == 0 ? strcpy(nod->students[i].name, update_key) :
					strcmp(update_column, "email") == 0 ? strcpy(nod->students[i].email, update_key):
					strcmp(update_column, "course_code") == 0 ? strcpy(nod->students[i].course_code, update_key) : strcpy(nod->students[i].gender, update_key);
				// printf("updated\n");
				write_file(tree, nod, nod->location_in_disk, 1);
				(*rows_updated)++;
				// return;
			}
		}
		if (strcmp(column_name, "email") == 0) {
			if (strcmp(nod->students[i].email, search_key) == 0) {
				strcmp(update_column, "name") == 0 ? strcpy(nod->students[i].name, update_key) :
					strcmp(update_column, "email") == 0 ? strcpy(nod->students[i].email, update_key):
					strcmp(update_column, "course_code") == 0 ? strcpy(nod->students[i].course_code, update_key) : strcpy(nod->students[i].gender, update_key);
				// printf("updated\n");
				write_file(tree, nod, nod->location_in_disk, 1);
				// return;
				(*rows_updated)++;
			}
		}
		if (strcmp(column_name, "course_code") == 0) {
			if (strcmp(nod->students[i].course_code, search_key) == 0) {
				strcmp(update_column, "name") == 0 ? strcpy(nod->students[i].name, update_key) :
					strcmp(update_column, "email") == 0 ? strcpy(nod->students[i].email, update_key):
					strcmp(update_column, "course_code") == 0 ? strcpy(nod->students[i].course_code, update_key) : strcpy(nod->students[i].gender, update_key);
				// printf("updated\n");
				write_file(tree, nod, nod->location_in_disk, 1);
				// return;
				(*rows_updated)++;
			}	
		}
		if (strcmp(column_name, "gender") == 0) {
			if (strcmp(nod->students[i].gender, search_key) == 0) {
				strcmp(update_column, "name") == 0 ? strcpy(nod->students[i].name, update_key) :
					strcmp(update_column, "email") == 0 ? strcpy(nod->students[i].email, update_key):
					strcmp(update_column, "course_code") == 0 ? strcpy(nod->students[i].course_code, update_key) : strcpy(nod->students[i].gender, update_key);
				// printf("updated\n");
				write_file(tree, nod, nod->location_in_disk, 1);
				// return;
				(*rows_updated)++;
			}
		}
	}
}

void update(struct bTree *tree, int root, char column_name[], char update_column[], char search_key[], char update_key[], int *rows_updated) 
{
	// printf("traverse called with",)
	if (-1 == root) {    
		return;
	}

	struct node *to_print = ALLOC(struct node);

	read_file(tree, to_print, root);

	check_in_node(tree, to_print, column_name, update_column, search_key, update_key, rows_updated);
	int i;
	for (i = 0; i < 2 * tree->min_degree; i++) {
		update(tree, to_print->children[i], column_name, update_column, search_key, update_key, rows_updated);
	}

	free(to_print);    
}

void check_where(struct node *nod, char column_name[], char column_key[])
{
	int i;
	char ar[] = {"|"};
	for (i = 0; i < nod->key_count; i++) {
		if (strcmp(column_name, "name") == 0) {
			if (strcmp(nod->students[i].name, column_key) == 0) {
				printf( IT "%20s%10s%20s%10s%20s%10s%20s%10s%5s    %s\n" RESET, nod->students[i].enrollment_no, ar, nod->students[i].name, ar, nod->students[i].email, ar, nod->students[i].course_code, ar, nod->students[i].gender, ar);
			}
		}

		if (strcmp(column_name, "email") == 0) {
			if (strcmp(nod->students[i].email, column_key) == 0) {
				printf( IT "%20s%10s%20s%10s%20s%10s%20s%10s%5s    %s\n" RESET, nod->students[i].enrollment_no, ar, nod->students[i].name, ar, nod->students[i].email, ar, nod->students[i].course_code, ar, nod->students[i].gender, ar);
			}
		}
		if (strcmp(column_name, "course_code") == 0) {
			if (strcmp(nod->students[i].course_code, column_key) == 0) {
				printf( IT "%20s%10s%20s%10s%20s%10s%20s%10s%5s    %s\n" RESET, nod->students[i].enrollment_no, ar, nod->students[i].name, ar, nod->students[i].email, ar, nod->students[i].course_code, ar, nod->students[i].gender, ar);
			}	
		}
		if (strcmp(column_name, "gender") == 0) {
			if (strcmp(nod->students[i].gender, column_key) == 0) {
				printf( IT "%20s%10s%20s%10s%20s%10s%20s%10s%5s    %s\n" RESET, nod->students[i].enrollment_no, ar, nod->students[i].name, ar, nod->students[i].email, ar, nod->students[i].course_code, ar, nod->students[i].gender, ar);
			}
		}
	}
}

void traverse(struct bTree *tree, int root) 
{
	// printf("traverse called with",)
	if (-1 == root) {    
		return;
	}

	struct node *to_print = ALLOC(struct node);

	read_file(tree, to_print, root);

	disp_node(to_print);
	int i;    
	for (i = 0; i < 2 * tree->min_degree; i++) {
		traverse(tree, to_print->children[i]);
	}

	free(to_print);    
}

void traverse_where(struct bTree *tree, int root, char column_name[], char column_key[]) 
{
	// printf("traverse called with",)
	if (-1 == root) {    
		return;
	}

	struct node *to_print = ALLOC(struct node);

	read_file(tree, to_print, root);

	check_where(to_print,column_name, column_key);
	int i;    
	for (i = 0; i < 2 * tree->min_degree; i++) {
		traverse_where(tree, to_print->children[i], column_name, column_key);
	}

	free(to_print);    
}

void read_string(char str[]) 
{
	int i = 0;
	char c;
	while ((c = getchar()) != '\n') {
		str[i++] = c;
	}

	str[i] = '\0' ;
}

void select_statement(struct bTree *bt, char column_name[], char column_key[], int mode) 
{
	char column_1[] = {"ENROLLMENT_NO"};
	char ar[] = {"|"};
	char column_2[] = {"NAME"};
	char column_3[] = {"EMAIL"};
	char column_4[] = {"COURSE_CODE"};
	char column_5[] = {"GENDER"};
	struct student *nod = NULL;
	// printf(UL "%10s%2s%11s%5s%10s%17s%10s%4s%10s\n" RESET, column_1, ar, column_2, ar, column_3, ar, column_4, ar, column_5);
	printf(UL IT "%20s%10s%20s%10s%20s%12s%20s%10s%5s   %s\n" RESET, column_1, ar, column_2, ar, column_3, ar, column_4, ar, column_5,ar);
	if (mode == 0) {
		traverse(bt, bt->root);
	} else {
		if (strcmp(column_name,"enrollment_no") == 0) {
			nod = search(bt, column_key);
			if(nod == NULL) {
				printf(RED "No Data Found\n" RESET);
			} else {
				char ar[] = {"|"};
				printf(IT "%20s%10s%20s%10s%20s%10s%20s%10s%5s    %s\n" RESET, nod->enrollment_no, ar, nod->name, ar, nod->email, ar, nod->course_code, ar, nod->gender, ar);
			}
		} else {
			traverse_where(bt, bt->root, column_name, column_key);
		}
	}
}

void exit_database(struct bTree *bt)
{
	exit_db(bt);
	printf("Successfully closed the DB\n");
	exit(0);
}

int main()
{
	struct bTree *bt = NULL;
	struct student *temp = NULL;
	int i;
	char *str;
	char *token;
	char *check;
	char *key;
	char c;	
	int mode;	
	bt = intialize_tree(T, 1);
	// printf("btree->next_pos : %d\n", bt->next_pos);
	i = 0;
	token = (char *) malloc(100 * sizeof(char));
	check = (char *) malloc(100 * sizeof(char));
	printf( UL CYN "Welcome to Strikers DB.\nGCC[5.4.0].\nDeveloped and maintained by Synergy Strikers.\n" RESET);
	while (1) {
		printf(BOLD YEL "SQL >> " RESET);
		str = (char *) malloc(100 * sizeof(char));
		read_string(str);
		// printf("%s\n",str);
		switch (str[0]) {

			case 'i' : {
					   strcpy(token, str);
					   check = strtok(token, "(,)");
					   check = strtok(NULL, "(,)");
					   if (bt->next_pos == 0 || search(bt, check) == NULL ) {
						   temp = intialize_student(str);
						   insert(bt, temp);
						   temp = NULL;
						   printf(GRN "1 Row inserted successfully\n" RESET);
						   // traverse(bt, bt->root);	
					   } else {
						   printf(RED "\nERROR %d : Primary key violated\n", rand() % 100000);
					   }
				   }
				   break;


			case 'e' : 		exit_database(bt);
						break;


			case 'u' : {
					   int rows_updated = 0;
					   strcpy(token, str);
					   check = strtok(token, "=''");
					   // while(check != NULL) {
					   // 	printf("token : %s\n", check);
					   // 	check = strtok(NULL, "=''");
					   // }
					   check = strtok(NULL, "=''");
					   char column_name[100];
					   char update_column[100];
					   char update_key[100];
					   char search_key[100];
					   if (check != NULL) {
						   strcpy(update_column, check);
						   check = strtok(NULL, "=''");
					   }
					   if (check != NULL) {
						   strcpy(update_key, check);
						   check = strtok(NULL, "=''");
					   }
					   if (check != NULL) {
						   check = strtok(NULL, "=''");
					   }
					   if (check != NULL) {
						   strcpy(column_name, check);
						   check = strtok(NULL, "=''");
					   }
					   if (check != NULL) {
						   strcpy(search_key, check);
						   check = strtok(NULL, "=''");
					   }
					   if (strcmp(column_name,"enrollment_no") == 0) {
						   update_pk(bt, search_key, update_column, update_key);
						   printf(GRN "1 row updated\n" RESET);
					   } else {
						   update(bt, bt->root, column_name, update_column, search_key, update_key, &rows_updated);
						   printf(GRN "%d rows updated\n" RESET, rows_updated);
					   }
				   }
				   break;
			case 's' : {
					   char column_name1[100];
					   char column_key1[100];
					   if(strlen(str) > 30) {
						   mode = 1;
						   check = strtok(str, "=''");
						   check = strtok(NULL, "=''");
						   if (check != NULL) {
							   strcpy(column_name1, check);
							   check = strtok(NULL, "=''");
						   }
						   if (check != NULL) {
							   strcpy(column_key1, check);
							   check = strtok(NULL, "=''");
						   } 
					   } else {
						   mode = 0;
					   }

					   if (bt->next_pos != 0) {
						   select_statement(bt, column_name1, column_key1, mode);
					   } else {
						   printf(RED "No data found\n");
					   }
				   }	
				   break;


			default : break;
		}
	}

	return 0;   
}

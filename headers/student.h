#include<stdio.h>
#define ALLOC(x) (x *)malloc(sizeof(x));

struct student {
	char enrollment_no[11]; //mit2018001
	char name[30];
	char email[23];
	char course_code[4];
	char gender[2];
};
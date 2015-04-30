/*======================================================================
    xrot
========================================================================*/

#ifndef COURSE
#define COURSE 7
#endif

#define NAME_MAX 10

extern unsigned int record[COURSE][3];
extern char *name[COURSE][3];

extern char rank_name[NAME_MAX+1];

/* function */
int check_record();
void get_name();
void show_record();

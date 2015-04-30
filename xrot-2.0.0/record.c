/*======================================================================
    xrot
========================================================================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pwd.h>
#include <errno.h>

#ifndef RECDIR
#define RECDIR "/usr/local/games/lib/xrot"
#endif
#ifndef RECFILE
#define RECFILE "xrot_record"
#endif

#include "record.h"
extern int select_course;
extern void conv_time();

#ifdef SECURE_RECORDFILE
#define PERM 0644
#else
#define PERM 0666
#endif

#define LOCK_FILE "xrot_lock"

static int lc_fd;
static int rec_fd;
static int home_dir;

static int non_write;
static int new_file;
static int update;

static char lockfile[100];
static char recordfile[100];

int course;

char rank_name[NAME_MAX+1];

void read_recfile();
void lock_file();
void read_record();
void write_recfile();
int rank( unsigned int rec_time );


unsigned int record[COURSE][3] = {
    {59999,59999,59999},
    {59999,59999,59999},
    {59999,59999,59999},
    {59999,59999,59999},
    {59999,59999,59999},
    {59999,59999,59999},
    {59999,59999,59999}
};

char *name[COURSE][3] = {
    {"none", "none", "none"},
    {"none", "none", "none"},
    {"none", "none", "none"},
    {"none", "none", "none"},
    {"none", "none", "none"},
    {"none", "none", "none"},
    {"none", "none", "none"}
};

static char name_heap[(NAME_MAX+1)*COURSE*3];
static char *heap_p;

void get_name()
{
    struct passwd *pw;

    pw = getpwuid(getuid());
    strncpy( rank_name, pw->pw_name, NAME_MAX );
    rank_name[NAME_MAX] = '\0';
}
    
int check_record( rec_time )
unsigned int rec_time;
{
    int high;

    course = select_course - 1;
#ifndef NOREC
    read_recfile();
#endif
    high = rank( rec_time );
#ifndef NOREC
    if( non_write == 0 ){
	if( high > 0 || new_file || update )
	    write_recfile();
	unlink( lockfile );
    }
    close( lc_fd );
#endif
    return high;
}

#ifndef NOREC
void show_record()
{
    int c,i;
    char buf_t[10];

    read_record();
    for( c = 0; c < COURSE; c++ ){
	printf( "\nCOURSE %d\n", (c+1) );
	for( i = 0; i < 3; i++ ){
	    conv_time( record[c][i], buf_t );
	    printf("%10s  %s\n", name[c][i], buf_t );
	}
    }
    printf("\n");
}

void read_recfile()
{
    non_write = 0;
    new_file = 0;
    home_dir = 0;
    update = 0;

    lock_file();
    read_record();
}

void lock_file()
{
    int retry_count = 0;

    sprintf( lockfile, "%s/%s", RECDIR, LOCK_FILE );
  retry:
    if( (lc_fd = open( lockfile, O_CREAT | O_EXCL, 666 )) < 0 )
	if( errno == EEXIST ){
	    u_sleep(200000);
	    retry_count++;
	    if( retry_count > 15 ){
		fprintf( stderr, "record file is locked: %s\n", lockfile );
		non_write = 1;
		return;
	    }
	    goto retry;
	}else{
	    if( home_dir == 0 ){
		home_dir = 1;
		sprintf( lockfile, "%s/%s", getenv("HOME"), LOCK_FILE );
		sprintf( recordfile, "%s/%s", getenv("HOME"), ".xrot_record" );
		goto retry;
	    }else{
		non_write = 1;
		fprintf( stderr, "can\'t create lock file\n" );
		return;
	    }
	}
    fchmod( lc_fd, 666 );
}

void read_record()
{
    int len;
    unsigned char *buf, rec_time[5];
    unsigned char *buf_p;
    int rec_t;
    int i, j, k;
    int course = COURSE;

    sprintf( recordfile, "%s/%s", RECDIR, RECFILE );
    if( (rec_fd = open( recordfile, O_RDWR | O_CREAT, PERM )) < 0 ){
	sprintf( recordfile, "%s/%s", getenv("HOME"), ".xrot_record" );
	if( (rec_fd = open( recordfile, O_RDONLY | O_CREAT, PERM )) < 0 ){
	    if( non_write == 0 )
		unlink( lockfile );
	    non_write = 1;
	    fprintf( stderr, "can\'t read record file\n" );
	    return;
	}
    }

    len = COURSE*3*(NAME_MAX+1+5);
    buf = (unsigned char *)malloc(len);
    if( read( rec_fd, buf, len ) != len ){
	len = 5*3*(NAME_MAX+1+5);  /* ver.1.2 */
	lseek( rec_fd, 0L, 0 );
	if( read( rec_fd, buf, len ) != len ){
	    new_file = 1;
	    return;
	}
	update = 1;
	course = 5;
    }

    buf_p = buf;
    heap_p = name_heap;
    for( i = 0; i < course; i++ )
	for( j = 0; j < 3; j++ ){
	    strncpy( heap_p, buf_p, NAME_MAX+1 );
	    name[i][j] = heap_p;
	    heap_p += NAME_MAX+1;
	    buf_p += NAME_MAX+1;

	    strncpy( rec_time, buf_p, 5 );
	    rec_t = 0;
	    for( k = 0; k < 5; k++ )
		rec_t = rec_t * 10 + (rec_time[k]-'0');
	    record[i][j] = rec_t;
	    buf_p += 5;
	}

    close(rec_fd);
    free(buf);
}

void write_recfile()
{
    unsigned char rec_name[NAME_MAX+1], rec_time[6];
    int i, j;

    if( (rec_fd = open( recordfile, O_WRONLY | O_CREAT, PERM )) < 0 ){
	fprintf( stderr, "can\'t write record file\n" );
	return;
    }
    if( new_file )
	fchmod( rec_fd, PERM );

    for( i = 0; i < COURSE; i++ )
	for( j = 0; j < 3; j++ ){
	    strncpy( rec_name, name[i][j], NAME_MAX );
	    rec_name[NAME_MAX] = '\0';
	    write( rec_fd, rec_name, NAME_MAX+1);

	    sprintf( rec_time, "%05d", record[i][j] );
	    write( rec_fd, rec_time, 5 );
	}

    close( rec_fd );
}
#endif

int rank( rec_time )
unsigned int rec_time;
{
    if( rec_time < record[course][0] ){
	record[course][2] = record[course][1];
	record[course][1] = record[course][0];
	record[course][0] = rec_time;
	name[course][2] = name[course][1];
	name[course][1] = name[course][0];
	name[course][0] = rank_name;
	return 1;
    }
    if( rec_time < record[course][1] ){
	record[course][2] = record[course][1];
	record[course][1] = rec_time;
	name[course][2] = name[course][1];
	name[course][1] = rank_name;
	return 2;
    }
    if( rec_time < record[course][2] ){
	record[course][2] = rec_time;
	name[course][2] = rank_name;
	return 3;
    }
    return 0;
}

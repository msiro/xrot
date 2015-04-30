/*======================================================================
    xrot
========================================================================*/

#include <sys/time.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "xrot.h"

#define TEST_COUNT 10

void test_sub( struct timeval* st, struct timeval* et );
void test_screen();

static char *t;
static int d_buf;

/* speed check rotate routine */

void check_speed()
{
    int td[TEST_COUNT], ave;
    double r, ave_r;
    struct timeval s, e;
    int i;

    vwidth = VWIDTH;
    vheight = VHEIGHT;

    for( i = 0; i < TEST_COUNT; i++ ){
	test_sub( &s, &e );     /* return time */
	td[i] = (e.tv_sec - s.tv_sec) * 1000000;
	if( e.tv_usec >= s.tv_usec )
	    td[i] += (e.tv_usec - s.tv_usec);
	else{
	    td[i] -= 1000000;
	    td[i] += ((1000000 + e.tv_usec) - s.tv_usec);
	}
    }
    for( ave = 0, i = 0; i < TEST_COUNT; i++ ){
	ave += td[i];
    }
    ave /= TEST_COUNT;
    ave_r = (double) ave * 1.4;

/* small resolution */
    if( ave_r > (MSPF*1000.0) ){
	r = ave_r / (MSPF*1000.0);
	r = sqrt(r);
	vwidth = (int) (vwidth / r);
	vheight = (int) (vheight / r);
    }
}

void test_sub( st, et )
struct timeval *st;
struct timeval *et;
{
/* prepare */
    int i;
    bw = bh = 1000;
    loc_x = vwidth / 2;
    loc_y = vheight;
    t = (char *)malloc(vwidth*vheight);
    bdata = (char *)malloc(bw*bh);
    for( i = 0; i <= vwidth*vheight*2; i++ )
	*(bdata+i) = 1;
    d_buf = 0;

    gettimeofday( st, &tzone );
    /* test phaze */
    test_screen();
    /* test end */
    gettimeofday( et, &tzone );

    free(t);
    free(bdata);
}
    
void test_screen()
{
    register char *p, *bg;
    register char a;
    register int uf, vf;
    register int u, v;
    register char *bdata_r;
    register int s, c;
    register int loc_x_r, loc_y_r;
    register int x;
    register int ub, vb;
    register int y;

    s = tsi[DEG-1];
    c = tco[DEG-1];
    uf = vwidth / 2; vf = vheight / 2;
    u = -uf * c + -vf * s;
    v = uf * s + -vf * c;
    p = t+d_buf;
    bg = t;
    bdata_r = bdata;
    loc_x_r = loc_x;
    loc_y_r = loc_y;
    for( y = 0; y < vheight; y++ ){
        ub = u; vb = v;
        for( x = 0; x < vwidth; x++ ){
            uf = u; vf = v;
#ifdef SHLOGIC
            uf /= R; vf /= R;
#else
            uf >>= P; vf >>= P;
#endif
            uf += loc_x_r; vf += loc_y_r;
            if( uf < 0 || uf > 999 || vf < 0 || vf > 999 )
                a = 0;
            else
                a = (*(bdata_r + uf + vf * 1000));
	    if( a ){
		*(p++) = a; bg++;
	    }else{
		*(p++) = a; *(bg++)=a;
	    }
            u += c; v -= s;
        }
        u = ub + s; v = vb + c;
    }
    d_buf = 1-d_buf;;
}

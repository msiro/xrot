/*======================================================================
    xrot
========================================================================*/

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <memory.h>
#include <sys/time.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "xrot.h"
#include "xwin.h"
#include "record.h"

void screen();
void test_shift();
void get_arg( int argc, char* argv[] );
void movex();
void action();
void init();
void init_val();
void init_table();
void goal();

struct timeval s_time;
struct timeval e_time;
struct timezone tzone;

/* internal valiable */
int deg;
int loc_x, loc_y;
int sx, sy;
int vx, vy;

#define B_R 10
int bound[3] = {15,11,20}; /* x10 */
int sup_v[2] = {THR*8, THR*9}; /* max speed */

int goalf;
int ball_type;

int select_course;

/* sin,cos table */
int tco[DEG];
int tsi[DEG];

int *back_data;

/* interval timeer */
int wait_f;
struct itimerval val, oval;

int over_time;

/* option */
int size_f;

#ifndef SIG_SET
struct sigaction act;
#endif

int main(argc, argv)
int argc;
char *argv[];
{
#ifndef SHLOGIC
    test_shift();
#endif
    get_arg( argc, argv );
    
    init();
    game_loop();

    return 0;
}

void test_shift()
{
    int test = -8;

    if( (test>>1) != -4 ){
	fprintf( stderr, "Please recompile xrot with SHIFT_LOGICAL\n" );
	exit(1);
    }
}

game_loop()
{
  st:
    title();
    select_course = course_select();
    restart = 0;

  rest:
    if( pre_ac() == 1 ){ /* count down */
	free_xres();
	goto st;
    }
    gettimeofday( &s_time, &tzone ); 
    set_itimer();
    action();
    off_itimer();
    gettimeofday( &e_time, &tzone );
    if( restart )
	goto rest;
    if( goalf )
	goal();
    free_xres();
    goto st;
}

void get_arg( argc, argv )
int argc;
char *argv[];
{
    int i;
    int wid,hei;

    owncmap = 0;
    size_f = 0;
    rank_name[0] = '\0';
    for( i = 1; i < argc; i++ ){
/*	if( !strcmp( argv[i], "-owncmap" ) ){
	    owncmap = 1;
	}else if( !strcmp( argv[i], "-size" ) ){
	    size_f = 1;
	    if( argc <= ++i )
		goto usage;
	    sscanf(argv[i], "%dx%d", &wid,&hei);
	    if( wid < 0 || wid > VWIDTH || hei < 0 || hei > VHEIGHT )
		goto usage;
	    vwidth = wid;
	    vheight = hei;
	}else*/
        if( !strcmp( argv[i], "-name" ) ){
	    if( argc <= ++i )
		goto usage;
	    strncpy( rank_name, argv[i], NAME_MAX );
	    rank_name[NAME_MAX] = '\0';
#ifndef NOREC
	}else if( !strcmp( argv[i], "-record" ) ){
	    show_record();
	    exit(0);
#endif
#ifdef MITSHM
	}else if( !strcmp( argv[i], "-noshm" ) ){
	    shm = 0;
#endif
	}else{
	  usage:
	    fprintf( stderr, "\nusage: xrot [-options ...]\n" );
	    fprintf( stderr, "\nwhere options include:\n" );
//	    fprintf( stderr, "\t-owncmap\t\tcreate private colormap\n" );
//	    fprintf( stderr,
//      "\t-size WIDTHxHEIGHT\tspecifies view window size (max. 200x200)\n");
	    fprintf( stderr,
	    "\t-name nickname\t\tspecifies nickname used time ranking\n" );
#ifndef NOREC
	    fprintf( stderr, "\t-record\t\t\tshow time ranking\n" );
#endif
#ifdef MITSHM
	    fprintf( stderr, "\t-noshm\t\t\tNot use shared memory\n" );
#endif
	    fprintf( stderr,
		     "\nATTENSION: This program runs only TrueColor mode\n" );
	    fprintf( stderr, "\nkeys:\n-----\n");
	    fprintf( stderr, "\tRight:\trotate clockwise\n" );
	    fprintf( stderr, "\tLeft :\trotate anticlockwise\n" );
	    fprintf( stderr, "\tUP   :\tjumping ball\n" );
	    fprintf( stderr, "\tDown :\treduce bouncing\n" );
	    fprintf( stderr, "\tSpace:\tspeed up\n" );
	    fprintf( stderr, "\tEnter:\trestart course\n" );
	    fprintf( stderr, "\tEsc  :\treturn to title\n" );
	    fprintf( stderr, "\tq    :\texit\n\n" );
	    exit(1);
	}
    }
}

void goal()
{
    unsigned int cs_time;
    int high;

    XFillRectangle(dp,win,ballGC[1][ball_type],ball_x, ball_y, ball_w, ball_h);
    num_state = 4;
    draw_mesg();
    u_sleep(500000);

    clear_screen();
    cs_time = (e_time.tv_sec - s_time.tv_sec) * 100;
    if( e_time.tv_usec >= s_time.tv_usec )
	cs_time += (e_time.tv_usec - s_time.tv_usec) / 10000;
    else{
	cs_time -= 100;
	cs_time += ((1000000 + e_time.tv_usec) - s_time.tv_usec) / 10000;
    }

    high = check_record( cs_time );
    draw_result(high, cs_time);
    pre_event();
    goal_event(high, cs_time);
}

int pre_ac()
{
    int a, i;

    if( restart == 0 ){
	a = (rand() & 0x60) >> 5;
	switch(a){
	case 1: case 2:
	    ball_type = 1;
	    break;
	case 0: case 3:
	    ball_type = 0;
	}

	set_background(a);
	create_course();
    }

    init_val();
    draw_win();
    screen();
    for( i = 2; i >= 0; i-- ){
	num_state = i;
	draw_mesg();
	u_sleep(700000);
	if( pre_ev() )
	    return 1;
    }
    num_state = 3;
    draw_mesg();
    return 0;
}
    
void init()
{
    init_table();
    if( strlen(rank_name) == 0 )
	get_name();
    if( size_f == 0 )
	check_speed();
    srand(time(0));

    signal( SIGINT, end_prog );
    signal( SIGPIPE, end_prog );
    signal( SIGTERM, end_prog );
#ifdef SIG_SET    
    signal( SIGALRM, alarm_receive );
#else
    memset( &act, 0, sizeof(act) );
    act.sa_handler = alarm_receive;
    sigaction( SIGALRM, &act, NULL );
#endif
    
    init_X();
}

void init_val()
{
    int a;

    a = select_course-1;
    sx = sy = 0;
    vx = vy = 0;
    key_space = jump_key =
	key_left = key_right = 0;
    if( a != 5 ){
	loc_x = lx[a];
	loc_y = ly[a];
	deg = sdeg[a];
    }else{ /* a == 5 */
	a = (random() >> 3) % 3;
	loc_x = c6_x[a];
	loc_y = c6_y[a];
	deg = c6_deg[a];
    }
    goalf = 0;
    escape = 0;
    restart = 0;
    over_time = 0;
}

void init_table()
{
    int i;
    double theta;

    theta = 0.0;
    for( i = 0; i < DEG; i++ ){
	tco[i] = (int) (cos(theta) * R);
	tsi[i] = (int) (sin(theta) * R);
	theta += 2 * M_PI / DEG;
    }
}

void screen()
{
    register int *p, *bg;
    register int a;
    register int uf, vf;
    register int u, v;
    register int *bdata_r;
    register int s, c;
    register int loc_x_r, loc_y_r;
    register int x;
    register int ub, vb;
    register int y;

    s = tsi[deg];
    c = tco[deg];
    uf = vwidth >> 1; vf = vheight >> 1;
    u = -uf * c + -vf * s;
    v = uf * s + -vf * c;
    p = (int*)image[dbl_buf];
    bg = back_data;
    bdata_r = (int*)bdata;
    loc_x_r = loc_x;
    loc_y_r = loc_y;
    for( y = 0; y < RHEIGHT; y++ ){
	ub = u; vb = v;
	for( x = 0; x < RWIDTH; x++ ){
	    uf = u; vf = v;
#ifdef SHLOGIC
	    uf /= R; vf /= R;
#else
	    uf >>= P; vf >>= P;
#endif
	    uf += loc_x_r; vf += loc_y_r;
	    if( (uf & MMASK) || (vf & MMASK) )
		a = 0;
	    else
		a = (*(bdata_r + uf + (vf<<MP)));
	    if( a ){
		*(p++) = a; bg++;
	    }else
		*(p++) = *(bg++);
	    u += c; v -= s;
	}
	u = ub + s; v = vb + c;
    }
#ifdef MITSHM
    if(shm)
	XShmPutImage( dp, win, copyGC, ximage[dbl_buf],
		            0, 0, view_x, view_y, vwidth, vheight, False );
    else
#endif
	XPutImage( dp, win, copyGC, ximage[dbl_buf],
		            0, 0, view_x, view_y, vwidth, vheight );
    dbl_buf = 1 - dbl_buf;
    XFlush( dp );
}

void movex()
{
    register int *cent;
    register int reg;
    register int dx,dy;
    register int key_space_r;
    register int reg2;
    register int ball_rest;

    deg += (key_right - key_left);
    deg &= 0x3f;

    key_space_r = key_space;

#ifdef SHLOGIC
    dx = (AC * tsi[deg])/R;
    dy = (AC * tco[deg])/R;
#else
    dx = (AC * tsi[deg])>>P;
    dy = (AC * tco[deg])>>P;
#endif
#ifdef SHLOGIC
    vx += (dx+dx/2*key_space_r);
    vy += (dy+dy/2*key_space_r);
#else
    vx += (dx+(dx>>1)*key_space_r);
    vy += (dy+(dy>>1)*key_space_r);
#endif
    if( jump_key ){
	jump_key = 0;
	if( vx > 0 )
	    reg = vx;
	else
	    reg = -vx;
	if( vy > 0 )
	    reg2 = vy;
	else
	    reg2 = -vy;
	if( reg < THR ) /* jump */
	    vx = -(dx<<4);
	if( reg2 < THR )
	    vy = -(dy<<4);
    }
    reg = sup_v[key_space_r];
    if(vx < -reg)
	vx = -reg;
    else if(vx > reg)
	vx = reg;
    if(vy < -reg)
	vy = -reg;
    else if(vy > reg)
	vy = reg;

    sx += vx;
    dx = sx / THR;
    sx -= dx << TP;

    sy += vy;
    dy = sy / THR;
    sy -= dy << TP;

    reg = g_pixel;
    cent = (((int*)bdata) + (loc_x+dx) + ((loc_y+dy)<<MP));
    if( *(cent+BALL) == reg || *(cent-BALL) == reg ||
    *(cent-0x2000) == reg || *(cent+0x2000) == reg || *(cent+6150) == reg ){
	goalf = 1;
	loc_x += dx;
	loc_y += dy;
	return;
    }

/* bound ball */
    if( soft_key > 0 ){
	soft_key--;
	key_space_r = 2;
    }
    cent = (((int*)bdata) + loc_x + (loc_y << MP));
    ball_rest = (dx <= 1) & (dx >= -1) & (dy <= 1) & (dy >= -1);
    if( dx > 0 )
	for( reg = 1; reg <= dx; reg++ ){
	    loc_x++;
	    cent++;
	    if( *(cent+BALL) || *(cent+6150) || *(cent-6138) )
		goto bound1;
	    if( *(cent-0x2000) ){
		if( ball_rest )
		    sy = RT;
		goto bound1;
	    }
	    if( *(cent+0x2000) ){
		if( ball_rest )
		    sy = -RT;
		goto bound1;
	    }
	    continue;
	  bound1:
	    vx = -vx*B_R/bound[key_space_r];
	    sx = 0;
	    loc_x--;
	    cent--;
	    break;
	}
    else
	for( reg = -1; reg >= dx; reg-- ){
	    loc_x--;
	    cent--;
	    if( *(cent-BALL) || *(cent+6138) || *(cent-6150) )
		goto bound2;
	    if( *(cent-0x2000) ){
		if( ball_rest )
		    sy = RT;
		goto bound2;
	    }
	    if( *(cent+0x2000) ){
		if( ball_rest )
		    sy = -RT;
		goto bound2;
	    }
	    continue;
	  bound2:
	    vx = -vx*B_R/bound[key_space_r];
	    sx = 0;
	    loc_x++;
	    cent++;
	    break;
	}
    if( dy > 0 )
	for( reg = 1; reg <= dy; reg++ ){
	    loc_y++;
	    cent += MWIDTH;
	    if(  *(cent+0x2000) || *(cent+6138) || *(cent+6150) )
		goto bound3;
	    if( *(cent-BALL) ){
		if( ball_rest )
		    sx = RT;
		goto bound3;
	    }
	    if( *(cent+BALL) ){
		if( ball_rest )
		    sx = -RT;
		goto bound3;
	    }
	    continue;
	  bound3:
	    vy = -vy*B_R/bound[key_space_r];
	    sy = 0;
	    loc_y--;
	    break;
	}
    else
	for( reg = -1; reg >= dy; reg-- ){
	    loc_y--;
	    cent -= MWIDTH;
	    if(  *(cent-0x2000) || *(cent-6138) || *(cent-6150) )
		goto bound4;
	    if( *(cent-BALL) ){
		if( ball_rest )
		    sx = RT;
		goto bound4;
	    }
	    if( *(cent+BALL) ){
		if( ball_rest )
		    sx = -RT;
		goto bound4;
	    }
	    continue;
	  bound4:
	    vy = -vy*B_R/bound[key_space_r];
	    sy = 0;
	    loc_y++;
	    break;
	}
}

void action(){
    while(1){
	if( wait_f == 0 )
	    pause();
	else
	    over_time++;
	wait_f = 0;
	if( goalf || escape || restart )
	    return;
	check_ev();
	movex();
	screen();
    }
}

void set_itimer()
{
    val.it_interval.tv_sec = 0;
    val.it_interval.tv_usec = MSPF * 1000;
    val.it_value.tv_sec = 0;
    val.it_value.tv_usec = MSPF * 1000;
    setitimer(ITIMER_REAL, &val, &oval);
    wait_f = 0;
}

void off_itimer()
{
    val.it_value.tv_sec = 0;
    val.it_value.tv_usec = 0;
    setitimer(ITIMER_REAL, &val, &oval);
}

void alarm_receive()
{
    wait_f = 1;
#ifdef SIG_SET
    signal( SIGALRM, alarm_receive );
#endif
}

void p_err( s )
char *s;
{
    fprintf( stderr, "%s\n", s );
    end_prog();
}

void u_sleep_sel( usecs )
unsigned int usecs;
{
    struct timeval timeout;

    timeout.tv_sec = usecs / 1000000;
    timeout.tv_usec =  usecs  % 1000000;
    select( 0, NULL, NULL, NULL, &timeout );
}

void u_sleep( usecs )
unsigned int usecs;
{
#ifndef NOUSLEEP
    usleep( usecs );
#else
    u_sleep_sel( usecs );
#endif
}

/*======================================================================
    xrot
========================================================================*/

#include <string.h>
#include <signal.h>
#include <memory.h>
#include <sys/time.h>
#include <X11/Xlib.h>
#include <X11/xpm.h>
#include <X11/keysym.h>

#include "xwin.h"
#include "xrot.h"
#include "pixmap/title.xpm"

Pixmap p_title;
XImage *title_image;
char *tdata;

static XEvent ev;

Pixel *ti_col;
unsigned int ti_num;

static XpmAttributes xpatt;

int torg_x, torg_y;
int t_oy;

int k_sp;

int blink;
void alarm_title_receive();
void set_col();
void ti_draw();
void set_title_itimer();
void off_title_itimer();
void show_keys();
void keycheck( XKeyEvent ev );
void screen_title( int d, double r );


#define TCOL1 "white"

static struct itimerval val, oval;

static char *keys[] = {
"Controler Keys",
"Left : rotate anticlockwise",
"Right : rotate clockwise",
"Up : jumping ball",
"Down : reduce bouncing",
"Space : speed up",
"Enter : restart course",
"Esc : return to title",
"Thank you for playing !",
" "
};

static int key_msg = 10;

#ifndef SIG_SET
static struct sigaction act;
#endif

void p_err( char* s );

void title()
{
    register int i, j;
    double r, d;
    int dt;
    int p_status;

    XClearWindow( dp, win );
/* load title pixmap */    
  retry:
    xpatt.valuemask = XpmReturnPixels | XpmColormap;
    xpatt.colormap = cmap;
    if( (p_status = XpmCreatePixmapFromData(
	dp, win, title_xpm, &p_title, NULL, &xpatt )) != XpmSuccess ){
	if( owncmap == 0 ){
	    change_cmap();
	    goto retry;
	}
	p_err(XpmGetErrorString(p_status));
    }
    ti_col = xpatt.pixels;
    ti_num = xpatt.npixels;

    title_image = XGetImage( dp, p_title, 0, 0, 200, 100, AllPlanes, ZPixmap );
    tdata = title_image->data;

/* init val */
    torg_x = (WWIDTH - vwidth) / 2;
    torg_y = (WHEIGHT - vheight) / 2;
    t_oy = 50 << 7; /* title y */
    
    d = (5.0 - (1.0/3.0)) / (2*DEG);
    r = 1.0/3.0;
/* rotate */
    set_itimer();
    for( j = 0; j < 2; j++ )
	for( i = 0; i < DEG; i++ ){
	    if( wait_f == 0 ) pause();
	    wait_f = 0;
	    r += d;  
	    screen_title( i, 1.0/r );
	    if( ti_event() )
		goto l1;
	}
    r += 2*d;
    d = (r - (200.0/vwidth))/50.0;
    dt = (50.0 - (0.0))*128/50;  /* dif t_oy : t_oy + dt*50 >> 7 */
    wait_f = 0;
/* enlarge */
    for( i = 0; i < 50; i++ ){
	if( wait_f == 0 ) pause();
	wait_f = 0;
	r -= d;
	t_oy = t_oy + dt;
	screen_title( 0, 1.0/r );
	if( ti_event() )
	    break;
    }
  l1:
    off_itimer();

    blink = 1;
    set_col(blink);
    ti_draw();

#ifdef SIG_SET
    signal( SIGALRM, alarm_title_receive );
#else
    memset( &act, 0, sizeof(act) );
    act.sa_handler = alarm_title_receive;
    sigaction( SIGALRM, &act, NULL );
#endif

    set_title_itimer();
    show_keys();
    off_title_itimer();
#ifdef SIG_SET
    signal( SIGALRM, alarm_receive );
#else
    memset( &act, 0, sizeof(act) );
    act.sa_handler = alarm_receive;
    sigaction( SIGALRM, &act, NULL );
#endif

    XClearWindow( dp, win );
    XFlush(dp);
    /* next */
    XFreeColors( dp, cmap, ti_col, ti_num, 0 );
    XFreePixmap( dp, p_title );
    XDestroyImage( title_image );
}

void ti_draw()
{
    XCharStruct overall;
    int dir;
    int ascent_i;
    int descent;
    int s_width;
    int x;
    char *buf = "PUSH SPACE KEY";
    char *buf2 = "push 'q' to exit";
    char *buf3 = "Ver. 2.0.0";
    char *buf4 = "";

    XClearWindow( dp, win );
    t_oy = 100 << 7;
    screen_title(0,(double)vwidth/200.0);

    XSetForeground( dp, titleGC, tipix[0] );
    XTextExtents( fn2, buf, strlen(buf), &dir, &ascent_i, &descent, &overall );
    s_width = overall.rbearing - overall.lbearing;
    x = (WWIDTH - s_width) / 2 - overall.lbearing;
    XDrawString( dp, win, titleGC, x, 170+ascent2, buf, strlen(buf) );

    XSetForeground( dp, titleGC, tipix[2] );
    XTextExtents(
	fn2, buf2, strlen(buf2), &dir, &ascent_i, &descent, &overall );
    s_width = overall.rbearing - overall.lbearing;
    x = (WWIDTH - s_width) / 2 - overall.lbearing;
    XDrawString( dp, win, titleGC, x, 145+ascent2, buf2, strlen(buf2) );

    XSetForeground( dp, titleGC, tipix[1] );
    XTextExtents(
	fn2, buf4, strlen(buf4), &dir, &ascent_i, &descent, &overall );
    s_width = overall.rbearing - overall.lbearing;
    x = (WWIDTH - s_width) / 2 - overall.lbearing;
    XDrawString( dp, win, titleGC, 210, 105+ascent2, buf3, strlen(buf3) );
    XDrawString( dp, win, titleGC, x, 195+ascent2, buf4, strlen(buf4) );
}

void set_title_itimer()
{
    val.it_interval.tv_sec = 0;
    val.it_interval.tv_usec = 900000;
    val.it_value.tv_sec = 0;
    val.it_value.tv_usec = 900000;
    setitimer(ITIMER_REAL, &val, &oval);
}

void off_title_itimer()
{
    val.it_value.tv_sec = 0;
    val.it_value.tv_usec = 0;
    setitimer(ITIMER_REAL, &val, &oval);
}

void alarm_title_receive()
{
    blink = 1 - blink;
    set_col(blink);
#ifdef SIG_SET
    signal( SIGALRM, alarm_title_receive );
#endif
}

void set_col(c)
int c;
{
    XColor set_pixel, rgb;
    if(c){
        XAllocNamedColor( dp, cmap, TCOL1, &set_pixel, &rgb );
        tipix[0] = set_pixel.pixel;
    }
    else{
        XAllocNamedColor( dp, cmap, "black", &set_pixel, &rgb );
        tipix[0] = set_pixel.pixel;
    }
    XFlush( dp );
}

void show_keys()
{
    int msg, p_msg;
    XColor mcol;
    int col_count;
    XCharStruct overall;
    int dir;
    int ascent_i;
    int descent;
    int s_width;
    int x;
    int i;

    mcol.flags = DoRed | DoGreen | DoBlue;
    mcol.green = 65535;
    mcol.blue = 0;
    mcol.red = 0;
    XAllocColor( dp, cmap, &mcol );
    tipix[3] = mcol.pixel;
    XSetForeground( dp, keysGC, tipix[3] );
    
    p_msg = 0;
    msg = 1;
    k_sp = 0;
    col_count = 32;
    XTextExtents( fn2, keys[0], strlen(keys[0]),
		                &dir, &ascent_i, &descent, &overall);
    s_width = overall.rbearing - overall.lbearing;
    x = (WWIDTH - s_width) / 2 - overall.lbearing;
    XDrawString(dp, win, keysGC, x, 125+ascent2, keys[0], strlen(keys[0]));
    for( i = 0; i < 16; i++ ){
	while( XPending( dp ) ){
	    XNextEvent( dp, &ev );
	    if( ev.type == Expose ){
		ti_draw();
		XDrawString(dp, win, keysGC, x,
			        125+ascent2, keys[0], strlen(keys[0]));
	    }
	    if( ev.type == KeyPress )
		keycheck(ev.xkey);
	}
	if( k_sp == 1 )
	    return;
	u_sleep_sel(60000);
    }
    while(k_sp == 0 ){
	if( col_count == 0 ){
	    XFillRectangle( dp, win, blackGC, 50, 125, 200, 20 );
	    XTextExtents( fn2, keys[msg], strlen(keys[msg]),
			              &dir, &ascent_i, &descent, &overall);
	    s_width = overall.rbearing - overall.lbearing;
	    x = (WWIDTH - s_width) / 2 - overall.lbearing;
	    XDrawString(dp, win, keysGC, x, 125+ascent2,
			              keys[msg], strlen(keys[msg]));
	    p_msg = msg;
	    msg = (msg+1) % key_msg;
	}
	col_count++;
	col_count &= 0x3f;
	mcol.green = grad[col_count];
        XAllocColor( dp, cmap, &mcol );
        tipix[3] = mcol.pixel;
        XSetForeground( dp, keysGC, tipix[3] );
        ti_draw();
        XDrawString(dp, win, keysGC, x, 125+ascent2,
                keys[p_msg], strlen(keys[p_msg]));
        
	while( XPending( dp ) ){
	    XNextEvent( dp, &ev );
	    if( ev.type == Expose ){
		ti_draw();
		XDrawString(dp, win, keysGC, x, 125+ascent2,
			                  keys[p_msg], strlen(keys[p_msg]));
	    }
	    if( ev.type == KeyPress )
		keycheck(ev.xkey);
	}
	u_sleep_sel(60000);
    }
}

/* skip rotate */
ti_event()
{
    int r;
    k_sp = 0;
    while( XEventsQueued( dp, QueuedAfterReading ) ){
	XNextEvent( dp, &ev );
	if( ev.type == KeyPress )
	    keycheck( ev.xkey );
    }
    r = k_sp;
    return r;
}

void keycheck( ev )
XKeyEvent ev;
{
    char ch;
    KeySym ksym;

    XLookupString( &ev, &ch, 1, &ksym, NULL );
    if( ksym == XK_space || ksym == XK_1 || ksym == XK_2 || ksym == XK_3 ||
	ksym == XK_4 || ksym == XK_5 || ksym == XK_6 || ksym == XK_7 )
	k_sp = 1;
    if( ksym == XK_q ){
	end_prog();
    }
}

void screen_title( d, r )
int d;
double r;
{
    register int *p;
    register int a, bp;
    register int uf, vf;
    register int u, v;
    register int *tdata_r;
    register int s, c;
    register int ub, vb;
    register int x, y;
    register int hw, hh;

    s = tsi[d] / r;
    c = tco[d] / r;
    hw = vwidth >> 1; hh = vheight >> 1;
    u = -hw * c + -hh * s;
    v = hw * s + -hh * c;
    p = (int*)image[dbl_buf];
    tdata_r = (int*)tdata;
    bp = black_pixel;
    for( y = 0; y < RHEIGHT; y++ ){
        ub = u; vb = v;
        for( x = 0; x < RWIDTH; x++ ){
	    uf = u; vf = v;
#ifdef SHLOGICAL
            uf /= R; vf /= R;
#else
            uf >>= P; vf >>= P;
#endif
	    uf += 100;
	    vf += (t_oy >> 7);
	    if( uf < 0 || uf > 199 || vf < 0 || vf > 99 )
		a = bp;
	    else{
		a = *(tdata_r + uf + vf * 200);
		if( a == 0 )
		    a = bp;
	    }
            *(p++) = a;
            u += c; v -= s;
        }
        u = ub + s; v = vb + c;
    }
#ifdef MITSHM
    if( shm )
	XShmPutImage( dp, win, copyGC, ximage[dbl_buf],
                            0, 0, torg_x, torg_y, vwidth, vheight, False );
    else
#endif
	XPutImage( dp, win, copyGC, ximage[dbl_buf],
		            0, 0, torg_x, torg_y, vwidth, vheight );
    dbl_buf = 1 - dbl_buf;
    XFlush( dp );
}

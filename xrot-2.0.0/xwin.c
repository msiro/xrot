/*======================================================================
    xrot
========================================================================*/

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/xpm.h>
#include <sys/types.h>
#include <memory.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

#include "xrot.h"
#include "pixmap.h"
#include "xwin.h"
#include "record.h"

#include "c1.h"
#include "c2.h"
#include "c3.h"
#include "c4.h"
#include "c5.h"
#include "c6.h"
#include "c7.h"

#ifdef MITSHM
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>
XShmSegmentInfo shminfo[2];
int shm = 1;
#endif

#define PROG_NAME "xrot"

int vwidth, vheight;
int RWIDTH, RHEIGHT;

Display *dp = NULL;
Colormap cmap;
XColor colors;
unsigned long gpixel, b3pixel, otpixel, cs_pixel;
unsigned long tipix[4];
int g_pixel;
unsigned short grad[64];
int redc;
int owncmap;

#define FONT   "-*-new century schoolbook-medium-i-*--18-*-*-*-p-*-*-*"
#define FONT_CS   "-*-new century schoolbook-medium-r-*--18-*-*-*-p-*-*-*"
#define FONT2  "-*-times-bold-r-*--14-*-*-*-p-*-*-*"

Pixel *bg_col;
unsigned int bg_num;

Window win, root;
GC copyGC, clearGC, blackGC, goalGC;
GC textGC, text_csGC[2], titleGC, keysGC, fadeGC;
int sc;
XImage *ximage[2];
int dbl_buf;
XEvent ev;
XFontStruct *fn, *fn2, *fn_cs;
int ascent, ascent2, ascent_cs;

static int cs_width, cs_x, cs_text_x;
static int cs_num;
static int cs_state[COURSE];

/* pixmap */
Pixmap p_frame, p_titlem;
Pixmap num[5];
int num_state;
Pixmap back_pix;
char **bg_data[4];
XImage *back_image;
Pixmap p_wall[21];
int wall_width[21], wall_height[21];
char **p_wall_data[21];
Pixmap point_ball;

Pixmap p_ball[2], mask[2];
GC ballGC[2][2];
XpmAttributes xpatt;
int ball_x, ball_y, ball_w, ball_h;
int ball_x_bg, ball_y_bg;
int view_x, view_y;
Pixmap p_course;

Pixmap fade[25];

char *image[2];
unsigned char *temp;

int key_right, key_left;
int key_space, jump_key, soft_key;
int escape;
int restart; /* RET key */

unsigned long black_pixel, white_pixel;

struct course_d *cs_d[COURSE];
int lx[COURSE];
int ly[COURSE];
int sdeg[COURSE];
int gx[COURSE];
int gy[COURSE];
int gw[COURSE];
int gh[COURSE];

/* course data */
unsigned int bw, bh;
XImage *bimage;
char *bdata;

int width, height;

#define TCOL1 "white"
#define TCOL2 "orange"
#define TCOL3 "yellow"

void read_pixmap();
void set_fade();
void free_pix( Pixmap fpix );
void create_cmap( unsigned long* bpixel, unsigned long* wpixel );
int keycheck3(XKeyEvent ev);
void set_cs_state();
void draw_cst();
void draw_cst_sub();
void init_course();
void c7_make();
void draw_c7( int x, int y, int ty );
int keycheck2(XKeyEvent ev);
void key_check( XKeyEvent ev );

void init_X()
{
    XSizeHints size;
    XWMHints hints;
    Visual *visual;
    XColor set_pixel, rgb;
    int i;
    unsigned long bpixel, wpixel;
    unsigned long plane_mask, pixel[64];

    if( (dp = XOpenDisplay( NULL )) == NULL ){
	fprintf( stderr, "Can't connect X server\n" );
	exit(1);
    }
    sc = DefaultScreen( dp );
    root = RootWindow( dp, sc );
    visual = DefaultVisual( dp, sc );
    win = XCreateSimpleWindow( dp, root, 0, 0, WWIDTH, WHEIGHT,
			                0, 0, 0);
    bpixel = BlackPixel( dp, sc );
    wpixel = WhitePixel( dp, sc );
    cmap = DefaultColormap( dp, sc );
    if( visual->class != TrueColor ){
        p_err("This X server doesn't support TrueColor");
    }
    XSetWindowBackground( dp, win, bpixel );

    copyGC = XCreateGC( dp, root, 0, NULL );
    XSetGraphicsExposures( dp, copyGC, False );
    clearGC = XCreateGC( dp, root, 0, NULL );
    XSetForeground( dp, clearGC, 0 );
    blackGC = XCreateGC( dp, root, 0, NULL );
    XSetForeground( dp, blackGC, bpixel );
    black_pixel = bpixel;
    white_pixel = wpixel;

    XAllocNamedColor( dp, cmap, "yellow", &set_pixel, &rgb );
    b3pixel = set_pixel.pixel;
    XAllocNamedColor( dp, cmap, "red", &set_pixel, &rgb );
    otpixel = set_pixel.pixel;
    XAllocNamedColor( dp, cmap, "orange", &set_pixel, &rgb );
    cs_pixel = set_pixel.pixel;

    set_fade();

    /* font */
    textGC = XCreateGC( dp, root, 0, NULL );
    if( (fn = XLoadQueryFont( dp, FONT )) == NULL )
	p_err("Unable to load font");
    XSetFont( dp, textGC, fn->fid );
    ascent = fn->max_bounds.ascent;
    XSetForeground( dp, textGC, wpixel );

    text_csGC[0] = XCreateGC( dp, root, 0, NULL );
    if( (fn_cs = XLoadQueryFont( dp, FONT_CS )) == NULL )
	p_err("Unable to load font");
    XSetFont( dp, text_csGC[0], fn_cs->fid );
    ascent_cs = fn_cs->max_bounds.ascent;
    XSetForeground( dp, text_csGC[0], wpixel );

    text_csGC[1] = XCreateGC( dp, root, 0, NULL );
    XSetFont( dp, text_csGC[1], fn_cs->fid );
    ascent_cs = fn_cs->max_bounds.ascent;
    XSetForeground( dp, text_csGC[1], cs_pixel );

    titleGC = XCreateGC( dp, root, 0, NULL );
    if( (fn2 = XLoadQueryFont( dp, FONT2 )) == NULL )
	p_err("Unable to load font");
    XSetFont( dp, titleGC, fn2->fid );
    keysGC = XCreateGC( dp, root, 0, NULL );
    XSetFont( dp, keysGC, fn2->fid );
    ascent2 = fn2->max_bounds.ascent;
    XAllocNamedColor( dp, cmap, TCOL2, &set_pixel, &rgb );
    tipix[1] = set_pixel.pixel;
    XAllocNamedColor( dp, cmap, TCOL3, &set_pixel, &rgb );
    tipix[2] = set_pixel.pixel;

    for( i = 0; i < 64; i++ )
	grad[i] = (unsigned short) (sin(M_PI/64.0*i) * 65535.0);
    redc = 0;
    
    XSelectInput( dp, win, ExposureMask | KeyPressMask | KeyReleaseMask );

    size.flags = PMinSize | PMaxSize;
    size.min_width  = WWIDTH;
    size.min_height = WHEIGHT;
    size.max_width  = WWIDTH;
    size.max_height = WHEIGHT;
    XSetNormalHints( dp, win, &size );

    hints.flags = InputHint;
#ifdef HINT_TRUE
    hints.input = True;
#else
    hints.input = False;
#endif
    XSetWMHints( dp, win, &hints );

    XStoreName( dp, win, PROG_NAME );

    key_left = key_right = 0;
    key_space = 0;
    jump_key = 0;
    XMapWindow( dp, win );
    XFlush( dp );
  ev_loop1:
    XNextEvent( dp, &ev );
    if( ev.type != Expose )
	goto ev_loop1;

#ifdef MITSHM
    if( XShmQueryExtension( dp ) != True )
	shm = 0;
    if( shm ){
    ximage[0] = XShmCreateImage( dp, DefaultVisual( dp, sc ),
      8, ZPixmap, NULL, &shminfo[0], vwidth, vheight );
    ximage[1] = XShmCreateImage( dp, DefaultVisual( dp, sc ),
      8, ZPixmap, NULL, &shminfo[1], vwidth, vheight );
    RWIDTH = ximage[0]->bytes_per_line;
    RHEIGHT = ximage[0]->height;
    shminfo[0].shmid = shmget( IPC_PRIVATE,
      ximage[0]->bytes_per_line * ximage[0]->height, IPC_CREAT | 0777 );
    shminfo[0].shmaddr = ximage[0]->data = image[0]
	               = (char *)shmat( shminfo[0].shmid, 0, 0 );
    shminfo[0].readOnly = True;
    if( !XShmAttach( dp, &shminfo[0] ) )
	p_err("MIT-SHM error\n");
    shminfo[1].shmid = shmget( IPC_PRIVATE,
      ximage[1]->bytes_per_line * ximage[1]->height, IPC_CREAT | 0777 );
    shminfo[1].shmaddr = ximage[1]->data = image[1]
	               = (char *)shmat( shminfo[1].shmid, 0, 0 );
    shminfo[1].readOnly = True;
    if( !XShmAttach( dp, &shminfo[1] ) )
	p_err("MIT-SHM error\n");
    }else
#endif
    {
    int depth = DefaultDepth( dp, sc );
    image[0] = (char *)malloc(vwidth * vheight * 4);
    image[1] = (char *)malloc(vwidth * vheight * 4);
    ximage[0] = XCreateImage( dp, DefaultVisual( dp, sc ),
      depth, ZPixmap, 0, image[0], vwidth, vheight, 32, 0 );
    ximage[1] = XCreateImage( dp, DefaultVisual( dp, sc ),
      depth, ZPixmap, 0, image[1], vwidth, vheight, 32, 0 );
    RWIDTH = ximage[0]->width;
    RHEIGHT = ximage[0]->height;
    }	
    dbl_buf = 0;
    XSync(dp, True);

    view_x = (225-vwidth)/2;
    view_y = (225-vheight)/2;
    ballGC[0][0] = XCreateGC( dp, root, 0, NULL );
    ballGC[1][0] = XCreateGC( dp, root, 0, NULL );
    ballGC[0][1] = XCreateGC( dp, root, 0, NULL );
    ballGC[1][1] = XCreateGC( dp, root, 0, NULL );
    xpatt.valuemask = XpmColormap;
    xpatt.colormap = cmap;
    XpmCreatePixmapFromData(
	dp, root, ball_xpm, &p_ball[0], &mask[0], &xpatt );
    xpatt.valuemask = XpmColormap;
    xpatt.colormap = cmap;
    XpmCreatePixmapFromData(
	dp, root, ball2_xpm, &p_ball[1], &mask[1], &xpatt );
    XSetFillStyle( dp, ballGC[0][0], FillTiled );
    XSetClipMask( dp, ballGC[0][0], mask[0] );
    XSetFillStyle( dp, ballGC[1][0], FillTiled );
    XSetClipMask( dp, ballGC[1][0], mask[1] );
    XSetFillStyle( dp, ballGC[0][1], FillTiled );
    XSetClipMask( dp, ballGC[0][1], mask[0] );
    XSetFillStyle( dp, ballGC[1][1], FillTiled );
    XSetClipMask( dp, ballGC[1][1], mask[1] );
    ball_w = xpatt.width;
    ball_h = xpatt.height;
    ball_x_bg = (200-xpatt.width)/2;
    ball_y_bg = (200-xpatt.height)/2;
    XSetClipOrigin( dp, ballGC[0][0], ball_x_bg, ball_y_bg );
    XSetTSOrigin( dp, ballGC[0][0], ball_x_bg, ball_y_bg );
    XSetTile( dp, ballGC[0][0], p_ball[0] );
    XSetClipOrigin( dp, ballGC[0][1], ball_x_bg, ball_y_bg );
    XSetTSOrigin( dp, ballGC[0][1], ball_x_bg, ball_y_bg );
    XSetTile( dp, ballGC[0][1], p_ball[1] );
    ball_x = (vwidth-xpatt.width)/2 + view_x;
    ball_y = (vheight-xpatt.height)/2 + view_y;
    XSetClipOrigin( dp, ballGC[1][0], ball_x, ball_y );
    XSetTSOrigin( dp, ballGC[1][0], ball_x, ball_y );
    XSetTile( dp, ballGC[1][0], p_ball[0] );
    XSetClipOrigin( dp, ballGC[1][1], ball_x, ball_y );
    XSetTSOrigin( dp, ballGC[1][1], ball_x, ball_y );
    XSetTile( dp, ballGC[1][1], p_ball[1] );

    p_wall_data[0] = wall1_xpm;
    p_wall_data[1] = wall2_xpm;
    p_wall_data[2] = wall3_xpm;
    p_wall_data[3] = wall4_xpm;
    p_wall_data[4] = wall5_xpm;
    p_wall_data[5] = wall6_xpm;
    p_wall_data[6] = wall7_xpm;
    p_wall_data[7] = wall8_xpm;
    p_wall_data[8] = wall9_xpm;
    p_wall_data[9] = wall10_xpm;
    p_wall_data[10] = down_xpm;
    p_wall_data[11] = left_xpm;
    p_wall_data[12] = right_xpm;
    p_wall_data[13] = up_xpm;
    p_wall_data[14] = down1_xpm;
    p_wall_data[15] = left1_xpm;
    p_wall_data[16] = right1_xpm;
    p_wall_data[17] = up1_xpm;
    p_wall_data[18] = left2_xpm;
    p_wall_data[19] = right2_xpm;
    p_wall_data[20] = pin_xpm;
    read_pixmap();
    bg_data[0] = bg1_xpm;
    bg_data[1] = bg2_xpm;
    bg_data[2] = bg3_xpm;
    bg_data[3] = bg4_xpm;

    goalGC = XCreateGC( dp, root, 0, NULL );
    colors.flags = DoRed | DoGreen | DoBlue;
    colors.green = 0x20*0xff;
    colors.blue = 0x20*0xff;
    colors.red = 0xf0*0xff;
    XAllocColor( dp, cmap, &colors );
    gpixel = colors.pixel;
    g_pixel = (int) gpixel;
    XSetForeground( dp, goalGC, gpixel );

/* course parameter set */
    cs_d[0] = c1;
    cs_d[1] = c2;
    cs_d[2] = c3;
    cs_d[3] = c4;
    cs_d[4] = c5;
    cs_d[5] = c6;
/*    cs_d[6] = c7; */

    lx[0] = c1_x;
    ly[0] = c1_y;
    sdeg[0] = c1_deg;
    gx[0] = g1x;
    gy[0] = g1y;
    gw[0] = g1w;
    gh[0] = g1h;
    lx[1] = c2_x;
    ly[1] = c2_y;
    sdeg[1] = c2_deg;
    gx[1] = g2x;
    gy[1] = g2y;
    gw[1] = g2w;
    gh[1] = g2h;
    lx[2] = c3_x;
    ly[2] = c3_y;
    sdeg[2] = c3_deg;
    gx[2] = g3x;
    gy[2] = g3y;
    gw[2] = g3w;
    gh[2] = g3h;
    lx[3] = c4_x;
    ly[3] = c4_y;
    sdeg[3] = c4_deg;
    gx[3] = g4x;
    gy[3] = g4y;
    gw[3] = g4w;
    gh[3] = g4h;
    lx[4] = c5_x;
    ly[4] = c5_y;
    sdeg[4] = c5_deg;
    gx[4] = g5x;
    gy[4] = g5y;
    gw[4] = g5w;
    gh[4] = g5h;
    gx[5] = g6x;
    gy[5] = g6y;
    gw[5] = g6w;
    gh[5] = g6h;
    lx[6] = c7_x;
    ly[6] = c7_y;
    sdeg[6] = c7_deg;
    XAutoRepeatOff( dp );
    cs_num = 0;
}

void read_pixmap()
{
    int i;

    xpatt.valuemask = XpmColormap;
    xpatt.colormap = cmap;
    for( i = 0; i < 21; i++ ){
	XpmCreatePixmapFromData( dp, root,
				 p_wall_data[i], &p_wall[i], NULL, &xpatt );
	wall_width[i] = xpatt.width;
	wall_height[i] = xpatt.height;
    }
    XpmCreatePixmapFromData( dp, root, frame_xpm, &p_frame, NULL, &xpatt );
    XpmCreatePixmapFromData( dp, root, titlem_xpm, &p_titlem, NULL, &xpatt );
    XpmCreatePixmapFromData( dp, root, ball3_xpm, &point_ball, NULL, &xpatt );

    XpmCreatePixmapFromData( dp, root, one_xpm, &num[0], NULL, &xpatt );
    XpmCreatePixmapFromData( dp, root, two_xpm, &num[1], NULL, &xpatt );
    XpmCreatePixmapFromData( dp, root, three_xpm, &num[2], NULL, &xpatt );
    XpmCreatePixmapFromData( dp, root, go_xpm, &num[3], NULL, &xpatt );
    XpmCreatePixmapFromData( dp, root, goal_xpm, &num[4], NULL, &xpatt );
}

void change_cmap()
{
    int i;
    unsigned long bpixel, wpixel;
    unsigned long plane_mask, pixel[1];
    XColor set_pixel, rgb;

    /* free pixmap */
    free_pix( p_ball[0] );
    free_pix( p_ball[1] );
    free_pix( mask[0] );
    free_pix( mask[1] );
    for( i = 0; i < 20; i++ )
	free_pix( p_wall[i] );
    free_pix( p_frame );
    free_pix( p_titlem );
    free_pix( point_ball );
    for( i = 0; i < 5; i++ )
	free_pix( num[i] );

    owncmap = 1;
    create_cmap( &bpixel, &wpixel );

    XSetForeground( dp, textGC, wpixel );
    XSetForeground( dp, text_csGC[0], wpixel );
    XSetForeground( dp, text_csGC[1], cs_pixel );
    XAllocColorCells( dp, cmap, False, &plane_mask, 0, pixel, 1 );
    tipix[0] = pixel[0];
    XAllocNamedColor( dp, cmap, TCOL2, &set_pixel, &rgb );
    tipix[1] = set_pixel.pixel;
    XAllocNamedColor( dp, cmap, TCOL3, &set_pixel, &rgb );
    tipix[2] = set_pixel.pixel;
    XAllocColorCells( dp, cmap, False, &plane_mask, 0, pixel, 1 );
    tipix[3] = pixel[0];
    XSetForeground( dp, keysGC, tipix[3] );

    XSetForeground( dp, blackGC, bpixel );
    black_pixel = bpixel;
    XSetWindowBackground( dp, win, bpixel );

    /* ball */ 
    xpatt.valuemask = XpmColormap;
    xpatt.colormap = cmap;
    XpmCreatePixmapFromData(
	dp, root, ball_xpm, &p_ball[0], &mask[0], &xpatt );
    XpmCreatePixmapFromData(
	dp, root, ball2_xpm, &p_ball[1], &mask[1], &xpatt );
    XSetClipMask( dp, ballGC[0][0], mask[0] );
    XSetClipMask( dp, ballGC[0][1], mask[1] );
    XSetTile( dp, ballGC[0][0], p_ball[0] );
    XSetTile( dp, ballGC[0][1], p_ball[1] );
    XSetClipMask( dp, ballGC[1][0], mask[0] );
    XSetClipMask( dp, ballGC[1][1], mask[1] );
    XSetTile( dp, ballGC[1][0], p_ball[0] );
    XSetTile( dp, ballGC[1][1], p_ball[1] );

    /* misc */
    read_pixmap();

    XAllocColorCells( dp, cmap, False, &plane_mask, 0, pixel, 1 );
    tipix[0] = pixel[0];
    XAllocColorCells( dp, cmap, False, &plane_mask, 0, pixel, 1 );
    tipix[3] = pixel[0];
    XSetForeground( dp, keysGC, tipix[3] );
    XAllocColorCells( dp, cmap, False, &plane_mask, 0, pixel, 1 );
    gpixel = pixel[0];
    g_pixel = (int) gpixel;
    XSetForeground( dp, goalGC, gpixel );
    colors.pixel = gpixel;
    colors.flags = DoRed | DoGreen | DoBlue;
    colors.green = 0;
    colors.blue = 0;
    colors.red = 0;
    XStoreColor( dp, cmap, &colors );
}

void set_fade()
{
    GC fgc,bgc;
    int i,j;
    struct {
	int x;
	int y;
    } maskd[25] = {
	{0,0},{2,2},{0,2},{2,0},{1,1},{3,3},{1,3},{3,1},
	{0,4},{2,4},{4,2},{4,0},{4,4},{0,1},{2,3},{3,0},
	{1,2},{4,3},{2,1},{1,4},{3,2},{1,0},{3,4},{4,1},{0,3}
    };

    for( i = 0; i < 25; i++ )
	fade[i] = XCreatePixmap( dp, root, 5, 5, 1 );
    fgc = XCreateGC( dp, fade[0], 0, NULL );
    bgc = XCreateGC( dp, fade[0], 0, NULL );
    XSetForeground( dp, fgc, 1 );
    XSetForeground( dp, bgc, 0 );
    for( i = 0; i < 25; i++ ){
	XFillRectangle( dp, fade[i], bgc, 0, 0, 5, 5 );
	for( j = 0; j <= i; j++ )
	    XDrawPoint( dp, fade[i], fgc, maskd[j].x, maskd[j].y );
    }
    fadeGC = XCreateGC( dp, root, 0, NULL );
    XSetFillStyle( dp, fadeGC, FillStippled );
    XFreeGC( dp, fgc );
    XFreeGC( dp, bgc );
}

void free_pix( fpix )
Pixmap fpix;
{
    if( fpix )
	XFreePixmap( dp, fpix );
}

void create_cmap( bpixel, wpixel )
unsigned long *bpixel, *wpixel;
{
    Colormap new_cmap;
    XColor set_pixel, rgb;
    XVisualInfo v_info;

    if( XMatchVisualInfo( dp, sc, 8, PseudoColor, &v_info ) == 0 )
	p_err("This X server doesn't support PseudoColor (256 colors)");
    new_cmap = XCreateColormap( dp, root, v_info.visual, AllocNone );
    XInstallColormap( dp, new_cmap );
    XSetWindowColormap( dp, win, new_cmap );
    XAllocNamedColor( dp, new_cmap, "black", &set_pixel, &rgb );
    *bpixel = set_pixel.pixel;
    XAllocNamedColor( dp, new_cmap, "white", &set_pixel, &rgb );
    *wpixel = set_pixel.pixel;
    XAllocNamedColor( dp, new_cmap, "yellow", &set_pixel, &rgb );
    b3pixel = set_pixel.pixel;
    XAllocNamedColor( dp, new_cmap, "red", &set_pixel, &rgb );
    otpixel = set_pixel.pixel;
    XAllocNamedColor( dp, new_cmap, "orange", &set_pixel, &rgb );
    cs_pixel = set_pixel.pixel;
    white_pixel = *wpixel;
    black_pixel = *bpixel;

    cmap = new_cmap;
}

/*
    goal
*/

void clear_screen()
{
    int i;

    XSetForeground( dp, fadeGC, black_pixel );
    for( i = 0; i < 25; i++ ){
	XSetStipple( dp, fadeGC, fade[i] );
	XFillRectangle( dp, win, fadeGC, 0, 0, WWIDTH, WHEIGHT );
	XFlush( dp );
	u_sleep(20000);
    }
    XSync( dp, True );
    u_sleep(300000);
}

void draw_result( high, res_time )
int high;
int res_time;
{
    XCharStruct overall;
    int dir;
    int ascent_i;
    int descent;
    int s_width;
    int i;
    int x, rec_x[3], min_x;
    char form[20];

    char buf[20], n_buf[3][20+NAME_MAX], buf_t[10];

    XClearWindow( dp, win );
    sprintf(buf, "COURSE  %d", select_course);
    XTextExtents( fn, buf, strlen(buf), &dir, &ascent_i, &descent, &overall );
    s_width = overall.rbearing - overall.lbearing;
    x = (WWIDTH - s_width) / 2 - overall.lbearing;
    XDrawString( dp, win, textGC, x, 30+ascent, buf, strlen(buf) );

    for( i = 0; i < 3; i++ ){
	conv_time( record[select_course-1][i], buf_t );
	sprintf(form, "%%d    %%s  %%-%ds", NAME_MAX );
	sprintf(n_buf[i], form, i+1, buf_t, name[select_course-1][i] );
	XTextExtents( fn, n_buf[i], strlen(n_buf[i]),
		                 &dir, &ascent_i, &descent, &overall );
	s_width = overall.rbearing - overall.lbearing;
	rec_x[i] = (WWIDTH - s_width) / 2 - overall.lbearing;
    }
    min_x = rec_x[0];
    if( rec_x[1] < min_x )
	min_x = rec_x[1];
    if( rec_x[2] < min_x )
	min_x = rec_x[2];
    for( i = 0; i < 3; i++ )
	XDrawString( dp, win, textGC, min_x,
		         65+ascent+i*30, n_buf[i], strlen(n_buf[i]) );

    if( high > 0 )
	XCopyArea( dp, point_ball, win, copyGC, 0, 0, 14, 14,
		               min_x-20+(overall.lbearing), 68+(high-1)*30 );
    conv_time( res_time, buf_t );
    sprintf(buf, "Results :  %s",buf_t);
    XTextExtents( fn, buf, strlen(buf), &dir, &ascent_i, &descent, &overall );
    s_width = overall.rbearing - overall.lbearing;
    x = (WWIDTH - s_width) / 2 - overall.lbearing;
    if( high > 0 ){
	XSetForeground( dp, textGC, b3pixel );
	XDrawString( dp, win, textGC, x, 180, buf, strlen(buf) );
	XSetForeground( dp, textGC, white_pixel );
    }else if( over_time > (res_time/6) ){
	XSetForeground( dp, textGC, otpixel );
	XDrawString( dp, win, textGC, x, 180, buf, strlen(buf) );
	XSetForeground( dp, textGC, white_pixel );
    }else
	XDrawString( dp, win, textGC, x, 180, buf, strlen(buf) );
	
    XFlush( dp );
}

void conv_time( tm, buf )
int tm;
char *buf;
{
    int min, sec, msec;

    msec = tm % 100;
    sec = tm / 100;
    min = sec / 60;
    sec = sec % 60;

    sprintf(buf, "%01d\'%02d\"%02d", min, sec, msec);
}

/*
    course select
*/

int course_select()
{
    int sc;

    set_cs_state();
    draw_cst();
    sc = cs_ev();
    cs_num = sc - 1;
    set_cs_state();
    draw_cst_sub();

    if( sc != 1 )
	XFillRectangle( dp, win, blackGC, cs_x, 45, cs_width, 23 );
    if( sc != 2 )
	XFillRectangle( dp, win, blackGC, cs_x, 70, cs_width, 23 );
    if( sc != 3 )
	XFillRectangle( dp, win, blackGC, cs_x, 95, cs_width, 23 );
    if( sc != 4 )
	XFillRectangle( dp, win, blackGC, cs_x, 120, cs_width, 23 );
    if( sc != 5 )
	XFillRectangle( dp, win, blackGC, cs_x, 145, cs_width, 23 );
    if( sc != 6 )
	XFillRectangle( dp, win, blackGC, cs_x, 170, cs_width, 23 );
    if( sc != 7 )
	XFillRectangle( dp, win, blackGC, cs_x, 195, cs_width, 23 );
    XFlush(dp);

    return sc;
}

int cs_ev()
{
    int a;
    while(1){
	XNextEvent( dp, &ev );
	if( ev.type == Expose )
            draw_cst();
        if( ev.type == KeyPress )
            if( (a = keycheck3(ev.xkey)) )
                return a;
    }
}

int keycheck3(ev)
XKeyEvent ev;
{
    char ch;
    KeySym ksym;

    XLookupString( &ev, &ch, 1, &ksym, NULL );
    switch(ksym){
    case XK_Up:
	cs_num--;
	cs_num = (cs_num + COURSE) % COURSE;
	set_cs_state();
	draw_cst_sub();
	return 0;
    case XK_Down:
	cs_num++;
	cs_num %= COURSE;
	set_cs_state();
	draw_cst_sub();
	return 0;
    case XK_space:
	return (cs_num+1);
    case XK_1:
	return 1;
    case XK_2:
	return 2;
    case XK_3:
	return 3;
    case XK_4:
	return 4;
    case XK_5:
	return 5;
    case XK_6:
	return 6;
    case XK_7:
	return 7;
    case XK_q:
	end_prog();
    }
    return 0;
}

void set_cs_state()
{
    int i;

    for( i = 0; i < COURSE; i++ )
	cs_state[i] = 0;
    cs_state[cs_num] = 1;
}

void draw_cst()
{
    XCharStruct overall;
    int dir;
    int ascent_i;
    int descent;
    int s_width;
    int x;

    char buf[20];

    XClearWindow( dp, win );
    sprintf(buf, "COURSE SELECT");
    XTextExtents( fn_cs, buf, strlen(buf),
		         &dir, &ascent_i, &descent, &overall );
    s_width = overall.rbearing - overall.lbearing;
    x = (WWIDTH - s_width) / 2 - overall.lbearing;
    XDrawString( dp, win, text_csGC[0], x, 15+ascent_cs, buf, strlen(buf) );

    sprintf(buf, "5.    Professional");
    XTextExtents( fn_cs, buf, strlen(buf),
		         &dir, &ascent_i, &descent, &overall );
    s_width = overall.rbearing - overall.lbearing;
    cs_text_x = (WWIDTH - s_width) / 2 - overall.lbearing;
    cs_width = s_width;
    cs_x = cs_text_x + overall.lbearing;

    draw_cst_sub();
}

void draw_cst_sub()
{
    char buf[20];

    XFillRectangle( dp, win, blackGC, cs_x, 45, cs_width, 175 );
    sprintf(buf, "1.    Novice");
    XDrawString( dp, win, text_csGC[cs_state[0]],
		                 cs_text_x, 45+ascent_cs, buf, strlen(buf) );
    sprintf(buf, "2.    Average");
    XDrawString( dp, win, text_csGC[cs_state[1]],
		                 cs_text_x, 70+ascent_cs, buf, strlen(buf) );
    sprintf(buf, "3.    Master");
    XDrawString( dp, win, text_csGC[cs_state[2]],
		                 cs_text_x, 95+ascent_cs, buf, strlen(buf) );
    sprintf(buf, "4.    Expert");
    XDrawString( dp, win, text_csGC[cs_state[3]],
		                 cs_text_x, 120+ascent_cs, buf, strlen(buf) );
    sprintf(buf, "5.    Professional");
    XDrawString( dp, win, text_csGC[cs_state[4]],
		                 cs_text_x, 145+ascent_cs, buf, strlen(buf) );
    sprintf(buf, "6.    Special  1");
    XDrawString( dp, win, text_csGC[cs_state[5]],
		                 cs_text_x, 170+ascent_cs, buf, strlen(buf) );
    sprintf(buf, "7.    Special  2");
    XDrawString( dp, win, text_csGC[cs_state[6]],
		                 cs_text_x, 195+ascent_cs, buf, strlen(buf) );
    XFlush( dp );
}


void create_course()
{
    int x, y, pix;
    struct course_d *cd;


    p_course = XCreatePixmap( dp, root, MWIDTH, MHEIGHT,
			                        DefaultDepth( dp, sc ) );
    XFillRectangle( dp, p_course, clearGC, 0, 0, MWIDTH, MHEIGHT );
    init_course();

    if( select_course != 7 ){
	cd = cs_d[select_course-1];
	while(1){
	    x = cd->x;
	    y = cd->y;
	    pix = (cd++)->pix;
	    if( pix == '@' )
		break;
	    if( pix == 20 )
		XCopyArea( dp, p_wall[20], p_course, copyGC, 0, 0,
			   wall_width[20], wall_height[20], x * 5, y * 5 );
	    else
		XCopyArea( dp, p_wall[pix], p_course, copyGC, 0, 0,
			   wall_width[pix], wall_height[pix], x * 25, y * 25 );
	}
    }else
	c7_make();

    bimage = XGetImage( dp, p_course, 0, 0,
			              MWIDTH, MHEIGHT, AllPlanes, ZPixmap );
    bw = MWIDTH; bh = MHEIGHT;
    bdata = bimage->data;
}

void init_course()
{
    int a;

    a = select_course-1;
    if( select_course != 7 )
	XFillRectangle( dp, p_course, goalGC, gx[a], gy[a], gw[a], gh[a] );
    else{
	XFillRectangle( dp, p_course, goalGC, g7x[0], g7y[0], g7w[0], g7h[0] );
	XFillRectangle( dp, p_course, goalGC, g7x[1], g7y[1], g7w[1], g7h[1] );
    }
}

/* course 7 */
void c7_make()
{
    int w[29][29], w2[29][29], *p;
    int dx[] = {0,1,0,-1}, dy[] = {-1,0,1,0};
    int x, y;
    int a, i;

    for( x = 0; x < 29; x++ )
	for( y = 0; y < 29; y++ )
	    w[x][y] = 0;
    for( x = 2; x < 28; x += 2 )
	for( y = 2; y < 28; y += 2 ){
	  l1:
	    if( x == 2 ) 
		a = (rand() & 0x30) >> 4;
	    else
		a = rand() % 3;
	    p = &w[x+dx[a]][y+dy[a]];
	    if( *p ){
                if( (rand() & 0xff) > 0x80 )
		    goto l1;
	    } else
		*p = 1;
	}
    a = rand() & 0x3;
    switch( a ){
    case 0:
	for( x = 0; x < 29; x++ )
	    for( y = 0; y < 29; y++ )
		w2[x][y] = w[x][y];
	break;
    case 1:
	for( x = 0; x < 29; x++ )
	    for( y = 0; y < 29; y++ )
		w2[28-y][x] = w[x][y];
	break;
    case 2:
	for( x = 0; x < 29; x++ )
	    for( y = 0; y < 29; y++ )
		w2[28-x][28-y] = w[x][y];
	break;
    case 3:
	for( x = 0; x < 29; x++ )
	    for( y = 0; y < 29; y++ )
		w2[y][28-x] = w[x][y];
    }
    for( x = 0; x < 29; x++ )
	for( y = 0; y < 29; y++ )
	    if( w2[x][y] )
		if( y & 1 )
		    draw_c7(x,y,1);
		else
		    draw_c7(x,y,0);

    for( i = 0; i < 950; i += 50 ){
	XCopyArea( dp, p_wall[2], p_course, copyGC, 0, 0,
		                  wall_width[2], wall_height[2], i, 995 );
	XCopyArea( dp, p_wall[3], p_course, copyGC, 0, 0,
		                  wall_width[3], wall_height[3], 0, i );
    }
    XCopyArea( dp, p_wall[2], p_course, copyGC, 0, 0,
		              wall_width[2]-5, wall_height[2], 950, 995 );
    XCopyArea( dp, p_wall[3], p_course, copyGC, 0, 0,
		              wall_width[3], wall_height[3]-5, 0, 950 );
    for( i = 970; i > 25; i -= 50 ){
	XCopyArea( dp, p_wall[2], p_course, copyGC, 0, 0,
		                  wall_width[2], wall_height[2], i, 0 );
	XCopyArea( dp, p_wall[3], p_course, copyGC, 0, 0,
		                  wall_width[3], wall_height[3], 995, i );
    }
    XCopyArea( dp, p_wall[2], p_course, copyGC, 5, 0,
	                      wall_width[2]-5, wall_height[2], 25, 0 );
    XCopyArea( dp, p_wall[3], p_course, copyGC, 0, 5,
		              wall_width[3], wall_height[3]-5, 995, 25 );

}

void draw_c7( x, y, ty )
int x, y, ty;
{
    int i;

    if( ty )
	for( i = 0; i < 8; i++ )
	    XCopyArea( dp, p_wall[20], p_course, copyGC,
		                 0, 0, 10, 10, x*35+15, (y-1)*35+i*10+15);
    else
	for( i = 0; i < 8; i++ )
	    XCopyArea( dp, p_wall[20], p_course, copyGC,
		                 0, 0, 10, 10, (x-1)*35+i*10+15, y*35+15);
}

void set_background(scn)
int scn;
{
    int bx, by;
    int y;
    int p_status;
    int wid;

  retry:
    xpatt.valuemask = XpmReturnPixels | XpmColormap;
    xpatt.colormap = cmap;
    if( (p_status = XpmCreatePixmapFromData(
	dp, win, bg_data[scn], &back_pix, NULL, &xpatt )) != XpmSuccess ){
	if( owncmap == 0 ){
	    change_cmap();
	    goto retry;
	}
	p_err(XpmGetErrorString(p_status));
    }
    bg_col = xpatt.pixels;
    bg_num = xpatt.npixels;
    XFillRectangle(dp, back_pix,
		   ballGC[0][ball_type], ball_x_bg, ball_y_bg, ball_w, ball_h);
    back_image = XGetImage( dp, back_pix, 0, 0, 200, 200, AllPlanes, ZPixmap );
    wid = RWIDTH;
    if( wid > 200 )
	wid = 200;
    back_data = (int *)malloc(wid * vheight * 4);
    bx = (200-wid)/2;
    by = (200-vheight)/2;
    for( y = 0; y < vheight; y++ )
	memcpy( back_data+y*wid, (back_image->data)+(bx+(by+y)*200)*4, wid*4 );
}

void free_xres()
{
    XFreePixmap( dp, p_course );
    XDestroyImage( bimage );

    XFreeColors( dp, cmap, bg_col, bg_num, 0 );
    XDestroyImage( back_image );
    XFreePixmap( dp, back_pix );
    free( back_data );
    XFlush(dp);
}

void draw_win()
{
    XClearWindow( dp, win );
    XCopyArea( dp, p_frame, win, copyGC, 0, 0, 225, 225, 0, 0 );
    XCopyArea( dp, p_titlem, win, copyGC, 0, 0, 70, 35,
	       (WWIDTH-225-75)/2+225, 10 );
}

void draw_mesg()
{
    XFillRectangle( dp, win, blackGC, (WWIDTH-225-70)/2+225, 150, 70, 60 );
    if( num_state < 3 )
	XCopyArea( dp, num[num_state], win, copyGC, 0, 0, 60, 60,
		   (WWIDTH-225-60)/2+225, 150 );
    else if( num_state < 5 )
	XCopyArea( dp, num[num_state], win, copyGC, 0, 0, 70, 60,
		   (WWIDTH-225-70)/2+225, 150 );

    XFlush(dp);
}

int pre_ev()
{
    while( XEventsQueued( dp, QueuedAfterReading ) ){
	XNextEvent( dp, &ev );
	switch( ev.type ){
	case Expose:
	    draw_win();
	    screen();
	    break;
	case KeyPress:
	case KeyRelease:
	    key_check( ev.xkey );
	    if( escape == 1 )
		return 1;
	    restart = 0;
	}
    }
    return 0;
}

void pre_event()
{
    while( XCheckWindowEvent( dp, win, KeyPressMask, &ev ) )
	;
}

void goal_event( h, t )
int h, t;
{
    while(1){
	XNextEvent( dp, &ev );
	if( ev.type == Expose )
	    draw_result( h, t );
	if( ev.type == KeyPress )
	    if( keycheck2(ev.xkey) )
		return;
    }
}

int keycheck2(ev)
XKeyEvent ev;
{
    char ch;
    KeySym ksym;

    XLookupString( &ev, &ch, 1, &ksym, NULL );
    if( ksym == XK_space )
        return 1;
    if( ksym == XK_q ){
        end_prog();
    }
    return 0;
}

void check_ev()
{
    while( XEventsQueued( dp, QueuedAfterReading ) ){
	XNextEvent( dp, &ev );
	switch( ev.type ){
	case KeyPress:
	case KeyRelease:
	    key_check( ev.xkey );
	    break;
	case Expose:
	    draw_win();
	    draw_mesg();
	}
    }
}

void key_check( ev )
XKeyEvent ev;
{
    register int set=0;
    KeySym ksym;
    char ch;

    if( ev.type == KeyPress )
	set = 1;
    
    XLookupString( &ev, &ch, 1, &ksym, NULL );
    switch( ksym ){
    case XK_Left:
	key_left = set;
	break;
    case XK_Right:
	key_right = set;
	break;
    case XK_space:
	key_space = set;
	break;
    case XK_Up:
	jump_key = set;
	break;
    case XK_Down:
	soft_key = set * 9;
	break;
    case XK_Return:
	if( ev.type == KeyPress )
	    restart = 1;
	break;
    case XK_Escape:
	if( ev.type == KeyPress )
	    escape = 1;
	break;
    case XK_q:
	end_prog();
    }
}

void end_prog()
{
    if( dp ){
#ifdef MITSHM
	if( shm ){
	    if( shminfo[0].shmid > 0 )
		XShmDetach( dp, &(shminfo[0]) );
	    if( shminfo[1].shmid > 0 )
		XShmDetach( dp, &(shminfo[1]) );
	}
#endif
	XAutoRepeatOn( dp );
	XCloseDisplay( dp );
    }
#ifdef MITSHM
    if( shm ){
	if( image[0] )
	    shmdt( image[0] );
	if( image[1] )
	    shmdt( image[1] );
	if( shminfo[0].shmid >= 0 )
	    shmctl( shminfo[0].shmid, IPC_RMID, 0 );
	if( shminfo[1].shmid >= 0 )
	    shmctl( shminfo[1].shmid, IPC_RMID, 0 );
    }
#endif
    exit(0);
}

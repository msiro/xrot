/*======================================================================
    xrot
========================================================================*/

/* window size */
#define WWIDTH  300
#define WHEIGHT 225

extern Display *dp;
extern Window win;
extern GC copyGC, titleGC, keysGC, blackGC;
extern XImage *ximage[2];
extern char *image[2];
extern int dbl_buf;
extern unsigned long black_pixel;
extern Colormap cmap;
extern int owncmap;

extern XFontStruct *fn2;
extern int ascent2;
extern unsigned long tipix[4];
extern unsigned short grad[64];

extern GC ballGC[2][2];
extern int ball_x, ball_y, ball_w, ball_h;

extern int view_x, view_y;

#define COURSE 7

#ifdef MITSHM
extern int shm;
#endif

/* key state */
extern int key_right, key_left;
extern int key_space, jump_key, soft_key;
extern int escape;
extern int restart;

extern int num_state;
extern int g_pixel;

/* course data */

extern int lx[COURSE];
extern int ly[COURSE];
extern int sdeg[COURSE];
extern int c6_x[3];
extern int c6_y[3];
extern int c6_deg[3];

/* function */
void init_X();
void check_ev();
void title();
void set_background();
void free_xres();
void draw_win();
void create_course();
void draw_mesg();
int pre_ev();
void clear_screen();
void draw_result();
void goal_event();
int course_select();
void pre_event();
void change_cmap();
void conv_time();

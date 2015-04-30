/*======================================================================
    xrot
========================================================================*/

/* MAX view size */

#define VWIDTH  200
#define VHEIGHT 200

/* MAP size */

#define MWIDTH 1024
#define MHEIGHT 1024
#define MP 10 /* log2(MWIDTH) */

#define MMASK 0xfffffc00   /* for 32 bits */

/* speed */
#define MSPF 30 /* msec per frame */

/* view window size */
extern int vwidth, vheight;
extern int RWIDTH, RHEIGHT;

/* misc. */
extern unsigned int bw, bh;
extern char *bdata;
extern int *back_data;
extern int select_course;
extern unsigned int record[7][3];
extern int ball_type;

/* internal valiable */
extern int loc_x, loc_y;

/* accelerate */
#define AC 64
#define THR 256
#define TP 8 /* log2(THR) */
#define RT 576

/* ball hankei */
#define BALL 8

/* divide degree */
#define DEG 64

extern int tco[DEG];
extern int tsi[DEG];

/* course */

struct course_d {
    int x;
    int y;
    int pix;
};

/* for integer */
#define R 65536
#define P 16    /* log2(R) */

extern struct timezone tzone;

/* wait */

extern int wait_f;
void set_itimer();
void off_itimer();
void alarm_receive();

extern int over_time;

/* function */
void screen();
void check_speed();
void p_error();
void wait_sc();
void end_prog();
void u_sleep_sel();
void u_sleep();

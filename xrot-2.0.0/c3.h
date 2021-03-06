/*======================================================================
    xrot
 =======================================================================*/

int c3_x = 125;
int c3_y = 100;
int c3_deg = 8;
int g3x = 200;
int g3y = 300;
int g3w = 15;
int g3h = 100;

struct course_d c3[] =
{
{0,22,9},
{0,24,9},
{0,26,9},
{0,28,9},
{0,30,9},
{0,32,9},
{0,34,9},
{0,36,9},
{0,38,12},
{0,7,10},
{0,9,10},
{1,1,1},
{1,11,14},
{1,13,3},
{1,22,4},
{1,3,1},
{1,5,1},
{10,1,2},
{10,15,2},
{10,22,3},
{10,38,6},
{10,6,2},
{11,11,6},
{11,12,11},
{11,24,3},
{11,32,15},
{11,34,6},
{11,9,11},
{12,1,2},
{12,15,2},
{12,18,6},
{12,19,6},
{12,2,0},
{12,20,3},
{12,22,3},
{12,24,3},
{12,26,3},
{12,28,3},
{12,30,3},
{12,38,6},
{12,6,2},
{13,11,6},
{13,27,2},
{13,34,6},
{14,1,2},
{14,12,11},
{14,16,3},
{14,18,6},
{14,19,6},
{14,2,0},
{14,21,1},
{14,23,1},
{14,25,0},
{14,38,6},
{14,6,2},
{15,11,6},
{15,21,0},
{15,27,5},
{15,29,5},
{15,31,5},
{15,33,5},
{16,1,2},
{16,12,11},
{16,2,0},
{16,25,0},
{16,27,4},
{16,38,6},
{16,6,2},
{17,11,6},
{17,14,17},
{17,16,17},
{17,18,17},
{17,20,17},
{18,1,2},
{18,18,9},
{18,2,0},
{18,20,9},
{18,22,9},
{18,24,9},
{18,27,4},
{18,38,6},
{18,6,2},
{19,11,6},
{19,18,8},
{19,30,5},
{19,32,5},
{19,34,17},
{19,36,17},
{2,1,19},
{2,15,3},
{2,2,12},
{2,27,1},
{2,38,12},
{2,4,12},
{2,6,19},
{20,1,2},
{20,27,4},
{20,38,6},
{20,6,2},
{21,11,19},
{21,12,13},
{21,14,13},
{21,16,13},
{21,18,8},
{21,25,11},
{22,1,2},
{22,27,4},
{22,38,12},
{22,6,2},
{23,11,19},
{23,18,8},
{23,21,14},
{23,23,14},
{23,25,5},
{23,28,3},
{23,30,3},
{23,32,3},
{23,34,3},
{24,1,2},
{24,21,6},
{24,27,4},
{24,38,12},
{24,6,2},
{25,11,19},
{25,18,8},
{25,5,0},
{26,1,2},
{26,21,6},
{26,27,4},
{26,30,9},
{26,32,9},
{26,34,9},
{26,36,9},
{26,38,12},
{26,6,2},
{27,14,0},
{27,18,8},
{27,5,0},
{28,1,2},
{28,21,6},
{28,27,4},
{28,38,12},
{28,6,2},
{29,14,0},
{29,18,8},
{29,28,1},
{29,30,1},
{29,32,1},
{29,34,1},
{29,5,0},
{3,17,3},
{3,22,4},
{3,26,1},
{3,28,1},
{3,30,7},
{3,32,7},
{3,34,6},
{30,1,2},
{30,21,6},
{30,35,2},
{30,38,12},
{30,6,2},
{31,14,0},
{31,18,8},
{31,5,0},
{32,1,2},
{32,10,5},
{32,12,5},
{32,21,3},
{32,23,3},
{32,25,3},
{32,27,3},
{32,29,3},
{32,31,3},
{32,33,3},
{32,35,2},
{32,38,12},
{32,6,5},
{32,8,5},
{33,18,18},
{33,21,3},
{33,23,3},
{33,25,3},
{33,29,3},
{33,31,3},
{33,33,3},
{34,1,2},
{34,21,3},
{34,23,3},
{34,31,3},
{34,33,3},
{34,35,2},
{34,38,6},
{35,18,18},
{35,33,3},
{36,1,2},
{36,10,14},
{36,12,7},
{36,14,7},
{36,16,7},
{36,2,10},
{36,38,6},
{36,4,10},
{36,6,10},
{36,8,14},
{37,18,3},
{37,20,3},
{38,18,1},
{38,20,1},
{38,22,1},
{38,24,1},
{38,26,1},
{38,28,1},
{38,30,1},
{38,32,1},
{38,34,13},
{38,36,13},
{38,38,6},
{4,1,19},
{4,19,3},
{4,29,1},
{4,31,1},
{4,38,12},
{4,6,19},
{5,11,6},
{5,12,14},
{5,21,3},
{5,32,15},
{5,34,6},
{6,1,19},
{6,14,3},
{6,23,3},
{6,38,6},
{6,6,19},
{7,11,6},
{7,12,1},
{7,14,1},
{7,16,3},
{7,25,3},
{7,32,15},
{7,34,6},
{8,1,2},
{8,15,2},
{8,18,3},
{8,27,3},
{8,38,6},
{8,6,2},
{9,11,6},
{9,20,3},
{9,32,15},
{9,34,6},
{9,9,11},
{'@','@','@'}
};

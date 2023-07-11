#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <stdbool.h>
#include <math.h>
#include <graphics.h>
#include <time.h>
#include <conio.h>
#include <set>

extern "C" {
	#include "aoi.h"
}
#pragma warning(disable:4996)

#define RECT_X_W 800
#define RECT_Y_H 800
#define MAX_SIZE 500

struct alloc_cookie {
	int count;
	int max;
	int current;
};

static int select_idx = 1;
static std::set<int> select_insight;

static void *
my_alloc(void *ud,void *ptr,size_t sz) {
	struct alloc_cookie *cookie = (struct alloc_cookie* )ud;
	if (ptr == NULL) {
		// alloc
		void *p = malloc(sz);
		++cookie->count;
		cookie->current += sz;
		if (cookie->current > cookie->max) {
			cookie->max = cookie->current;
		}
		//printf("%p + %lu\n",p,sz);
		return p;
	}
	--cookie->count;
	cookie->current -= sz;
	free(ptr);
	//printf("%p - %lu\n",ptr,sz);
	return NULL;
}

typedef struct OBJECT {
	float pos[3];
	float v[3];
	char mode[4];
} OBJECT;

static struct OBJECT OBJ[MAX_SIZE];
static bool check_leave_aoi = true;
static float map_size[3] = { RECT_X_W,RECT_Y_H,100};
static float view_size[3] = {100,100,100};
// 2d
//static float map_size[3] = {100,100,0};
//static float view_size[3] = {4.5,4.5,0};


static void
init_obj(uint32_t id,float x,float y,float z,float vx,float vy,float vz,const char *mode) {
	OBJ[id].pos[0] = x;
	OBJ[id].pos[1] = y;
	OBJ[id].pos[2] = z;

	OBJ[id].v[0] = vx;
	OBJ[id].v[1] = vy;
	OBJ[id].v[2] = vz;
	strcpy(OBJ[id].mode,mode);
}

static void
update_obj(struct aoi_space *aoi,uint32_t id) {
	int i;
	for (i=0; i<3; i++) {
		OBJ[id].pos[i] += OBJ[id].v[i];
		if (OBJ[id].pos[i] > map_size[i] - view_size[i]) {
			OBJ[id].v[i] *= -1;
			//OBJ[id].pos[i] -= map_size[i]/2;
		} else if (OBJ[id].pos[i] < 0) {
			OBJ[id].v[i] *= -1;
			//OBJ[id].pos[i] += map_size[i]/2;
		}
	}
	aoi_move(aoi,id,OBJ[id].pos);
}
/*
static void
update_obj(struct aoi_space* space, uint32_t id) {
	int i;
	for (i = 0; i < 3; i++) {
		OBJ[id].pos[i] += OBJ[id].v[i];
		if (OBJ[id].pos[i] < 0) {
			//OBJ[id].pos[i]+=3.0f;
			OBJ[id].v[i] *= -1;
		}
		else if (OBJ[id].pos[i] > 700.0f) {
			OBJ[id].v[i] *= -1;
		}
	}
	aoi_update(space, id, OBJ[id].mode, OBJ[id].pos);
	aoi_message(space, message, NULL);
}*/

static bool
in_view(float pos1[3],float pos2[3]) {
	int i;
	for (i=0; i<3; i++) {
		if (fabs(pos1[i]-pos2[i]) > view_size[i]) {
			return false;
		}
	}
	return true;
}

static void
enterAOI(void *ud,uint32_t watcher,uint32_t marker) {
	if (watcher != select_idx) {
		return;
	}
	select_insight.insert(marker);
	printf("op=enterAOI,watcher=[id=%d,pos=(%.1f,%.1f,%.1f)],marker=[id=%d,pos=(%.1f,%.1f,%.1f)]\n",
			watcher,OBJ[watcher].pos[0],OBJ[watcher].pos[1],OBJ[watcher].pos[2],
			marker,OBJ[marker].pos[0],OBJ[marker].pos[1],OBJ[marker].pos[2]);
	//assert(in_view(OBJ[watcher].pos,OBJ[marker].pos));
}

static void
leaveAOI(void *ud,uint32_t watcher,uint32_t marker) {
	if (watcher != select_idx) {
		return;
	}
	if (select_insight.count(marker)) {
		select_insight.erase(select_insight.find(marker));
	}
	printf("op=leaveAOI,watcher=[id=%d,pos=(%.1f,%.1f,%.1f)],marker=[id=%d,pos=(%.1f,%.1f,%.1f)]\n",
			watcher,OBJ[watcher].pos[0],OBJ[watcher].pos[1],OBJ[watcher].pos[2],
			marker,OBJ[marker].pos[0],OBJ[marker].pos[1],OBJ[marker].pos[2]);
	if (check_leave_aoi) {
		// True if event not triggered by aoi_leave
		//assert(!in_view(OBJ[watcher].pos,OBJ[marker].pos));
	}
}

static void
doinit(struct aoi_space* space) {
	srand((unsigned)time(NULL));
	float vec[14][2] = {
		{0,1},
		{0,-1},
		{1,-1},
		{1,0},
		{1,1},
		{-1,0},
		{-1,1},
		{-1,-1},
		{1.75,1.75},
		{1.75,-1.75},
		{-1.75,1.75},
		{-1.75,-1.75},
		{-1.75,0},
		{1.75,0},
	};
	for (int i = 0; i < MAX_SIZE; i++) {
		float x = rand() % ( RECT_X_W- int(view_size[0]) );
		float y = rand() % ( RECT_Y_H - int(view_size[1]) );
		if (i % 2) {
			init_obj(i, x, y, 0, vec[i % 14][0], vec[i % 14][1], 0, "wm");
		}
		else {
			init_obj(i, x, y, 0, vec[i % 14][0], vec[i % 14][1], 0, "wm");
		}
	}
	int i = 0;
	for (i = 0; i < MAX_SIZE; i++) {
		aoi_enter(space, i, OBJ[i].pos, OBJ[i].mode);
	}
}

static void
test(struct aoi_space *aoi) {
	int i,j;
	check_leave_aoi = true;
	// w(atcher) m(arker)
	init_obj(0,40,0,0,0,2,0,"wm");
	init_obj(1,42,100,0,0,-2,0,"wm");
	init_obj(2,0,40,0,2,0,0,"w");
	init_obj(3,100,42,0,-2,0,0,"w");
	init_obj(4,42,40,1,0,0,2,"wm");
	init_obj(5,40,42,100,0,0,-2,"w");
	init_obj(6,40,42,100,0,0,-2,"m");
	for(i=0; i<7; i++) {
		aoi_enter(aoi,i,OBJ[i].pos,OBJ[i].mode);
	}
	for(i=0; i<100; i++) {
		if (i < 50) {
			for(j=0; j<7; j++) {
				update_obj(aoi,j);
			}
		} else if (i == 50) {
			strcpy(OBJ[6].mode,"wm");
			aoi_change_mode(aoi,6,OBJ[6].mode);
		} else {
			for(j=0; j<7; j++) {
				update_obj(aoi,j);
			}
		}
	}
	int number = 0;
	float range[3] = {4,4,0};
	float pos[3] = {40,4,0};
	void **ids = aoi_get_view_by_pos(aoi,pos,range,&number);
	//ids = aoi_get_view_by_pos(aoi,pos,NULL,&number);
	if (ids != NULL && number != 0) {
		printf("op=get_view_by_pos,pos=(%.1f,%.1f,%.1f),range=(%.1f,%.1f,%.1f),ids=",
				pos[0],pos[1],pos[2],range[0],range[1],range[2]);
		for(i=0; i<number; i++) {
			if (i == number -1) {
				printf("%u",(uint32_t)ids[i]);
			} else {
				printf("%u,",(uint32_t)ids[i]);
			}
		}
		printf("\n");
	}
	uint32_t id = 5;
	ids = aoi_get_view(aoi,id,range,&number);
	//ids = aoi_get_view(aoi,id,NULL,&number);
	if (ids != NULL && number != 0) {
		printf("op=get_view,id=%u,pos=(%.1f,%.1f,%.1f),range=(%.1f,%.1f,%.1f),ids=",
				id,OBJ[id].pos[0],OBJ[id].pos[1],OBJ[id].pos[2],range[0],range[1],range[2]);
		for(i=0; i<number; i++) {
			if (i == number -1) {
				printf("%u",(uint32_t)ids[i]);
			} else {
				printf("%u,",(uint32_t)ids[i]);
			}
		}
		printf("\n");
	}
	check_leave_aoi = false;
	for(i=0; i<7; i++) {
		aoi_leave(aoi,i);
	}
}

static void showOne( int i, int posX, int posY) {
	circle(posX, int(posY), 2);

	switch (i % 4) {
	case 0:
		setfillcolor(RGB(204, 116, 51));
		break;
	case 1:
		setfillcolor(RGB(92, 205, 50));
		break;
	case 2:
		setfillcolor(RGB(48, 207, 203));
		break;
	case 3:
		setfillcolor(RGB(183, 49, 206));
		break;
	}
	if (i == select_idx) {
		//draw view size
		rectangle(posX - view_size[0], posY - view_size[1], posX + view_size[0], posY + view_size[1]);
		//fillcircle(int(OBJ[i].pos[0]), int(OBJ[i].pos[1]), 2);
		//circle(int(OBJ[i].pos[0]), int(OBJ[i].pos[1]), view_size[0]);
	}
	WCHAR info[16];
	_itot(i, info, 10);
	setbkmode(OPAQUE);
	setbkmode(TRANSPARENT);
	settextcolor(RGB(0, 0, 0));
	LOGFONT ff;
	gettextstyle(&ff);
	ff.lfHeight = 4;
	ff.lfWidth = 4;
	ff.lfQuality = ANTIALIASED_QUALITY;
	outtextxy(int(posX + view_size[0] / 8), int(posY - 8), info);
}
static void drawMonitorMapView() {
	clearrectangle(800, 0, 1200, 8 + view_size[1] * 2);
	roundrect(810, 3, 810 + view_size[0]*2, 3 + view_size[1]*2, 1, 1);
	int posCenterX = 810 + view_size[0];
	int posCenterY = view_size[0] + 3;
	//circle(posCenterX, posCenterY, view_size[0]);
	circle(posCenterX, posCenterY, 1);
	//draw all inside;
	for (auto iter = select_insight.begin(); iter != select_insight.end(); iter++) {
		int sidx = *iter;
		int diff_x = OBJ[sidx].pos[0] - OBJ[select_idx].pos[0];
		int diff_y = OBJ[sidx].pos[1] - OBJ[select_idx].pos[1];
		int relate_x = posCenterX + diff_x;
		int relate_y = posCenterY + diff_y;
		showOne(sidx, relate_x, relate_y);
	}
}

static void drawMonitorText() {
	clearrectangle(810, 700, 1200, 800);
	roundrect(810, 700, 1200, 800, 1, 1);
	//1200, 800
	WCHAR info[16];
	wsprintf(info, _T("SELECTED idx:%d"), select_idx);
	setbkmode(OPAQUE);
	setbkmode(TRANSPARENT);
	settextcolor(RGB(0, 0, 0));
	LOGFONT ff;
	gettextstyle(&ff);
	ff.lfHeight = 15;
	ff.lfWidth = 15;
	ff.lfQuality = ANTIALIASED_QUALITY;
	outtextxy(810, 700, info);
}


static void showObjs(struct aoi_space* space) {

	int i = 0;
	for (i = 0; i < MAX_SIZE; i++) {
		showOne(i, OBJ[i].pos[0], OBJ[i].pos[1]);
	}
	FlushBatchDraw();
}
void dumpall() {
	int i = 0;
	printf("---------------------------------\r\n");
	for (int i = 0; i < MAX_SIZE; i++) {
		printf("%d pos{%f,%f}\r\n", i, OBJ[i].pos[0], OBJ[i].pos[1]);
	}
	printf("---------------------------------\r\n");
}

void manualMain(struct aoi_space* aoi) {
	BeginBatchDraw();
	int id = 1;
	drawMonitorText();
	while (true) {
		for (int i = 0; i < MAX_SIZE; i++) {
			if (i == select_idx) {
				continue;
			}
			update_obj(aoi, i);
		}
		//cleardevice();
		clearroundrect(0, 0, RECT_X_W, RECT_Y_H, 1, 1);
		roundrect(0, 0, RECT_X_W, RECT_Y_H, 1, 1);
		
		showObjs(aoi);
		drawMonitorMapView();
		Sleep(17);

		if (_kbhit()) {
			
			char key = _getch();

			if (key == 75) {
				OBJ[select_idx].pos[0] += -10;
			}
			else if (key == 77) {
				OBJ[select_idx].pos[0] += 10;
			}
			else if (key == 72) {
				OBJ[select_idx].pos[1] -= 10;
			}
			else if (key == 80) {
				OBJ[select_idx].pos[1] += 10;
			}
			else if (key == 115) {
				select_idx = select_idx++ % MAX_SIZE;
				select_insight.clear();
				drawMonitorText();
			}
			//adajust pos
			if (OBJ[select_idx].pos[0] < 0) {
				OBJ[select_idx].pos[0] = 0;
			}
			if (OBJ[select_idx].pos[1] < 0) {
				OBJ[select_idx].pos[1] = 0;
			}
			if (OBJ[select_idx].pos[0] > RECT_X_W- view_size[0]) {
				OBJ[select_idx].pos[0] = RECT_X_W -view_size[0];
			}
			if (OBJ[select_idx].pos[1] > RECT_Y_H - view_size[1]) {
				OBJ[select_idx].pos[1] = RECT_Y_H - view_size[1];
			}
			//printf("%d was pressed (%f,%f)\r\n", key,OBJ[id].pos[0], key, OBJ[id].pos[1]);
			//dumpall();
			aoi_move(aoi, select_idx, OBJ[select_idx].pos);
		}
	}

}

int
main() {
	struct alloc_cookie cookie = {0,0,0};
	initgraph(1200, 800, SHOWCONSOLE);
	setbkcolor(RGB(128, 128, 128));
	cleardevice();
	//void roundrect(int left, int top, int right, int bottom, int ellipsewidth, int ellipseheight);		// Draw a rounded rectangle without filling

	struct aoi_space *aoi = aoi_create(my_alloc,&cookie,map_size,view_size,enterAOI,leaveAOI,NULL);
	doinit(aoi);
	
	manualMain(aoi);

	//test(aoi);
	aoi_release(aoi);
	printf("max memory = %d,current memory = %d\n",cookie.max,cookie.current);
	return 0;
}
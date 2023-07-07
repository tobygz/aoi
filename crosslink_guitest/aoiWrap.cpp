#include "aoiWrap.h"
#include <graphics.h>
#include <conio.h>
#include <set>
#include <time.h>
#pragma warning(disable:4996)
static void
enterAOI(void *ud,uint32_t watcher,uint32_t marker) {
	aoiWrap* p = (aoiWrap*)ud;

	if (watcher != p->getSelIdx()) {
		return;
	}
	p->addSelSet(marker);
	printf("[enterAOI] %d enter %d\r\n", watcher, marker);
}

static void
leaveAOI(void* ud, uint32_t watcher, uint32_t marker) {
	aoiWrap* p = (aoiWrap*)ud;
	if (watcher != p->getSelIdx()) {
		return;
	}
	p->delSetSet(marker);
	printf("[leaveAOI] %d leave %d\r\n", watcher, marker);
}

static void
moveAOI(void* ud, uint32_t watcher, uint32_t marker,float newPos[3]) {
	aoiWrap* p = (aoiWrap*)ud;
	printf(" moveAOI w(%d) see m(%d) is moving pos(%f,%f) \r\n", watcher, marker, newPos[0], newPos[1]);
	if (watcher != p->getSelIdx()) {
		return;
	}
	//todo, got pos from aoi module.
	if (!p->existsSelSet(marker)) {
		return;
	}
	//check marker in watcher's set.
}

void aoiWrap::init_obj(uint32_t id,float x,float y,float z,float vx,float vy,float vz,const char *mode) {
	OBJ[id].pos[0] = x;
	OBJ[id].pos[1] = y;
	OBJ[id].pos[2] = z;

	OBJ[id].v[0] = vx;
	OBJ[id].v[1] = vy;
	OBJ[id].v[2] = vz;
	strcpy(OBJ[id].mode,mode);
}

void aoiWrap::update_obj(struct aoi_space *aoi,uint32_t id) {
	int i;
	for (i=0; i<3; i++) {
		OBJ[id].pos[i] += OBJ[id].v[i];
		if (OBJ[id].pos[i] > map_size[i] - view_size[i]) {
			OBJ[id].v[i] *= -1;
		} else if (OBJ[id].pos[i] < 0) {
			OBJ[id].v[i] *= -1;
		}
	}
	aoi_move(aoi,id,OBJ[id].pos);
}

void aoiWrap::doinit(struct aoi_space* space) {
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

void aoiWrap::showOne( int i, int posX, int posY) {
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

void aoiWrap::drawMonitorMapView() {
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

void aoiWrap::drawMonitorText() {
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

void aoiWrap::showObjs(struct aoi_space* space) {

	int i = 0;
	for (i = 0; i < MAX_SIZE; i++) {
		showOne(i, OBJ[i].pos[0], OBJ[i].pos[1]);
	}
	FlushBatchDraw();
}

void aoiWrap::manualMain(struct aoi_space* aoi) {
	BeginBatchDraw();
	int id = 1;
	drawMonitorText();
	while (true) {
		for (int i = 0; i < MAX_SIZE; i++) {
			if (i == select_idx) {
				continue;
			}
			//update_obj(aoi, i);
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
				select_idx++;
				select_idx = select_idx % MAX_SIZE;
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

void* aoiWrap::my_alloc(void* ud, void* ptr, size_t sz) {
	struct alloc_cookie* cookie = (struct alloc_cookie*)ud;
	if (ptr == NULL) {
		// alloc
		void* p = malloc(sz);
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

void aoiWrap::Exec(){
	struct alloc_cookie cookie = {0,0,0};
	initgraph(1200, 800, SHOWCONSOLE);
	setbkcolor(RGB(128, 128, 128));
	cleardevice();
	//void roundrect(int left, int top, int right, int bottom, int ellipsewidth, int ellipseheight);		// Draw a rounded rectangle without filling

	struct aoi_space *aoi = aoi_create(my_alloc,&cookie,map_size,view_size,enterAOI,leaveAOI, moveAOI,this);
	doinit(aoi);
	
	manualMain(aoi);

	//test(aoi);
	aoi_release(aoi);
}
#include "aoiWrap.h"
#include <graphics.h>
#include <conio.h>
#include <set>
#include <time.h>
#include <iostream>
#include <chrono>

#pragma warning(disable:4996)
#define UNSET_POS_VAL -2000
//#define SHOW_CELL_FONT true

aoi_client* aoi_client::_inst;


static int rsFrameCount = 0;
static long frameStartTk = 0;
static long moveCount = 0;
static long aoiCost = 0;

long getMS() {
	using namespace std::chrono;
	uint64_t ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
	return ms;
}

static void
enterAOI(void *ud,uint32_t watcher,uint32_t marker,float pos[3]) {
	aoiWrap* p = (aoiWrap*)ud;

	if (watcher != aoi_client::getInst()->get_sel_idx()) {
		return;
	}
	obj_info oobj;
	oobj.objID = marker;
	oobj.newpos[0] = pos[0];
	oobj.newpos[1] = pos[1];
	oobj.newpos[2] = pos[2];
	p->addAoiObjCli(oobj);
	printf("[enterAOI] %d enter %d\r\n", watcher, marker);
}

static void
leaveAOI(void* ud, uint32_t watcher, uint32_t marker) {
	aoiWrap* p = (aoiWrap*)ud;
	if (watcher != aoi_client::getInst()->get_sel_idx()) {
		return;
	}
	p->delAoiObjCli(marker);
	printf("[leaveAOI] %d leave %d\r\n", watcher, marker);
}

static void
moveAOI(void* ud, uint32_t watcher, uint32_t marker,float newPos[3]) {
	moveCount++;
	aoiWrap* p = (aoiWrap*)ud;
	
	if (watcher != aoi_client::getInst()->get_sel_idx()) {
		return;
	}
	if (!aoi_client::getInst()->objInsight(marker)) {
		return;
	}
	//printf(" moveAOI w(%d) see m(%d) is moving pos(%f,%f) \r\n", watcher, marker, newPos[0], newPos[1]);
	p->updateAoiObjCli(marker, newPos);
}



void aoiWrap::init_obj(uint32_t id,float x,float y,float z,float vx,float vy,float vz,const char *mode) {
	auto pobj = getObj(id);
	if (!pobj) {
		return;
	}
	pobj->pos[0] = x;
	pobj->pos[1] = y;
	pobj->pos[2] = z;

	pobj->v[0] = vx;
	pobj->v[1] = vy;
	pobj->v[2] = vz;
	strcpy(pobj->mode,mode);
}

void aoiWrap::update_obj(struct aoi_space *aoi,uint32_t id) {
	int i;
	auto obj = getObj(id);
	if (!obj) {
		return;
	}
	for (i=0; i<3; i++) {
		obj->pos[i] += obj->v[i];
		if (obj->pos[i] > map_size[i] - view_size[i]) {
			obj->v[i] *= -1;
		} else if (obj->pos[i] < 0) {
			obj->v[i] *= -1;
		}
	}
	aoi_move(aoi,id, obj->pos);
}
void aoiWrap::doinit_one(struct aoi_space* space, int i) {
	static float vec[14][2] = {
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
	float x = rand() % (RECT_X_W - int(view_size[0]));
	float y = rand() % (RECT_Y_H - int(view_size[1]));
	if (i % 2) {
		init_obj(i, x, y, 0, vec[i % 14][0], vec[i % 14][1], 0, "wm");
	}
	else {
		init_obj(i, x, y, 0, vec[i % 14][0], vec[i % 14][1], 0, "wm");
	}
}
void aoiWrap::doinit(struct aoi_space* space,int idx) {
	srand((unsigned)time(NULL));
	for (auto iter = allObjs.begin(); iter != allObjs.end(); iter++) {
		auto obj = iter->second;

		if (idx == -1) {
			doinit_one(space, obj->objID);
			aoi_enter(space, iter->first, obj->pos, obj->mode);

		}else if(obj->objID == idx) {
			doinit_one(space, obj->objID);
			aoi_enter(space, iter->first, obj->pos, obj->mode);
		}

		if (iter->first == aoi_client::getInst()->get_sel_idx()) {
			aoi_client::getInst()->sync_pos(obj->pos);
		}
	}
}

void showOne( int i, int posX, int posY) {
	//circle(posX, int(posY), 1);
	putpixel(posX, posY, RGB(255, 200, 255));


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
	auto viewsize = aoi_client::getInst()->getViewSize();
	if (i == aoi_client::getInst()->get_sel_idx()) {
		//draw view size
		circle(posX, int(posY), 1);
		fillcircle(int(posX), int(posY), 2);
		rectangle(posX - viewsize[0], posY - viewsize[1], posX + viewsize[0], posY + viewsize[1]);
	}
#ifdef SHOW_CELL_FONT
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
#endif
}


void aoiWrap::drawMonitorText() {
	clearrectangle(810, 700, 1200, 800);
	roundrect(810, 700, 1200, 800, 1, 1);
	//1200, 800
	WCHAR info[128];
	wsprintf(info, _T("SEL:%d, Count: %d FC: %d AOICost:%d ViewCount:%d"), aoi_client::getInst()->get_sel_idx(), 
		getObjCount(), rsFrameCount, aoiCost,
		aoi_client::getInst()->objCount());
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
	//*
	for (auto iter = allObjs.begin(); iter != allObjs.end(); iter++) {
		showOne(iter->first, iter->second->pos[0], iter->second->pos[1]);
	}
	//*/
	FlushBatchDraw();
}


void aoiWrap::manualMain(struct aoi_space* aoi) {
	BeginBatchDraw();
	int id = 1;
	
	frameStartTk = getMS();
	int frameCount = 0;

	long drawCost = 0;
	long sleepCost = 0;
	long leftCost = 0;

	long totalCost = 0;
	long totalStart = 0;

	while (true) {
		if (getMS() - frameStartTk > 1000) {
			drawMonitorText();
			rsFrameCount = frameCount;
			frameCount = 0;
			frameStartTk = getMS();
			int viewCount = aoi_client::getInst()->objCount();
			std::cout << "--------------------> draw cost ms:" << drawCost << " sleepCost:" << 
				sleepCost << " leftCost:" << leftCost<<" aoiCost:" << aoiCost << " totalCost:" << totalCost
				<< " moveCount:" << moveCount << " viewCount:"<< viewCount 
				<< " cook.ct:"<<my_cookie.count 
				<< " cook.current:"<< my_cookie.current 
				<<" cook.max:" << my_cookie.max
				<< std::endl;
			moveCount = 0;
			drawCost = 0;
			sleepCost = 0;
			leftCost = 0;
			aoiCost = 0;
			totalCost = 0;
			
		}
		frameCount++;
		long sms = getMS();
		totalStart = sms;
		auto select_idx = aoi_client::getInst()->get_sel_idx();
		for (auto iter = allObjs.begin(); iter != allObjs.end(); ++iter) {
			if (iter->first == select_idx) {
				continue;
			}
			update_obj(aoi, iter->first);
		}
		
		aoiCost += getMS() - sms;
		//cleardevice();
		sms = getMS();
		clearroundrect(0, 0, RECT_X_W, RECT_Y_H, 1, 1);
		roundrect(0, 0, RECT_X_W, RECT_Y_H, 1, 1);
		
		showObjs(aoi);
		aoi_client::getInst()->drawMonitorMapView();
		drawCost += getMS() - sms;
		sms = getMS();
		Sleep(1);
		sleepCost += getMS() - sms;

		sms = getMS();
		if (_kbhit()) {
			char key = _getch();
			//printf("key:%d\r\n", key);
			if (key == 49) {
				addOneEnterAOI();
			}
			else if (key == 50) {
				decOne();
			}
			auto pobj = getObj(select_idx);
			//if (pobj) {
			if (key == 75) {
				if (pobj) {
					pobj->pos[0] += -10;
				}
			}
			else if (key == 77) {
				if (pobj) {
					pobj->pos[0] += 10;
				}
			}
			else if (key == 72) {
				if (pobj) {
					pobj->pos[1] -= 10;
				}
			}
			else if (key == 80) {
				if (pobj) {
					pobj->pos[1] += 10;
				}
			}
			else if (key == 115) {
				//todo, get next obj.
				select_idx = getRandIdx();
				//clearSelSet();
				aoi_client::getInst()->update_sel_idx(select_idx);
				//drawMonitorText();
				pobj = getObj(select_idx);
			}
			//adajust pos
			if (pobj) {
				if (pobj->pos[0] < 0) {
					pobj->pos[0] = 0;
				}
				if (pobj->pos[1] < 0) {
					pobj->pos[1] = 0;
				}
				if (pobj->pos[0] > RECT_X_W - view_size[0]) {
					pobj->pos[0] = RECT_X_W - view_size[0];
				}
				if (pobj->pos[1] > RECT_Y_H - view_size[1]) {
					pobj->pos[1] = RECT_Y_H - view_size[1];
				}
				aoi_client::getInst()->sync_pos(pobj->pos);
				//printf("%d was pressed (%f,%f)\r\n", key,OBJ[id].pos[0], key, OBJ[id].pos[1]);
				//dumpall();
				if (key == 115) {
					aoi_enter(aoi, select_idx, pobj->pos, pobj->mode);
				}
				aoi_move(aoi, select_idx, pobj->pos);
			}
			//}
		}
		leftCost += getMS() - sms;
		totalCost += getMS() - totalStart;
	}
}


aoiWrap::aoiWrap(int size, float mapsize[3], float viewsize[3]) {
	for (int i = 0; i < size; i++) {
		OBJECT* op = new OBJECT;
		op->objID = i;
		allObjs.insert(std::make_pair<>(i, op));
	}
	for (int i = 0; i < 3; i++) {
		map_size[i] = mapsize[i];
		view_size[i] = view_size[i];
	}
	aoi_client::getInst()->setViewSize(viewsize);
}
int aoiWrap::getObjCount() {
	return allObjs.size();
}
OBJECT* aoiWrap::getObj(int id) {
	auto iter = allObjs.find(id);
	if (iter != allObjs.end()) {
		return allObjs[id];
	}
	return NULL;
}
void aoiWrap::addOneEnterAOI() {
	OBJECT* op = new OBJECT;
	int maxID = 0;
	for (auto iter = allObjs.begin(); iter != allObjs.end(); ++iter) {
		if (iter->first > maxID) {
			maxID = iter->first;
		}
	}
	op->objID = maxID + 1;
	allObjs.insert(std::make_pair<>(op->objID, op));
	doinit(aoi, op->objID);
}
int aoiWrap::getRandIdx() {
	int idx = 0;
	int count = rand() % getObjCount();
	int i = 0;
	for (auto iter = allObjs.begin(); iter != allObjs.end(); ++iter) {
		if (i++ == count) {
			idx = iter->first;
			break;
		}
	}
	return idx;
}
void aoiWrap::decOne() {
	if (getObjCount() == 0) {
		return;
	}
	int idx = getRandIdx();
	if (allObjs.count(idx)) {
		auto iter = allObjs.find(idx);
		aoi_leave(this->aoi, iter->second->objID);
		delete iter->second;
		allObjs.erase(iter);
		printf("remove idx: %d\r\n", idx);
	}
	else {
		printf("remove failed, idx: %d\r\n", idx);
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
	initgraph(1200, 800, SHOWCONSOLE);
	setbkcolor(RGB(128, 128, 128));
	cleardevice();

	aoi = aoi_create(my_alloc,&my_cookie,map_size,view_size,enterAOI,leaveAOI, moveAOI,this);
	doinit(aoi);
	
	manualMain(aoi);

	//test(aoi);
	aoi_release(aoi);
}


void aoiWrap::updateAoiObjCli(int objid, float newPos[3]) {
	obj_info mi;
	for (int i = 0; i < 3; i++) {
		mi.newpos[i] = newPos[i];
	}
	mi.objID = objid;
	aoi_client::getInst()->updateMi(mi);
}

void aoiWrap::delAoiObjCli(int objid) {
	aoi_client::getInst()->delMi(objid);
}


void aoi_client::updateMi(obj_info mi) {
	if (all_obj.count(mi.objID)) {
		all_obj[mi.objID] = mi;
	}
	else {
		std::cout << "updateMi failed, objid:" << mi.objID << std::endl;
	}
}
void aoi_client::delMi(int objID) {
	if (all_obj.count(objID)) {
		all_obj.erase(all_obj.find(objID));
	}
	else {
		std::cout << "delMi failed, objid:" << objID << std::endl;
	}
}

void aoi_client::drawMonitorMapView() {
	clearrectangle(800, 0, 1200, 400 + view_size[1] * 2);
	roundrect(810, 3, 810 + view_size[0] * 2, 3 + view_size[1] * 2, 1, 1);
	int posCenterX = 810 + view_size[0];
	int posCenterY = view_size[0] + 3;
	circle(posCenterX, posCenterY, 1);
	//draw all inside;
	for (auto iter = all_obj.begin(); iter != all_obj.end(); iter++) {
		auto obj = iter->second;
		if (obj.newpos[0] == UNSET_POS_VAL) {
			continue;
		}
		int sidx = obj.objID;
		int diff_x = obj.newpos[0] - mypos[0];
		int diff_y = obj.newpos[1] - mypos[1];
		int relate_x = posCenterX + diff_x;
		int relate_y = posCenterY + diff_y;
		showOne(sidx, relate_x, relate_y);
	}
}
#pragma once

extern "C" {
	#include "aoi.h"
}

#include <set>
#include <list>
#include <map>

//#define MAX_SIZE 33
#define RECT_X_W 800
#define RECT_Y_H 800

typedef struct OBJECT {
	float pos[3];
	float v[3];
	char mode[4];
	int objID;
} OBJECT;

struct alloc_cookie {
	int count;
	int max;
	int current;
};

struct obj_info {
	int objID;
	float newpos[3];
};

static float view_size[3] = { 100,100,100 };
static void showOne(int i, int posX, int posY);

class aoi_client {
	static aoi_client *_inst;
	std::map<int, obj_info> all_obj;
	int select_idx = 1;
	float mypos[3];
public:
	static aoi_client* getInst() {
		if (!_inst) {
			_inst = new aoi_client;
		}
		return _inst;
	}

	void clear() {
		all_obj.clear();
	}
	bool objInsight(int oid) {
		return all_obj.count(oid) != 0;
	}
	void addMi(obj_info mi) {
		all_obj[mi.objID] = mi;
	}
	void updateMi(obj_info mi);
	void delMi(int objID);
	void drawMonitorMapView();
	int get_sel_idx() { return select_idx; }
	void update_sel_idx(int val) { if (select_idx == val)return; select_idx = val; clear(); }
	void sync_pos(float pos[3]) {
		for (int i = 0; i < 3; i++) {
			mypos[i] = pos[i];
		}
	}
};

class aoiWrap {
	private:

	//struct OBJECT OBJ[MAX_SIZE];
	std::map<int, OBJECT*> allObjs;

	 float map_size[3] = { RECT_X_W,RECT_Y_H,100};
	 
	 struct alloc_cookie cookie = { 0,0,0 };
	 struct aoi_space *aoi;
	 std::set<int> select_insight_set;
	 static void* my_alloc(void* ud, void* ptr, size_t sz);
	 void init_obj(uint32_t id, float x, float y, float z, float vx, float vy, float vz, const char* mode);
	 void doinit(struct aoi_space* space, int i=-1);
	 void doinit_one(struct aoi_space* space,int idx);
	 
	 void drawMonitorText();
	 void showObjs(struct aoi_space* space);
	 void update_obj(struct aoi_space* aoi, uint32_t id);

public:
	aoiWrap(int size = 3000);
	int getObjCount();
	OBJECT* getObj(int id);
	void addOneEnterAOI();
	int getRandIdx();
	void decOne();
	void addAoiObjCli(obj_info mi) {
		aoi_client::getInst()->addMi(mi);
	}
	void updateAoiObjCli(int objid, float newPos[3]);
	void delAoiObjCli(int objid);

	void clearMoveList() {
		aoi_client::getInst()->clear();
	}
	 void manualMain(struct aoi_space* aoi);

	 void Exec();
};
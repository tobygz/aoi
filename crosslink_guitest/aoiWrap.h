#pragma once

extern "C" {
	#include "aoi.h"
}

#include <set>

#define MAX_SIZE 2
#define RECT_X_W 800
#define RECT_Y_H 800

typedef struct OBJECT {
	float pos[3];
	float v[3];
	char mode[4];
} OBJECT;

struct alloc_cookie {
	int count;
	int max;
	int current;
};

class aoiWrap {
	private:
	struct OBJECT OBJ[MAX_SIZE];
	 float map_size[3] = { RECT_X_W,RECT_Y_H,100};
	 float view_size[3] = {100,100,100};
	 struct alloc_cookie cookie = { 0,0,0 };
	 struct aoi_space *aoi;
	 int select_idx = 1;
	 std::set<int> select_insight;
	 static void* my_alloc(void* ud, void* ptr, size_t sz);
	 void init_obj(uint32_t id, float x, float y, float z, float vx, float vy, float vz, const char* mode);
	 void doinit(struct aoi_space* space);
	 void showOne(int i, int posX, int posY);
	 void drawMonitorMapView();
	 void drawMonitorText();
	 void showObjs(struct aoi_space* space);
	 void update_obj(struct aoi_space* aoi, uint32_t id);

public:
	aoiWrap() {
	}
	 void manualMain(struct aoi_space* aoi);
	 int getSelIdx() { return select_idx; }
	 void addSelSet(int val) { select_insight.insert(val); }
	 bool existsSelSet(int val) { return select_insight.count(val) > 0; }
	 void delSetSet(int val) { if (select_insight.count(val)) { select_insight.erase(select_insight.find(val)); } }
	 void clearSelSet() { select_insight.clear(); }
	 void Exec();
};
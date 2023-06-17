/*
 * mesh.h
 *
 *  Created on: 17.06.2023
 *      Author: D-Laptop
 */

#ifndef LIB_MESH_MESH_H_
#define LIB_MESH_MESH_H_

#include "painlessMesh.h"

#include <disp.h>

#define MESH_PREFIX "whateverYouLike"
#define MESH_PASSWORD "somethingSneaky"
#define MESH_PORT 5555

class MeshManagement{
public:
	MeshManagement() {}
	void init();
	void loop();
	void dispFoundStations(DispManagement disp);
private:
//	void receivedCallback(uint32_t, String);

	Scheduler userScheduler;  // to control your personal task
	painlessMesh myMesh;
};

#endif /* LIB_MESH_MESH_H_ */

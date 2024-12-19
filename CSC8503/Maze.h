#pragma once

#include "NavigationGrid.h"
#include "NavigationMesh.h"
#include "Vector.h"





using namespace NCL::Maths;

namespace NCL {
	
	namespace CSC8503 {
		class Maze {
		public:
			NavigationGrid grid{ "TestGrid1.txt" };
			vector<Vector3> nodes;

			GameObject* enemy;

			bool pathFound = false;
			bool chasingEnemy = false;
			int totalNodes = 0;
			float progress = 0;
			float speed = .7f;


			bool FindPath(Vector3 start, Vector3 end) {

				nodes.clear();
				NavigationPath outPath;
				bool found = grid.FindPath(start, end, outPath);

				Vector3 pos;
				while (outPath.PopWaypoint(pos)) {
					nodes.push_back(pos);
					totalNodes = nodes.size();
					progress = 0;
					pathFound = true;
				}
				return found;
			}

			void DisplayPath() {
				for (int i = 1; i < nodes.size(); ++i) {
					Vector3 a = nodes[i - 1];
					Vector3 b = nodes[i];

					Debug::DrawLine(a, b, Vector4(0, 1, 0, 1));
				}
			}

			
			
		protected:
		};
	}
}
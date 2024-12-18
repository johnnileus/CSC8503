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
			vector<Vector3> testNodes;




			void TestPathfinding() {


				Vector3 startPos(80, 0, 10);
				Vector3 endPos(80, 0, 80);
				NavigationPath outPath;

				bool found = grid.FindPath(startPos, endPos, outPath);

				Vector3 pos;
				while (outPath.PopWaypoint(pos)) {
					testNodes.push_back(pos);
				}
			}

			void DisplayPath() {
				for (int i = 1; i < testNodes.size(); ++i) {
					Vector3 a = testNodes[i - 1];
					Vector3 b = testNodes[i];

					Debug::DrawLine(a, b, Vector4(0, 1, 0, 1));
				}
			}
			
		protected:
		};
	}
}
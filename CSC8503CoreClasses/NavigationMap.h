#pragma once
#include "NavigationPath.h"
namespace NCL {
	namespace CSC8503 {
		class NavigationMap
		{
		public:
			NavigationMap() {}
			~NavigationMap() {}

			virtual bool FindPath(const Vector3& from, const Vector3& to, NavigationPath& outPath) = 0;
		};
	}
}


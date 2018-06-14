#include "BoostMetaGraph.h"
#include <stdio.h>
#include <iostream>
namespace
{
	struct mvec
	{
		float data[3];

		mvec()
		{
			for (int i = 0; i < 3; ++i)
			{
				data[i] = 0.0;
			}
		}
		mvec(float *v)
		{
			for (int i = 0; i < 3; ++i)
			{
				data[i] = v[i];
			}
		}
		mvec(float x, float y, float z)
		{
			data[0] = x;
			data[1] = y;
			data[2] = z;
		}
		mvec(Point3d p)
		{
			for (int i = 0; i < 3; ++i)
			{
				data[i] = p[i];
			}
		}
		float& operator[](size_t i)
		{
			return data[i];
		}
		mvec operator+(mvec &other)
		{
			mvec result = mvec();
			for (int i = 0; i < 3; ++i)
			{
				result[i] = operator[](i) + other[i];
			}
			return result;
		}
		mvec operator+(Point3d &other)
		{
			return operator+(mvec(other));
		}
		mvec operator-(mvec &other)
		{
			mvec result = mvec();
			for (int i = 0; i < 3; ++i)
			{
				result[i] = operator[](i) - other[i];
			}
			return result;
		}
		mvec operator-(Point3d &other)
		{
			return operator-(mvec(other));
		}
		mvec operator/(double div)
		{
			mvec result = mvec();
			for (int i = 0; i < 3; ++i)
			{
				result[i] = operator[](i) / div;
			}
			return result;
		}
		mvec operator*(double mul)
		{
			mvec result = mvec();
			for (int i = 0; i < 3; ++i)
			{
				result[i] = operator[](i) * mul;
			}
			return result;
		}
		double norm()
		{
			double result = 0.0;
			for (int i = 0; i < 3; ++i)
			{
				result += data[i] * data[i];
			}
			return sqrt(result);
		}
		void normalize()
		{
			double normal = norm();
			for (int i = 0; i < 3; ++i)
			{
				data[i] /= normal;
			}
		}

		float dot(mvec other)
		{
			float result = 0;
			for (int i = 0; i < 3; ++i)
			{
				result += operator[](i) * other[i];
			}
			return result;
		}
		mvec cross(mvec other)
		{
			mvec result = mvec();
			result[0] = operator[](1) * other[2] - operator[](2) *other[1];
			result[1] = operator[](2) * other[0] - operator[](0) * other[2];
			result[3] = operator[](0) * other[1] - operator[](1) * other[0];
			return result;
		}
	};
	
	//sets hitDist to -1 if it is not a valid hit
	void linePick(mvec rayDir, mvec rayOrigin, mvec p0, mvec p1, float &rayToRayDist, float &hitDist)
	{
		mvec u = rayDir;
		mvec v = p1 - p0;
		v.normalize();
		mvec w = rayOrigin - p0;

		float a = u.dot(u);
		float b = u.dot(v);
		float c = v.dot(v);
		float d = u.dot(w);
		float e = v.dot(w);
		float D = a*c - b*b;
		float sc = 0.0;
		float tc = 0.0;


		if (D < 0.0001)
		{
			if (b > c)
			{
				tc = d / b;
			}
			else
			{
				tc = e / c;
			}
		}
		else
		{
			sc = (b*e - c*d) / D;
			tc = (a*e - b*d) / D;
		}

		mvec dP = w + (u*sc) - (v*tc);
		
		rayToRayDist = dP.norm();

		mvec lineVec = p0 - p1;

		float lineLength = lineVec.norm();

		if (lineLength > tc && tc > 0 & sc > 0)
		{
			hitDist = sc;
		}
		else
		{
			hitDist = -1;
		}
	}

	void pointPick(mvec rayDir, mvec rayOrigin, mvec point, float pointScale, bool &hits, float &hitDist)
	{
		mvec rayToCenter = point - rayOrigin;
		hitDist = rayDir.dot(rayToCenter);

		mvec closestApproach = rayOrigin + rayDir * hitDist;

		mvec rayPointVec = (point - closestApproach);
		float rayPointDist = rayPointVec.norm();

		if (rayPointDist < pointScale)
		{
			hits = true;
		}
		else
		{
			hits = false;
			hitDist = -1;
		}
	}
}

namespace Roots
{

	//void BMetaGraph::selectMetaNode(float eyeX, float eyeY, float eyeZ, float lookX, float lookY, float lookZ)
	//{
	//	MetaV node;
	//	bool isValid = false;

	//	node = getFirstMetaNodeHit(eyeX, eyeY, eyeZ, lookX, lookY, lookZ, isValid);

	//	if (isValid)
	//	{
	//		//if the 'lead' nnode is already selected
	//		if (selectNode1Valid)
	//		{
	//			//if we reselect the lead node, then unselect it, leaving the follow node as the lead and return
	//			if (node == selectNode1)
	//			{
	//				selectNode1Valid = selectNode2Valid;
	//				selectNode1 = selectNode2;
	//				selectNode2Valid = false;
	//				return;
	//			}
	//			//if the 'follow' node is already selected and we did not select the lead node again
	//			if (selectNode2Valid)
	//			{
	//				//if the 'follow' node is selected and we picked it again, remove the follow node
	//				if (node == selectNode2)
	//				{
	//					selectNode2Valid = false;
	//					return;
	//				}
	//				//if the follow node is selected but we didn't pick either it or the lead node
	//				else
	//				{
	//					//if the new node is on the same component as the lead node but not the same as the follow,
	//					//replace the lead node with this one and leave the follow node alone
	//					if (operator[](node).connectedComponent == operator[](selectNode1).connectedComponent
	//						&& operator[](node).connectedComponent != operator[](selectNode2).connectedComponent)
	//					{
	//						selectNode1Valid = true;
	//						selectNode1 = node;
	//					}
	//					//in all other circumstances, replace the trailing node with the lead node and the
	//					//lead node with this node
	//					else
	//					{
	//						selectNode2Valid = selectNode1Valid;
	//						selectNode2 = selectNode1;

	//						selectNode1 = node;
	//						selectNode1Valid = true;
	//						return;
	//					}
	//				}
	//			}
	//			//if the 'follow' node is unselected then set the follow node to the lead node and the lead node as this
	//			else
	//			{
	//				selectNode2Valid = selectNode1Valid;
	//				selectNode2 = selectNode1;

	//				selectNode1 = node;
	//				selectNode1Valid = true;
	//				return;
	//			}
	//		}
	//		//if no lead node selected this will become the lead node
	//		else
	//		{
	//			selectNode1 = node;
	//			selectNode1Valid = true;
	//		}
	//	}
	//}
	//void BMetaGraph::selectPromotionNode(float eyeX, float eyeY, float eyeZ, float lookX, float lookY, float lookZ)
	//{
	//	SkelVert vert;
	//	bool isValid = false;
	//	vert = getFirstSkelNodeHit(eyeX, eyeY, eyeZ, lookX, lookY, lookZ, isValid);
	//	std::cout << "Selecting vert " << std::endl;
	//	if (isValid)
	//	{
	//		std::cout << "Vert is valid " << std::endl;
	//		if (selectVertValid)
	//		{
	//			//if we reselect the same vertex, then remove it
	//			if (vert == selectVert)
	//			{
	//				selectVertValid = false;
	//			}
	//			else
	//			{
	//				selectVert = vert;
	//				selectVertValid = true;
	//			}
	//		}
	//		else
	//		{
	//			std::cout << "First vert selected, set to selectVert " << vert << std::endl;
	//			selectVert = vert;
	//			selectVertValid = true;
	//		}
	//	}
	//}

	void BMetaGraph::selectConnectionNode(float eyeX, float eyeY, float eyeZ, float lookX, float lookY, float lookZ)
	{
		MetaV node;
		bool isValid = false;

		node = getFirstMetaNodeHit(eyeX, eyeY, eyeZ, lookX, lookY, lookZ, isValid);
		if (isValid)
		{
			privateSelectConnectionNode(operator[](node).mSrcVert, operator[](node).connectedComponent);
		}
		else
		{
			SkelVert vert;
			isValid = false;
			vert = getFirstSkelNodeHit(eyeX, eyeY, eyeZ, lookX, lookY, lookZ, isValid);
			if (isValid)
			{
				metaEdgeIter mei = boost::edges(*this);
				int connectedComponent = -1;
				for (; mei.first != mei.second; ++mei)
				{
					BMetaEdge *edge = &operator[](*mei.first);
					for (SkelVert v : edge->mVertices)
					{
						if (vert == v)
						{
							connectedComponent = edge->connectedComponent;
							break;
						}
					}
					if (connectedComponent != -1)
					{
						break;
					}
				}

				privateSelectConnectionNode(vert, connectedComponent);
			}
		}
		//if (isValid)
		//{
		//	std::cout << "...Node params... " << std::endl;
		//	std::cout << "Degree : " << boost::degree(node, *this) << std::endl;
		//	std::cout << "Component : " << operator[](node).connectedComponent << std::endl;
		//	std::cout << "Thickness : " << operator[](node).nodeThickness << std::endl;
		//	std::cout << "Width : " << operator[](node).nodeWidth << std::endl;
		//	std::cout << "................." << std::endl;
		//}
	}

	void BMetaGraph::privateSelectConnectionNode(SkelVert nodeVert, int selectComponent)
	{
		int component1=-1, component2=-1;
		SkelVert selectVert1=99999, selectVert2=999999;
		if (selectNode1Valid)
		{
			component1 = operator[](selectNode1).connectedComponent;
			selectVert1 = operator[](selectNode1).mSrcVert;
			if (boost::degree(selectNode1, *this) == 0)
			{
				removeNode(selectNode1);
				selectNode1 = 99999;
			}
		}
		if (selectNode2Valid)
		{
			component2 = operator[](selectNode2).connectedComponent;
			selectVert2 = operator[](selectNode2).mSrcVert;
			if (boost::degree(selectNode2, *this) == 0)
			{
				removeNode(selectNode2);
				selectNode2 = 99999;
			}
		}
		std::cout << "private select connection node " << std::endl;
		for (int i = 0; i < 1; ++i)
		{
			if (selectNode1Valid)
			{
				if (nodeVert == selectVert1)
				{
					std::cout << "Selecting node1 again" << std::endl;
					if (selectNode2Valid)
					{
						selectNode1Valid = selectNode2Valid;
						selectNode1 = selectNode2;
						component1 = component2;
						selectVert1 = selectVert2;
						selectNode2Valid = false;
						break;
					}
					else
					{
						std::cout << "No node 2 " << std::endl;
						selectNode1Valid = false;
						break;
					}
				}
				else if (selectComponent == component1)
				{
					std::cout << "Selecting on same component " << std::endl;
					selectNode1Valid = true;
					component1 = selectComponent;
					selectVert1 = nodeVert;
					if (vertNodeMap.count(nodeVert) > 0)
					{
						selectNode1 = vertNodeMap[nodeVert];
					}
					else
					{
						selectNode1 = 999999;
					}
					break;
				}
				else
				{
					if (selectNode2Valid)
					{
						if (nodeVert == selectVert2)
						{
							std::cout << "selecting node2 again" << std::endl;
							selectNode2Valid = false;
							break;
						}
					}
					std::cout << "Selecting new node1, pushing node1 onto node 2" << std::endl;
					selectNode2Valid = selectNode1Valid;
					selectNode2 = selectNode1;
					selectVert2 = selectVert1;
					component2 = component1;
					selectNode1Valid = true;
					component1 = selectComponent;
					selectVert1 = nodeVert;
					if (vertNodeMap.count(nodeVert) > 0)
					{
						selectNode1 = vertNodeMap[nodeVert];
					}
					else
					{
						selectNode1 = 999999;
					}
					break;
				}
			}
			else
			{
				std::cout << "selecting new node 1 " << std::endl;
				selectNode1Valid = true;
				component1 = selectComponent;
				selectVert1 = nodeVert;
				if (vertNodeMap.count(nodeVert) > 0)
				{
					selectNode1 = vertNodeMap[nodeVert];
				}
				else
				{
					selectNode1 = 999999;
				}
				break;
			}
		}
		
		if (selectNode2Valid)
		{
			std::cout << "component2 " << component2 << std::endl;
			std::cout << "vert 2 " << selectVert2 << std::endl;
			if (vertNodeMap.count(selectNode2) == 0)
			{
				std::cout << "Creating node 2 " << std::endl;
				selectNode2 = addNode(selectVert2, &mSkeleton, true);
				operator[](selectNode2).connectedComponent = component2;
				operator[](selectNode2).updateColors(nodeOptions);
			}
			std::cout << "Node 2 " << selectNode2 << std::endl;
		}
		if (selectNode1Valid)
		{
			std::cout << "component 1 " << component1 << std::endl;
			std::cout << "vert 1 " << selectVert1 << std::endl;
			if (vertNodeMap.count(selectNode1) == 0)
			{
				std::cout << "Creating node 1" << std::endl;
				selectNode1 = addNode(selectVert1, &mSkeleton, true);
				operator[](selectNode1).connectedComponent = component1;
				operator[](selectNode1).updateColors(nodeOptions);
			}
			std::cout << "Node 1 " << selectNode1 << std::endl;
		}

		std::cout << "Leaving private selection " << std::endl;

		
		////if the 'lead' nnode is already selected
		//if (selectNode1Valid)
		//{
		//	//if we reselect the lead node, then unselect it, leaving the follow node as the lead and return
		//	if (node == selectNode1)
		//	{
		//		if (boost::degree(selectNode1, *this) == 0)
		//		{
		//			if (selectNode1 < selectNode2)
		//			{
		//				selectNode2 -= 1;
		//			}
		//			if (selectNode1 < node)
		//			{
		//				node -= 1;
		//			}
		//			removeNode(selectNode1);
		//		}
		//		selectNode1Valid = selectNode2Valid;
		//		selectNode1Promoted = selectNode2Promoted;
		//		selectNode1 = selectNode2;

		//		selectNode2Valid = false;
		//		selectNode2Promoted = false;
		//		return;
		//	}
		//	//if the 'follow' node is already selected and we did not select the lead node again
		//	if (selectNode2Valid)
		//	{
		//		//if the 'follow' node is selected and we picked it again, remove the follow node
		//		if (node == selectNode2)
		//		{
		//			if (boost::degree(selectNode2, *this) == 0)
		//			{
		//				if (selectNode2 < selectNode1)
		//				{
		//					selectNode1 -= 1;
		//				}
		//				if (selectNode2 < node)
		//				{
		//					node -= 1;
		//				}
		//				removeNode(selectNode2);
		//			}

		//			selectNode2Valid = false;
		//			selectNode2Promoted = false;
		//			return;
		//		}
		//		//if the follow node is selected but we didn't pick either it or the lead node
		//		else
		//		{
		//			//if the new node is on the same component as the lead node but not the same as the follow,
		//			//replace the lead node with this one and leave the follow node alone
		//			if (operator[](node).connectedComponent == operator[](selectNode1).connectedComponent
		//				&& operator[](node).connectedComponent != operator[](selectNode2).connectedComponent)
		//			{
		//				if (boost::degree(selectNode1, *this) == 0)
		//				{
		//					if (selectNode1 < selectNode2)
		//					{
		//						selectNode2 -= 1;
		//					}
		//					if (selectNode1 < node)
		//					{
		//						node -= 1;
		//					}
		//					removeNode(selectNode1);
		//				}
		//				selectNode1Valid = true;
		//				selectNode1 = node;
		//				selectNode1Promoted = isPromoted;
		//			}
		//			//in all other circumstances, replace the trailing node with the lead node and the
		//			//lead node with this node
		//			else
		//			{
		//				if (boost::degree(selectNode2, *this) == 0)
		//				{
		//					if (selectNode2 < selectNode1)
		//					{
		//						selectNode1 -= 1;
		//					}
		//					if (selectNode2 < node)
		//					{
		//						node -= 1;
		//					}
		//					removeNode(selectNode2);
		//				}
		//				selectNode2Valid = selectNode1Valid;
		//				selectNode2Promoted = selectNode1Promoted;
		//				selectNode2 = selectNode1;

		//				selectNode1 = node;
		//				selectNode1Valid = true;
		//				selectNode1Promoted = isPromoted;
		//				return;
		//			}
		//		}
		//	}
		//	//if the 'follow' node is unselected then set the follow node to the lead node and the lead node as this
		//	else
		//	{
		//		selectNode2Valid = selectNode1Valid;
		//		selectNode2 = selectNode1;
		//		selectNode2Promoted = selectNode1Promoted;

		//		selectNode1 = node;
		//		selectNode1Valid = true;
		//		selectNode1Promoted = isPromoted;
		//		return;
		//	}
		//}
		////if no lead node selected this will become the lead node
		//else
		//{
		//	selectNode1 = node;
		//	selectNode1Valid = true;
		//	selectNode1Promoted = isPromoted;
		//}
	}

	void BMetaGraph::selectBreakEdge(float eyeX, float eyeY, float eyeZ, float lookX, float lookY, float lookZ)
	{
		std::pair<MetaV, MetaV> edge;
		bool isValid = false;

		edge = getFirstMetaEdgeHit(eyeX, eyeY, eyeZ, lookX, lookY, lookZ, isValid);

		//if we have a valid selection do stuff
		if (isValid)
		{
			//unselectAllEdges();
			//if there is an existing break edge we want to unselect it
			if (breakEdgeValid)
			{
				//if we selected the existing break edge, simply unselect that edge and return
				if ((edge.first == breakEdge.first || edge.first == breakEdge.second) && 
					(edge.second == breakEdge.first || edge.second == breakEdge.second))
				{
					std::cout << "Breakedge == edge -> unselect break edge" << std::endl;
					unselectEdge(edge);
					breakEdgeValid = false;
					std::cout << "Finished assignment " << std::endl;
				}
				//elsewise, unselect that break edge and replace it with this one
				else
				{
					std::cout << "Breakedge != edge -> unselect break edge and assign as edge" << std::endl;
					selectEdge(edge);
					unselectEdge(breakEdge);
					breakEdge = edge;
					breakEdgeValid = true;
					std::cout << "Finished assignment " << std::endl;
				}
			}
			//if there is existing break edge, we set the break edge valid to true and select the edge
			else
			{
				std::cout << "No existing break edge -> assigning this edge as the break edge " << std::endl;
				breakEdgeValid = true;
				breakEdge = edge;
				selectEdge(edge);
				std::cout << "Finished assignment " << std::endl;
			}
		}
		if (breakEdgeValid)
		{
			MetaE e;
			bool exists;
			boost::tie(e, exists) = boost::edge(breakEdge.first, breakEdge.second, *this);

			std::cout << "...Edge parameters..." << std::endl;
			std::cout << "Component : " << operator[](e).connectedComponent << std::endl;
			std::cout << "Thickness : " << operator[](e).averageThickness << std::endl;
			std::cout << "Width : " << operator[](e).averageWidth << std::endl;
			std::cout << "IsBridge : " << std::to_string(operator[](e).isBridge) << std::endl;
			std::cout << "v0 " << vertNodeMap[operator[](e).start()] << std::endl;
			std::cout << "v1 " << vertNodeMap[operator[](e).end()] << std::endl;
			std::cout << "....................." << std::endl;
		}
	}
	void BMetaGraph::selectSplitEdge(float eyeX, float eyeY, float eyeZ, float lookX, float lookY, float lookZ)
	{
		std::pair<MetaV, MetaV> edge;
		bool isValid = false;
		edge = getFirstMetaEdgeHit(eyeX, eyeY, eyeZ, lookX, lookY, lookZ, isValid);
		if (!isValid)
		{
			return;
		}
		MetaE edgeMetaE;
		bool exists = false;
		boost::tie(edgeMetaE, exists) = boost::edge(edge.first, edge.second, *this);
		if (exists)
		{
			BMetaEdge *selectedEdge = &operator[](edgeMetaE);

			if (splitEdgeValid)
			{
				MetaE splitEdgeE;
				boost::tie(splitEdgeE, exists) = boost::edge(splitEdge.first, splitEdge.second, *this);
				BMetaEdge *existingEdge = &operator[](splitEdgeE);
				//if there is already a splitEdge and it matches the selected, unselect it and return
				if (edge.first == splitEdge.first && edge.second == splitEdge.second ||
					edge.second == splitEdge.first && edge.first == splitEdge.second)
				{
					splitEdgeValid = false;
					unselectEdge(edge);
				}
				//if there is salready a spltEdge and it is neighbors with the selected, add the selected to the neighbors list
				else if (edge.first == splitEdge.first || edge.second == splitEdge.first || 
					edge.first == splitEdge.second || edge.second == splitEdge.second)
				{
					//if the neighbor is already in the neighbors list, then remove it and return
					bool inlist = false;
					for (std::pair<MetaV, MetaV> neighbor : splitNeighbors)
					{
						if (neighbor == edge)
						{
							unselectEdge(neighbor);
							inlist = true;
							break;
						}
					}
					//else add it to the neighbors list and select that edge
					if (!inlist)
					{
						splitNeighbors.push_back(edge);
						selectEdge(edge);
						splitEdgeValid = true;
					}
				}
				//if there is already a split edge and this edge is not its neighbor, then replace the split edge
				//and unselect all existing edges
				else
				{
					unselectEdge(splitEdge);
					for (std::pair<MetaV, MetaV> neighbor : splitNeighbors)
					{
						unselectEdge(neighbor);
					}
					splitNeighbors = {};
					splitEdge = edge;
					selectEdge(splitEdge);
					splitEdgeValid = true;
				}
			}
			else
			{
				splitEdgeValid = true;
				splitEdge = edge;
				selectEdge(splitEdge);
				for (std::pair<MetaV, MetaV> neighbor : splitNeighbors)
				{
					unselectEdge(neighbor);
				}
				splitNeighbors = {};
			}
		}

		if (splitEdgeValid)
		{
			MetaE e;
			bool exists;
			boost::tie(e, exists) = boost::edge(splitEdge.first, splitEdge.second, *this);

			std::cout << "...Edge parameters..." << std::endl;
			std::cout << "Component : " << operator[](e).connectedComponent << std::endl;
			std::cout << "Thickness : " << operator[](e).averageThickness << std::endl;
			std::cout << "Width : " << operator[](e).averageWidth << std::endl;
			std::cout << "IsBridge : " << std::to_string(operator[](e).isBridge) << std::endl;
			std::cout << "....................." << std::endl;
		}

	}

	MetaV BMetaGraph::getFirstMetaNodeHit(float eyeX, float eyeY, float eyeZ, float lookX, float lookY, float lookZ, bool &isValid)
	{
		mvec rayOrigin = mvec(eyeX, eyeY, eyeZ);
		mvec rayDir = mvec(lookX, lookY, lookZ);
		
		bool hitFound = false;
		float minDist = 100000000;
		MetaV nodeHit = 0;
		
		metaVertIter mvi = boost::vertices(*this);
		for (; mvi.first != mvi.second; ++mvi)
		{
			BMetaNode *node = &operator[](*mvi.first);
			if (onlyDisplaySelectedComponents)
			{
				if (node->connectedComponent != selectedComponent1 && node->connectedComponent != selectedComponent2)
				{
					continue;
				}
			}
			mvec point = mvec(node->p);

			bool hits = false;
			float hitDist = 10000000;
			float scale = 0.0;
			int degree = boost::degree(*mvi.first, *this);
			if (degree <= 1)
			{
				scale = nodeOptions.endpointScale;
			}
			else if (degree > 2)
			{
				scale = nodeOptions.junctionScale;
			}

			if (*mvi.first == selectNode1 || *mvi.first == selectNode2)
			{
				scale *= nodeSelectionScaling;
			}

			pointPick(rayDir, rayOrigin, point, scale, hits, hitDist);

			if (hits && hitDist > 0)
			{
				if (hitDist < minDist)
				{
					hitFound = true;
					nodeHit = *mvi.first;
					minDist = hitDist;
				}
			}
		}
		isValid = hitFound;
		return nodeHit;

	}
	SkelVert BMetaGraph::getFirstSkelNodeHit(float eyeX, float eyeY, float eyeZ, float lookX, float lookY, float lookZ, bool &isValid)
	{
		mvec rayOrigin = mvec(eyeX, eyeY, eyeZ);
		mvec rayDir = mvec(lookX, lookY, lookZ);

		bool hitFound = false;
		float minDist = 100000000;
		SkelVert vertHit = 0;

		skelVertIter svi = boost::vertices(mSkeleton);
		for (; svi.first != svi.second; ++svi)
		{
			Point3d *vert = &mSkeleton[*svi.first];

			mvec point = mvec(*vert);

			bool hits = false;
			float hitDist = 10000000;
			float scale = nodeOptions.endpointScale;
			

			pointPick(rayDir, rayOrigin, point, scale, hits, hitDist);

			if (hits && hitDist > 0)
			{
				if (hitDist < minDist)
				{
					hitFound = true;
					vertHit = *svi.first;
					minDist = hitDist;
				}
			}
		}
		isValid = hitFound;
		return vertHit;
	}

	std::pair<MetaV, MetaV> BMetaGraph::getFirstMetaEdgeHit(float eyeX, float eyeY, float eyeZ, float lookX, float lookY, float lookZ, bool &isValid)
	{


		mvec rayOrigin = mvec(eyeX, eyeY, eyeZ);
		mvec rayDir = mvec(lookX, lookY, lookZ);

		std::vector<float> rayToRayDists = {};
		std::vector<float> hitDists = {};
		std::vector<MetaE> correspondingEdges = {};

		float minRayToRaySoFar = 10000000;
		float minDistSoFar = 10000000;

		metaEdgeIter mei = boost::edges(*this);
		std::cout << "Iterating over edges " << std::endl;
		for (; mei.first != mei.second; ++mei)
		{
			BMetaEdge edge = operator[](*mei.first);
			float minRayToRayForEdge = 10000000;
			float minDistForEdge = -1;
			float rayToRayDist, hitDist;
			for (int vertI = 0; vertI < edge.mVertices.size() - 1; ++vertI)
			{
				mvec p0 = mvec(mSkeleton[edge.mVertices[vertI]]);
				mvec p1 = mvec(mSkeleton[edge.mVertices[vertI + 1]]);
				linePick(rayDir, rayOrigin, p0, p1, rayToRayDist, hitDist);
				if (rayToRayDist < minRayToRayForEdge && hitDist > 0)
				{
					minRayToRayForEdge = rayToRayDist;
					minDistForEdge = hitDist;
				}
			}
			if (minDistForEdge > 0)
			{
				bool doConsider = false;
				if (minRayToRayForEdge < minRayToRaySoFar)
				{
					minRayToRaySoFar = minRayToRayForEdge;
					doConsider = true;
				}
				//if the edge ray to ray is close to the min
				else if (10 * minRayToRayForEdge < minRayToRaySoFar)
				{
					doConsider = true;
				}
				if (doConsider)
				{
					if (minDistForEdge < minDistSoFar)
					{
						minDistSoFar = minDistForEdge;
					}
					if (minDistForEdge > 10 * minDistSoFar)
					{
						doConsider = false;
					}
				}
				if (doConsider)
				{
					std::cout << "Considering edge with ray to ray of " << minRayToRayForEdge << " and dist of " << minDistForEdge << std::endl;
					rayToRayDists.push_back(minRayToRayForEdge);
					hitDists.push_back(minDistForEdge);
					correspondingEdges.push_back(*mei.first);
				}
			}
		}
		std::cout << "Ending iteration over edges" << std::endl;

		if (rayToRayDists.size() <= 0)
		{
			isValid = false;
			
			return std::make_pair(MetaV(0), MetaV(0));
		}


		float bestPairRayToRaySoFar = rayToRayDists[0];
		float bestPairDistSoFar = hitDists[0];
		int bestPairSoFar = 0;
		
		for (int i = 1; i < rayToRayDists.size(); ++i)
		{
			
			float hitratio = hitDists[i] / bestPairDistSoFar;
			hitratio *= hitratio;
			if (rayToRayDists[i] < bestPairRayToRaySoFar / hitratio)
			{
				bestPairRayToRaySoFar = rayToRayDists[i];
				bestPairDistSoFar = hitDists[i];
				bestPairSoFar = i;
			}
			
		}
		

		if (bestPairRayToRaySoFar < 1)
		{
			isValid = true;
			return std::make_pair(MetaV(correspondingEdges[bestPairSoFar].m_source), MetaV(correspondingEdges[bestPairSoFar].m_target));
		}
		else
		{
			isValid = false;
			return std::make_pair(MetaV(0), MetaV(0));
		}
		
	}



	void BMetaGraph::setSelectionColor(float r, float g, float b)
	{
		selectionColor[0] = r;
		selectionColor[1] = g;
		selectionColor[2] = b;
		selectionColor[3] = 1.0;
	}
	void BMetaGraph::unselectAll()
	{
		selectNode1Valid = false;
		selectNode2Valid = false;
		//unselectAllEdges();
		if (breakEdgeValid)
		{
			unselectEdge(breakEdge);
		}
		breakEdgeValid = false;
		if (splitEdgeValid)
		{
			unselectEdge(splitEdge);
			for (std::pair<MetaV, MetaV> edge : splitNeighbors)
			{
				unselectEdge(edge);
			}
		}
		splitEdgeValid = false;
		
	}

	void BMetaGraph::unselectAllEdges()
	{
		metaVertIter mvi = boost::vertices(*this);

		for (; mvi.first != mvi.second; ++mvi)
		{
			BMetaGraph::adjacency_iterator adjIt, adjEnd;
			boost::tie(adjIt, adjEnd) = boost::adjacent_vertices(*mvi.first, *this);

			for (; adjIt != adjEnd; ++adjIt)
			{
				MetaV leadNode = *adjIt;
				unselectEdge(std::make_pair(*adjIt, *mvi.first));
			}
		}
	}

	void BMetaGraph::unselectEdge(std::pair<MetaV, MetaV> toUnselect)
	{
		MetaE edge;
		bool exists = false;
		boost::tie(edge, exists) = boost::edge(toUnselect.first, toUnselect.second, *this);
		if (exists)
		{
			operator[](edge).unselect(edgeColors);
		}
		
	}

	void BMetaGraph::selectEdge(std::pair<MetaV, MetaV> toSelect)
	{
		MetaE edge;
		bool exists = false;
		boost::tie(edge, exists) = boost::edge(toSelect.first, toSelect.second, *this);
		if (exists)
		{
			operator[](edge).select(edgeOptions.flatSelectionColor, edgeColors);
		}
	}

	void BMetaGraph::startRotation(int mx, int my)
	{
		arcball_start(mx, my);
	}

	void BMetaGraph::mouseMoved(int mx, int my)
	{
		arcball_move(mx, my);
	}

	void BMetaGraph::setZoom(float rad, float eyex, float eyey, float eyez, float upx, float upy, float upz)
	{
		vec eye = vec(eyex, eyey, eyez);
		vec up = vec(upx, upy, upz);

		arcball_setzoom(rad, eye, up);
	}

}
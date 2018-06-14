#include "BoostSkeleton.h"



namespace Roots
{
	std::string BSkeleton::beginSkeletonString = "begin_skeleton";
	std::string BSkeleton::endSkeletonString = "end_skeleton";
	std::string BSkeleton::vertexString = "vertex";
	std::string BSkeleton::edgeString = "edge";
	std::string BSkeleton::endHeaderString = "end_header";
	std::string BSkeleton::beginPlyString = "ply";


	BSkeleton::BSkeleton()
	{
		Initialize();
	}

	void BSkeleton::Initialize()
	{
		mMinX = 0, mMaxX = 0, mMinY = 0, mMaxY = 0, mMinZ = 0, mMaxZ = 0;
		mBoundsFound = false;
		mCenter = Point3d();
		mRadius = 0;
		clear();
	}

	int BSkeleton::loadFromLines(std::vector<std::string> lines, int startingLine, std::vector<GLfloat> &glVertices)
	{
		Initialize();
		mBoundsFound = false;
		bool endHeaderReached = false;
		std::string line;
		int numEdges = 0, numVerts = 0;
		line = lines[startingLine];

		int result = startingLine;
		if (!boost::iequals(line, beginSkeletonString))
		{
			//std::cout << "The Skeleton file lacks the " << beginSkeletonString << "header.  Should attempt to load wenzhen style file" << std::endl;
			result = loadWenzhenLines(lines, glVertices, startingLine);
		}
		else if (!boost::iequals(line, beginPlyString))
		{
			result = loadDanPly(lines, glVertices, startingLine);
		}
		else
		{
			//std::cout << "This is recognized as a PLY style file.  Attempting to read..." << std::endl;
			result = loadPlyStyleLines(lines, glVertices, startingLine);
		}

		/*for (skelEdgeIter sei = boost::edges(*this); sei.first != sei.second; ++sei)
		{
			std::cout << "Line between " << operator[](sei.first->m_source) << " and " << operator[](sei.first->m_target) << std::endl;
		}*/

		findBounds();
		findBoundingSphere();
		return result;
	}

	void BSkeleton::writeToStream(std::ostream & out)
	{

		out << beginSkeletonString << std::endl;
		out << vertexString << " " << m_vertices.size() << std::endl;
		out << edgeString << " " << m_edges.size() << std::endl;
		out << endHeaderString << std::endl;
		skelVertIter vi = boost::vertices(*this);
		while (vi.first != vi.second)
		{
			out << getVertData(*vi.first) << std::endl;
			++vi;
		}

		skelEdgeIter ei = boost::edges(*this);
		while (ei.first != ei.second)
		{
			out << getEdgeData(*ei.first) << std::endl;
			++ei;
		}
	}

	int BSkeleton::loadWenzhenLines(std::vector<std::string> lines, std::vector<GLfloat> &glVertices, int startingLine)
	{
		int lineOn = startingLine;
		
		int numEdges=0, numVertices=0, numFaces=0;
		std::vector<std::string> words = {};

		boost::split(words, lines[lineOn], boost::is_any_of(" "));
		++lineOn;
		if (words.size() < 2)
		{
			//std::cout << "The header information did not contain enough info. ";
			//std::cout << "Please ensure file indicates at least number of vertices and edges" << std::endl;
		}
		if (words.size() >= 2)
		{
			numVertices = boost::lexical_cast<int>(words[0]);
			//std::cout << "Number of vertices " << numVertices << std::endl;
			numEdges = boost::lexical_cast<int>(words[1]);
			//std::cout << "Number of edges " << numEdges << std::endl;
		}
		if (words.size() == 3)
		{
			//std::cout << "Faces listed as an input, but no behavior is currently defined for them" << std::endl;
			numFaces = boost::lexical_cast<int>(words[2]);
		}
		
		loadVertices(lines, lineOn, numVertices);
		loadEdges(lines, lineOn, numEdges, glVertices);
		if (lineOn >= lines.size())
		{
			std::cout << "Line on greater than lines size" << std::endl;
		}
		return lineOn;
	}

	int BSkeleton::loadDanPly(std::vector<std::string> lines, std::vector<GLfloat> &glVertices, int startingLine)
	{
		int lineOn = startingLine;

		int numEdges = 0, numVertices = 0, numFaces = 0;


	}

	int BSkeleton::loadPlyStyleLines(std::vector<std::string> lines, std::vector<GLfloat> &glVertices, int startingLine)
	{
		std::vector<std::vector<std::string>> headerInfo;
		bool endHeaderReached = false;
		int lineOn = startingLine;
		while (!endHeaderReached && lineOn < lines.size())
		{
			std::vector<std::string> words = {};
			boost::split(words, lines[lineOn], boost::is_any_of(" "));
			++lineOn;
			if (boost::iequals(words[0], endHeaderString))
			{
				endHeaderReached = true;
			}
			headerInfo.push_back(words);

		}
		if (lineOn >= lines.size())
		{
			//std::cout << "End of file reached before end of header, check file.  It is missing '" << endHeaderString << "' somewhere." << std::endl;
			return lineOn;
		}

		for (int headerLine = 0; headerLine < headerInfo.size(); ++headerLine)
		{
			std::vector<std::string> headerWords = headerInfo[headerLine];

			if (boost::iequals(headerWords[0], vertexString))
			{
				//handle vertices
				int numVerts = boost::lexical_cast<int>(headerWords[1]);
				loadVertices(lines, lineOn, numVerts);
				
			}
			else if (boost::iequals(headerWords[0], edgeString))
			{
				//handle edges
				int numEdges = boost::lexical_cast<int>(headerWords[1]);
				loadEdges(lines, lineOn, numEdges, glVertices);
			}
			else
			{
				//another kind of BSkeleton attribute (eg a face?), currently undefined behavior
				//std::cout << "The described Skeleton element " << headerWords[0] << " is not defined in this implementation." << std::endl;
				//std::cout << "No action will be taken for this element" << std::endl;
			}
		}
		return lineOn;
	}

	void BSkeleton::loadVertices(std::vector<std::string> lines, int &lineOn, int numVertices)
	{
		std::vector<std::string> words;
		float vertData[3];
		for (int i = 0; i < numVertices; ++i, ++lineOn)
		{
			words = {};
			boost::split(words, lines[lineOn], boost::is_any_of(" "));
			for (int axis = 0; axis < 3; ++axis)
			{
				vertData[axis] = boost::lexical_cast<float>(words[axis]);
			}
			//std::cout << "Vertex " << words[0] << " " << words[1] << " " << words[2] << std::endl;
			addVertex(Point3d(vertData));
			if (words.size() > 3)
			{
				//std::cout << "This line looks to be not a point" << std::endl;
				//std::cout << lines[lineOn] << std::endl;
			}
		}
		//std::cout << "Loaded all vertices, ending on line " << lineOn << std::endl;
		//std::cout << lines[lineOn] << std::endl;
	}

	void BSkeleton::loadEdges(std::vector<std::string> lines, int &lineOn, int numEdges, std::vector<GLfloat> &glVertices)
	{
		//std::cout << "Beginning to load edges" << std::endl;
		//std::cout << "first line is : " << lines[lineOn] << std::endl;
		int v0, v1;
		float attributedata[NumAttributes];
		std::vector<std::string> words;
		for (int i = 0; i < numEdges && lineOn < lines.size(); ++i, ++lineOn)
		{
			words = {};
			boost::split(words, lines[lineOn], boost::is_any_of(" "));
			v0 = boost::lexical_cast<int>(words[0]);
			v1 = boost::lexical_cast<int>(words[1]);
			for (int att = 0; att < NumAttributes; ++att)
			{
				attributedata[att] = boost::lexical_cast<float>(words[2 + att]);
			}
			if (words.size() < 2 + NumAttributes)
			{
				std::cout << "Edge: " << i << "at line: " << lineOn << " may be missing attribution" << std::endl;
			}
			//std::cout << "Edge between " << v0 << " " << v1 << std::endl;
			Point3d p0 = Point3d(), p1 = Point3d();
			if (m_vertices.size() > v1 && m_vertices.size() > v0)
			{
				SkelVert bv0 = boost::vertex(v0, *this);
				SkelVert bv1 = boost::vertex(v1, *this);
				p0 = operator[](bv0);
				p1 = operator[](bv1);
				addEdge(v0, v1, RootAttributes(attributedata, p0, p1), glVertices);
			}

		}
	}

	SkelEdge BSkeleton::addEdge(int v0, int v1, RootAttributes attributes, std::vector<GLfloat> &glVertices)
	{
		SkelEdge e;
		bool edgeAdded;
		boost::tie(e, edgeAdded) = boost::add_edge(v0, v1, *this);
		if (edgeAdded)
		{
			operator[](e) = attributes;
			operator[](e).v0id = v0;
			operator[](e).v1id = v1;
			operator[](e).glV0 = glVertices.size() / 3;
			for (int i = 0; i < 3; ++i)
			{
				glVertices.push_back(operator[](v0)[i]);
			}
			operator[](e).glV1 = glVertices.size() / 3;
			for (int i = 0; i < 3; ++i)
			{
				glVertices.push_back(operator[](v1)[i]);
			}

			//this causes a compiler error for some reason
			//boost::put(boost::edge_weight_t(), *this, e, attributes.euclidLength);
		}
		return e;
	}
	SkelVert BSkeleton::addVertex(Point3d pointLocation)
	{
		SkelVert v;
		v = boost::add_vertex(*this);
		operator[](v) = pointLocation;
		operator[](v).id = m_vertices.size() - 1;
		mBoundsFound = false;
		return v;
	}

	void BSkeleton::findBounds()
	{
		if (mBoundsFound)
		{
			return;
		}
		else
		{
			float big = 99999999999;
			mMinX = big;
			mMaxX = -big;
			mMinY = big;
			mMaxY = -big;
			mMinZ = big;
			mMaxZ = -big;

			Point3d p;
			skelVertIter it = boost::vertices(*this);
			while (it.first != it.second)
			{
				p = getVertData(*(it.first));
				mMinX = std::min(p[0], mMinX);
				mMinY = std::min(p[1], mMinY);
				mMinZ = std::min(p[2], mMinZ);
				mMaxX = std::max(p[0], mMaxX);
				mMaxY = std::max(p[1], mMaxY);
				mMaxZ = std::max(p[2], mMaxZ);
				++it;
			}
			mBoundsFound = true;
		}

	}
	void BSkeleton::findBoundingSphere()
	{
		findBounds();
		Point3d minPoint = Point3d(mMinX, mMinY, mMinZ);
		Point3d maxPoint = Point3d(mMaxX, mMaxY, mMaxZ);

		Point3d sum = Point3d();
		skelVertIter vIter = boost::vertices(*this);
		for(; vIter.first != vIter.second; ++vIter)
		{
			sum = sum + operator[](*vIter.first);
		}

		sum = sum / m_vertices.size();
		//Point3d sum = minPoint + maxPoint;
		mCenter = sum;

		mRadius = (maxPoint - mCenter).mag();
	}

	BSkeleton BSkeleton::recenterSkeleton(Point3d newCenter, std::vector<float> &vertices)
	{
		findBoundingSphere();
		Point3d offset = newCenter - mCenter;

		BSkeleton result = *this;
		skelVertIter resIter = boost::vertices(result);

		while (resIter.first != resIter.second)
		{
			result[*resIter.first] += offset;
			++resIter;
		}
		for (int i = 0; i < vertices.size() / 3; ++i)
		{
			for (int v = 0; v < 3; ++v)
			{
				vertices[i * 3 + v] += offset[v];
			}
		}
		return result;
	}

	SkelEdge BSkeleton::getEdge(int v0, int v1, bool &exists)
	{
		SkelEdge result;
		boost::tie(result, exists) = boost::edge(v0, v1, *this);
		return result;
	}

	RootAttributes& BSkeleton::getEdgeData(int v0, int v1, bool &exists)
	{
		SkelEdge temp = getEdge(v0, v1, exists);
		if (exists)
		{
			return getEdgeData(temp);
		}
		else
		{
			return RootAttributes();
		}
	}

	RootAttributes& BSkeleton::getEdgeData(SkelEdge e)
	{
		return operator[](e);
	}
	Point3d& BSkeleton::getVertData(SkelVert v)
	{
		return operator[](v);
	}

	std::vector<SkelVert> BSkeleton::GetNotableVertices()
	{
		std::vector<SkelVert> result = {};
		skelVertIter vi = boost::vertices(*this);
		while (vi.first != vi.second)
		{
			int deg = boost::degree(*vi.first, *this);
			if (deg != 2)
			{
				result.push_back(*vi.first);
			}
			++vi;
		}
		return result;
	}
}

PySkeleton::PySkeleton()
{
	mSkeleton = nullptr;
	minThickness = 0;
	maxThickness = 0;
	minWidth = 0;
	maxWidth = 0;
	minRatio = 0;
	maxRatio = 0;
	thicknessPercentiles = boost::python::list();
	widthPercentiles = boost::python::list();
	ratioPercentiles = boost::python::list();

}

PySkeleton::PySkeleton(Roots::BSkeleton *srcSkel)
{
	//std::cout << "setting skeleton to own skeleton " << std::endl;
	mSkeleton = srcSkel;
	reload();
}

void PySkeleton::findBoundingSphere()
{
	mSkeleton->findBoundingSphere();
}

void PySkeleton::reload()
{
	radius = 0;
	center = Point3d();
	mVertexList = boost::python::list();
	mEdgeList = boost::python::list();

	thicknessPercentiles = boost::python::list();
	widthPercentiles = boost::python::list();
	ratioPercentiles = boost::python::list();

	minThickness = 100000;
	maxThickness = 0;
	minWidth = 100000;
	maxWidth = 0;
	minRatio = 100000;
	maxRatio = 0;

	std::vector<float> thicknessList = {};
	std::vector<float> widthList = {};
	std::vector<float> ratioList = {};
	//std::cout << "reloading" << std::endl;
	if (mSkeleton == nullptr)
	{
		//std::cout << "Skeleton is nullptr" << std::endl;
		return;
	}
	
	for (skelVertIter svi = boost::vertices(*mSkeleton); svi.first != svi.second; ++svi)
	{
		Point3d vertexAdded = mSkeleton->operator[](*svi.first);
		//std::cout << "Vertex reloaded " << vertexAdded << std::endl;
		mVertexList.append<Point3d>(vertexAdded);
	}
	for (skelEdgeIter sei = boost::edges(*mSkeleton); sei.first != sei.second; ++sei)
	{
		Roots::RootAttributes rootAdded = mSkeleton->operator[](*sei.first);
		minThickness = std::min(rootAdded.thickness, minThickness);
		maxThickness = std::max(rootAdded.thickness, maxThickness);
		thicknessList.push_back(rootAdded.thickness);

		minWidth = std::min(rootAdded.width, minWidth);
		maxWidth = std::max(rootAdded.width, maxWidth);
		widthList.push_back(rootAdded.width);

		minRatio = std::min(rootAdded.thickness / rootAdded.width, minRatio);
		maxRatio = std::max(rootAdded.thickness / rootAdded.width, maxRatio);
		ratioList.push_back(rootAdded.thickness / rootAdded.width);

		//std::cout << "Root reloaded " << rootAdded << std::endl;
		mEdgeList.append<Roots::RootAttributes>(rootAdded);
	}

	std::sort(thicknessList.begin(), thicknessList.end());
	std::sort(widthList.begin(), widthList.end());
	std::sort(ratioList.begin(), ratioList.end());

	for (int i = 0; i < 11; ++i)
	{
		float fi = i;
		int idx = (fi / 10.0) * thicknessList.size();
		idx -= 1;
		idx = std::max(idx, 0);
		thicknessPercentiles.append<float>(thicknessList[idx]);
		widthPercentiles.append<float>(widthList[idx]);
		ratioPercentiles.append<float>(ratioList[idx]);
	}


	std::cout << "min thickness " << minThickness << " max thickness " << maxThickness << " min width " << minWidth << " max width " << maxWidth << " min ratio " << minRatio << " max ratio " << maxRatio << std::endl;
	radius = mSkeleton->mRadius;
	center = mSkeleton->mCenter;
}
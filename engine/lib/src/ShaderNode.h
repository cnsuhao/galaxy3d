#ifndef __ShaderNode_h__
#define __ShaderNode_h__

#include <string>
#include <vector>

namespace Galaxy3D
{
	struct ShaderNode
	{
		std::string type;
		std::string name;
		std::string block;
		std::vector<ShaderNode> children;
		ShaderNode *parent;
		int start;
	};
}

#endif
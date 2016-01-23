#include "UIAtlas.h"
#include "GTFile.h"
#include "json.h"

namespace Galaxy3D
{
    std::shared_ptr<UIAtlas> UIAtlas::LoadJsonFile(const std::string &file)
    {
        std::shared_ptr<UIAtlas> atlas;

        if(GTFile::Exist(file))
        {
            int size;
            char *bytes = (char *) GTFile::ReadAllBytes(file, &size);

            if(bytes != NULL)
            {
                Json::Reader reader;
                Json::Value root;

                if(reader.parse(bytes, bytes + size, root))
                {
                    auto frames = root["frames"];
                    auto meta = root["meta"];


                }

                free(bytes);
            }
        }

        return atlas;
    }
}
#include "ressource_compiler_armature.hpp"

#include "armature.hpp"

#include "tinyxml/tinyxml2.h"

using BoneDictionnary = std::vector<const char*>;

namespace ressource_compiler {

    void convertToMatrix(const tinyxml2::XMLElement& element, glm::mat4& mat)
    {
        assert(0 == strcmp(element.Value(), "Matrix4"));
        const char* matrix_text = element.GetText();
        float matrix[16];
        sscanf(matrix_text, "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f", &matrix[0], &matrix[1], &matrix[2], &matrix[3], &matrix[4], &matrix[5], &matrix[6], &matrix[7], &matrix[8], &matrix[9], &matrix[10], &matrix[11], &matrix[12], &matrix[13], &matrix[14], &matrix[15]);
        //mat = glm::mat4(matrix[0], matrix[1], matrix[2], matrix[3], matrix[4], matrix[5], matrix[6], matrix[7], matrix[8], matrix[9], matrix[10], matrix[11], matrix[12], matrix[13], matrix[14], matrix[15]);
        mat = glm::mat4(matrix[0], matrix[4], matrix[8], matrix[12], matrix[1], matrix[5], matrix[9], matrix[13], matrix[2], matrix[6], matrix[10], matrix[14], matrix[3], matrix[7], matrix[11], matrix[15]);
    }

    class Bones_visitor : public tinyxml2::XMLVisitor {
    public:
        Bones_visitor(Armature& armature, BoneDictionnary& bone_names)
            : mArmature(armature)
            , mBoneNames(bone_names)
        {}
        bool VisitEnter(const tinyxml2::XMLElement& element, const tinyxml2::XMLAttribute* /*firstAttribute*/) override {
            if (0 == strcmp( element.Value(), "Matrix4"))
            {
                Bone& bone = mArmature.bones.back();
                convertToMatrix(element, bone.transform);
                if (2 <= parents.size())
                {
                    bone.parent = parents[parents.size() -2];
                }
                else
                {
                    bone.parent = -1;
                }
                return true;
            }
            else if (0 == strcmp(element.Value(), "Node"))
            {
                parents.push_back(mArmature.bones.size());
                mArmature.bones.push_back(Bone());  
                mBoneNames.push_back(element.FindAttribute("name")->Value());
            }
            return true;
        }
        bool VisitExit(const tinyxml2::XMLElement& element) override {
            if(!parents.empty() && (0 == strcmp(element.Value(), "Node")))
                parents.pop_back();
            return true;
        }
    private:
        std::vector<uint> parents;
        Armature& mArmature;
        BoneDictionnary& mBoneNames;
    };

    void compile_armature(const char* filepath, Armature& armature, SkinMesh& skinMesh)
    {
        tinyxml2::XMLDocument doc;
        tinyxml2::XMLError error = doc.LoadFile(filepath);
        assert(!error);
        const tinyxml2::XMLElement* visualSceneElement = doc.FirstChildElement("ASSIMP")->FirstChildElement("Scene");
        const tinyxml2::XMLElement* meshList = visualSceneElement->FirstChildElement("MeshList");
        assert(1 == meshList->IntAttribute("num"));
        const tinyxml2::XMLElement* visualSceneNodesElement = visualSceneElement->FirstChildElement("Node")->FirstChildElement("NodeList");
        const tinyxml2::XMLElement* armatureElement = nullptr;
        for (const tinyxml2::XMLElement* child = visualSceneNodesElement->FirstChildElement(); child != NULL; child = child->NextSiblingElement())
        {
            if (child->Attribute("name", "Armature"))
            {
                armatureElement = child;
                break;
            }
        }
        assert(armatureElement);
        const tinyxml2::XMLElement* bonesElement = armatureElement->FirstChildElement("NodeList");
        assert(bonesElement);
        BoneDictionnary bones_dict;
        Bones_visitor bones_visitor(armature, bones_dict);
        bonesElement->Accept(&bones_visitor);
        const size_t bone_dict_size = bones_dict.size();
        const tinyxml2::XMLElement* animationElement = doc.FirstChildElement("ASSIMP")->FirstChildElement("Scene")->FirstChildElement("AnimationList");
        for (const tinyxml2::XMLElement* animation_node = animationElement->FirstChildElement(); animation_node != NULL; animation_node = animation_node->NextSiblingElement())
        {
            Animation animation;
            animation.bones_keyframes.resize(bone_dict_size);
            animation.duration = animation_node->FloatAttribute("duration");
            for (const tinyxml2::XMLElement* animation_bone_node = animation_node->FirstChildElement()->FirstChildElement(); animation_bone_node != NULL; animation_bone_node = animation_bone_node->NextSiblingElement())
            {
                const char* bone_name = animation_bone_node->FindAttribute("node")->Value();
                size_t bone_index = -1;
                for (size_t idx = 0; idx < bone_dict_size; ++idx)
                {
                    if (0 == strcmp(bones_dict[idx], bone_name))
                    {
                        bone_index = idx;
                        break;
                    }
                }
                assert(-1 != bone_index);
                BoneKeyFrames& boneKeyFrame = animation.bones_keyframes[bone_index];
                for (const tinyxml2::XMLElement* animation_bone_keyframe_list = animation_bone_node->FirstChildElement(); animation_bone_keyframe_list != NULL; animation_bone_keyframe_list = animation_bone_keyframe_list->NextSiblingElement())
                {
                    if (0 == strcmp(animation_bone_keyframe_list->Name(), "PositionKeyList"))
                    {
                        boneKeyFrame.translate.reserve(animation_bone_keyframe_list->IntAttribute("num"));
                        for (const tinyxml2::XMLElement* animation_bone_keyframe_node = animation_bone_keyframe_list->FirstChildElement(); animation_bone_keyframe_node != NULL; animation_bone_keyframe_node = animation_bone_keyframe_node->NextSiblingElement())
                        {
                            assert(0 == strcmp(animation_bone_keyframe_node->Name(), "PositionKey"));
                            TranslateKeyframe keyframe;
                            keyframe.timestamp = animation_bone_keyframe_node->FloatAttribute("time");
                            const char* value = animation_bone_keyframe_node->GetText();
                            sscanf(value, "%f %f %f", &keyframe.translate[0], &keyframe.translate[1], &keyframe.translate[2]);
                            boneKeyFrame.translate.push_back(keyframe);
                        }
                    }
                    else if (0 == strcmp(animation_bone_keyframe_list->Name(), "ScalingKeyList"))
                    {
                        boneKeyFrame.scale.reserve(animation_bone_keyframe_list->IntAttribute("num"));
                        for (const tinyxml2::XMLElement* animation_bone_keyframe_node = animation_bone_keyframe_list->FirstChildElement(); animation_bone_keyframe_node != NULL; animation_bone_keyframe_node = animation_bone_keyframe_node->NextSiblingElement())
                        {
                            assert(0 == strcmp(animation_bone_keyframe_node->Name(), "ScalingKey"));
                            ScaleKeyframe keyframe;
                            keyframe.timestamp = animation_bone_keyframe_node->FloatAttribute("time");
                            const char* value = animation_bone_keyframe_node->GetText();
                            sscanf(value, "%f %f %f", &keyframe.scale[0], &keyframe.scale[1], &keyframe.scale[2]);
                            boneKeyFrame.scale.push_back(keyframe);
                        }
                    }
                    else if (0 == strcmp(animation_bone_keyframe_list->Name(), "RotationKeyList"))
                    {
                        boneKeyFrame.rotation.reserve(animation_bone_keyframe_list->IntAttribute("num"));
                        for (const tinyxml2::XMLElement* animation_bone_keyframe_node = animation_bone_keyframe_list->FirstChildElement(); animation_bone_keyframe_node != NULL; animation_bone_keyframe_node = animation_bone_keyframe_node->NextSiblingElement())
                        {
                            assert(0 == strcmp(animation_bone_keyframe_node->Name(), "RotationKey"));
                            RotationKeyframe keyframe;
                            keyframe.timestamp = animation_bone_keyframe_node->FloatAttribute("time");
                            const char* value = animation_bone_keyframe_node->GetText();
                            sscanf(value, "%f %f %f %f", &keyframe.rotation[0], &keyframe.rotation[1], &keyframe.rotation[2], &keyframe.rotation[3]);
                            boneKeyFrame.rotation.push_back(keyframe);
                        }
                    }
                }
            }
            armature.animations.push_back(animation);
        }
        const tinyxml2::XMLElement* meshElement = meshList->FirstChildElement("Mesh");
        const tinyxml2::XMLElement* boneListElement = meshElement->FirstChildElement("BoneList");
        assert(boneListElement->IntAttribute("num") <= bone_dict_size);
        const tinyxml2::XMLElement* positionsElement = meshElement->FirstChildElement("Positions");
        const int vertexCount = positionsElement->IntAttribute("num");
        skinMesh.mVertexBone.resize(vertexCount);
        for (int vertexIdx = 0; vertexIdx < vertexCount; ++vertexIdx)
        {
            VertexBoneData& vertexBoneData = skinMesh.mVertexBone[vertexIdx];
            for (int boneIdx = 0; boneIdx < VertexBoneData::bonePerVertex; ++boneIdx)
            {
                vertexBoneData.index[boneIdx] = 0;
                vertexBoneData.weight[boneIdx] = 0.f;
            }
        }
        for (const tinyxml2::XMLElement* child = boneListElement->FirstChildElement(); child != NULL; child = child->NextSiblingElement())
        {  
            const char* boneName = child->Attribute("name");
            int boneId;
            for (boneId = 0; boneId < bone_dict_size; ++boneId)
            {
                if (0 == strcmp(boneName, bones_dict[boneId]))
                {
                    break;
                }
            }
            assert(boneId < bone_dict_size);
            Bone& bone = armature.bones[boneId];
            const tinyxml2::XMLElement* matrixElement = child->FirstChildElement("Matrix4");
            convertToMatrix(*matrixElement, bone.offset);
            const tinyxml2::XMLElement* weightListElement = child->FirstChildElement("WeightList");
            assert(weightListElement->IntAttribute("num") < vertexCount);
            for (const tinyxml2::XMLElement* weightElement = weightListElement->FirstChildElement(); weightElement != NULL; weightElement = weightElement->NextSiblingElement())
            {
                const int vertexId = weightElement->IntAttribute("index");
                const char* weightValueStr = weightElement->GetText();
                const float weight = static_cast<float>(atof(weightValueStr));
                assert(0.f < weight);
                assert(weight <= 1.f);
                VertexBoneData& vertexBoneData = skinMesh.mVertexBone[vertexId];
                int boneIdx;
                for (boneIdx = 0; boneIdx < VertexBoneData::bonePerVertex; ++boneIdx)
                {
                    if (0.f == vertexBoneData.weight[boneIdx])
                    {
                        break;
                    }
                }
                assert(boneIdx < VertexBoneData::bonePerVertex);
                vertexBoneData.index[boneIdx] = boneId;
                vertexBoneData.weight[boneIdx] = weight;
            }
        }
        for (int vertexIdx = 0; vertexIdx < vertexCount; ++vertexIdx)
        {
            VertexBoneData& vertexBoneData = skinMesh.mVertexBone[vertexIdx];
            float weightSum = 0.f;
            for (int boneIdx = 0; boneIdx < VertexBoneData::bonePerVertex; ++boneIdx)
            {
                weightSum += vertexBoneData.weight[boneIdx];
            }
            assert(glm::abs(weightSum - 1.f) < 0.05f);
        }
        skinMesh.mVertex.resize(vertexCount);
        {
            static const int bufferSize = 128;
            char buffer[bufferSize];
            const char* v = positionsElement->GetText();
            for(int vertexIdx = 0; vertexIdx <vertexCount; ++vertexIdx )
            {
                glm::vec3& vertex = skinMesh.mVertex[vertexIdx];
                for (int comp = 0; comp < 3; ++comp)
                {
                    int bufferIdx;
                    while (isspace(*v))
                    {
                        ++v;
                    }
                    for (bufferIdx = 0; bufferIdx < bufferSize; ++bufferIdx)
                    {
                        if (isspace(*v))
                        {
                            buffer[bufferIdx] = '\0';
                            break;
                        }
                        buffer[bufferIdx] = *v;
                        ++v;
                    }
                    assert(bufferIdx < bufferSize);
                    vertex[comp] = static_cast<float>(atof(buffer));
                }
            }
        }
        const tinyxml2::XMLElement* normalsElement = meshElement->FirstChildElement("Normals");
        assert(vertexCount == normalsElement->IntAttribute("num"));
        skinMesh.mNormal.resize(vertexCount);
        {
            static const int bufferSize = 128;
            char buffer[bufferSize];
            const char* v = normalsElement->GetText();
            for (int vertexIdx = 0; vertexIdx <vertexCount; ++vertexIdx)
            {
                glm::vec3& normal = skinMesh.mNormal[vertexIdx];
                for (int comp = 0; comp < 3; ++comp)
                {
                    int bufferIdx;
                    while (isspace(*v))
                    {
                        ++v;
                    }
                    for (bufferIdx = 0; bufferIdx < bufferSize; ++bufferIdx)
                    {
                        if (isspace(*v))
                        {
                            buffer[bufferIdx] = '\0';
                            break;
                        }
                        buffer[bufferIdx] = *v;
                        ++v;
                    }
                    assert(bufferIdx < bufferSize);
                    normal[comp] = atof(buffer);
                }
                assert(glm::abs(glm::length(normal) - 1.f) < 0.05f);
            }
        }
        const tinyxml2::XMLElement* textureCoordElement = meshElement->FirstChildElement("TextureCoords");
        assert(vertexCount == textureCoordElement->IntAttribute("num"));
        skinMesh.mTextureCoord.resize(vertexCount);
        {
            static const int bufferSize = 128;
            char buffer[bufferSize];
            const char* v = textureCoordElement->GetText();
            for (int vertexIdx = 0; vertexIdx <vertexCount; ++vertexIdx)
            {
                glm::vec2& uv = skinMesh.mTextureCoord[vertexIdx];
                for (int comp = 0; comp < 2; ++comp)
                {
                    int bufferIdx;
                    while (isspace(*v))
                    {
                        ++v;
                    }
                    for (bufferIdx = 0; bufferIdx < bufferSize; ++bufferIdx)
                    {
                        if (isspace(*v))
                        {
                            buffer[bufferIdx] = '\0';
                            break;
                        }
                        buffer[bufferIdx] = *v;
                        ++v;
                    }
                    assert(bufferIdx < bufferSize);
                    uv[comp] = atof(buffer);
                }
            }
        }
        const tinyxml2::XMLElement* faceListElement = meshElement->FirstChildElement("FaceList");
        skinMesh.mIndex.reserve(faceListElement->IntAttribute("num")*3);
        for (const tinyxml2::XMLElement* faceElement = faceListElement->FirstChildElement(); faceElement != nullptr; faceElement = faceElement->NextSiblingElement())
        {
            const int vertexPerFace = 3;
            assert(vertexPerFace == faceElement->IntAttribute("num"));
            glm::ivec3 face;
            sscanf(faceElement->GetText(),"%d %d %d", &face[0], &face[1], &face[2]);
            for (int vertexPerFaceIdx = 0; vertexPerFaceIdx < vertexPerFace; ++vertexPerFaceIdx)
            {
                assert(face[vertexPerFaceIdx] < vertexCount);
                skinMesh.mIndex.push_back(face[vertexPerFaceIdx]);
            }
        }
    }
};

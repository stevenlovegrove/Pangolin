#ifdef HAVE_ASSIMP
#include <assimp/Exporter.hpp>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#endif

#include <pangolin/drawable/drawn_group.h>
#include <pangolin/drawable/drawn_primitives.h>
#include <pangolin/image/image_io.h>

#include <filesystem>

namespace pangolin
{

struct MeshData {
  Eigen::Matrix<uint32_t, 3, Eigen::Dynamic> indices;
  Eigen::Matrix<float, 3, Eigen::Dynamic> vertices;
  Eigen::Matrix<float, 3, Eigen::Dynamic> colors;
  Eigen::Matrix<float, 3, Eigen::Dynamic> normals;
  Eigen::Matrix<float, 3, Eigen::Dynamic> uvs;
  Eigen::Vector4d default_color = {1.0, 1.0, 1.0, 1.0};
  sophus2::IntensityImage<> texture_color;
};

Shared<DrawnPrimitives> toDrawnPrimitives(MeshData& mesh)
{
  auto prim = DrawnPrimitives::Create({});
  prim->default_color = mesh.default_color;
  prim->element_type = DrawnPrimitives::Type::triangles;

  if (!mesh.texture_color.isEmpty()) {
    prim->geometry_texture->update(mesh.texture_color);
    mesh.texture_color = sophus2::IntensityImage<>();
  }

  if (mesh.vertices.size()) {
    std::vector<Eigen::Vector3f> test;
    prim->vertices->queueUpdate(std::move(mesh.vertices));
  }

  if (mesh.indices.size()) {
    prim->indices->queueUpdate(std::move(mesh.indices));
  }

  if (mesh.colors.size()) {
    prim->colors->queueUpdate(std::move(mesh.colors));
  }

  if (mesh.normals.size()) {
    prim->normals->queueUpdate(std::move(mesh.normals));
  }

  if (mesh.uvs.size()) {
    prim->uvs->queueUpdate(std::move(mesh.uvs));
  }
  return prim;
}

#ifdef HAVE_ASSIMP
Shared<DrawnGroup> loadMesh(
    const aiScene* ai_scene, const std::filesystem::path& root_path)
{
  using Mat3X = Eigen::Matrix<float, 3, Eigen::Dynamic>;

  auto group = Shared<DrawnGroup>::make();

  for (uint64_t i = 0; i < ai_scene->mNumMeshes; ++i) {
    aiMesh* ai_mesh = ai_scene->mMeshes[i];

    MeshData mesh;
    mesh.vertices =
        Eigen::Map<Mat3X>(&ai_mesh->mVertices->x, 3, ai_mesh->mNumVertices);
    if (ai_mesh->HasTextureCoords(0)) {
      mesh.uvs = Eigen::Map<Mat3X>(
          &ai_mesh->mTextureCoords[0]->x, 3, ai_mesh->mNumVertices);
    }
    if (ai_mesh->HasNormals()) {
      mesh.normals =
          Eigen::Map<Mat3X>(&ai_mesh->mNormals->x, 3, ai_mesh->mNumVertices);
    }
    if (ai_mesh->HasVertexColors(0)) {
      mesh.colors =
          Eigen::Map<Mat3X>(&ai_mesh->mColors[0]->r, 3, ai_mesh->mNumVertices);
    }

    mesh.indices.resize(3, ai_mesh->mNumFaces);

    for (uint64_t i = 0; i < ai_mesh->mNumFaces; ++i) {
      FARM_ASSERT_EQ(ai_mesh->mFaces[i].mNumIndices, 3);
      mesh.indices.col(i) = Eigen::Matrix<uint32_t, 3, 1>(
          ai_mesh->mFaces[i].mIndices[0], ai_mesh->mFaces[i].mIndices[1],
          ai_mesh->mFaces[i].mIndices[2]);
    }

    PANGO_ENSURE(ai_mesh->mMaterialIndex < ai_scene->mNumMaterials);
    aiMaterial* ai_material = ai_scene->mMaterials[ai_mesh->mMaterialIndex];

    aiColor4D ambient_color;
    if (AI_SUCCESS ==
        ai_material->Get(AI_MATKEY_COLOR_AMBIENT, ambient_color)) {
      mesh.default_color = Eigen::Vector4d(
          ambient_color.r, ambient_color.g, ambient_color.b, ambient_color.a);
    }

    const int num_textures =
        ai_material->GetTextureCount(aiTextureType_DIFFUSE);
    if (num_textures > 0) {
      aiString texture_relative_path;
      if (AI_SUCCESS == ai_material->GetTexture(
                            aiTextureType_DIFFUSE, 0, &texture_relative_path)) {
        std::string fix_texture_path(texture_relative_path.C_Str());
        std::replace(
            fix_texture_path.begin(), fix_texture_path.end(), '\\', '/');
        const auto tex_filename = root_path / fix_texture_path;
        mesh.texture_color = pangolin::LoadImage(tex_filename);
      }
    }

    group->children.push_back(toDrawnPrimitives(mesh));
  }

  return group;
}
#endif  // HAVE_ASSIMP

Shared<DrawnGroup> DrawnGroup::Load(
    const std::filesystem::path& filename, const LoadParams& params)
{
#ifdef HAVE_ASSIMP

  Assimp::Importer importer;
  // importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_NORMALS);

  const aiScene* ai_scene = importer.ReadFile(
      filename.c_str(), aiProcess_Triangulate | aiProcess_SortByPType |
                            aiProcess_JoinIdenticalVertices |
                            aiProcess_GenSmoothNormals /* |
                            aiProcess_RemoveComponent */);
  PANGO_ENSURE(ai_scene, "{}: {}", filename, importer.GetErrorString());
  return loadMesh(ai_scene, filename.parent_path());
#else
  PANGO_FATAL("Please compile with ASSIMP support for mesh loading.");
#endif  // HAVE_ASSIMP
}

Shared<DrawnGroup> DrawnGroup::Load(
    const void* data, size_t num_bytes, const LoadParams& params)
{
#ifdef HAVE_ASSIMP

  Assimp::Importer importer;
  // importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_NORMALS);

  const aiScene* ai_scene = importer.ReadFileFromMemory(data, num_bytes,
      aiProcess_Triangulate | aiProcess_SortByPType |
                            aiProcess_JoinIdenticalVertices |
                            aiProcess_GenSmoothNormals /* |
                            aiProcess_RemoveComponent */);
  PANGO_ENSURE(ai_scene, "{}", importer.GetErrorString());
  return loadMesh(ai_scene, ".");
#else
  PANGO_FATAL("Please compile with ASSIMP support for mesh loading.");
#endif  // HAVE_ASSIMP}
}

Shared<DrawnGroup> DrawnGroup::Create(const DrawnGroup::Params& p)
{
  auto ret = Shared<DrawnGroup>::make();
  ret->children = p.children;
  return ret;
}

}  // namespace pangolin

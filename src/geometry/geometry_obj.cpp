/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2011 Steven Lovegrove
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include <pangolin/geometry/geometry_obj.h>
#include <tinyobj/tiny_obj_loader.h>
#include <pangolin/utils/variadic_all.h>
#include <pangolin/utils/simple_math.h>

// Needed for enum type. TODO: Make this independent of GL
#include <pangolin/gl/glplatform.h>
#include <pangolin/image/image_io.h>
#include <pangolin/utils/file_utils.h>

namespace pangolin {

template<typename T>
Geometry::Element ConvertVectorToGeometryElement(const std::vector<T>& v, size_t count_per_element, const std::string& attribute_name )
{
    PANGO_ASSERT(v.size() % count_per_element == 0);
    const size_t num_elements = v.size() / count_per_element;

    // Allocate buffer and copy into it
    Geometry::Element el(sizeof(T) * count_per_element, num_elements);
    el.CopyFrom(Image<T>(v.data(), count_per_element, num_elements));
    el.attributes[attribute_name] = el.template UnsafeReinterpret<T>();
    return el;
}

template<typename T>
Image<T> GetImageWrapper(std::vector<T>& vec, size_t count_per_element)
{
    PANGO_ASSERT(vec.size() % count_per_element == 0);
    if(vec.size()) {
        return Image<T>(vec.data(), count_per_element, vec.size() / count_per_element, count_per_element * sizeof(T));
    }else{
        return Image<T>();
    }
}


pangolin::Geometry LoadGeometryObj(const std::string& filename)
{
    pangolin::Geometry geom;

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::string warn;
    std::string err;
    if(tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename.c_str(), PathParent(filename).c_str())) {
        PANGO_ASSERT(attrib.vertices.size() % 3 == 0);
        PANGO_ASSERT(attrib.normals.size() % 3 == 0);
        PANGO_ASSERT(attrib.colors.size() % 3 == 0);
        PANGO_ASSERT(attrib.texcoords.size() % 2 == 0);

        // Load textures - a bit of a hack for now.
        for(size_t i=0; i < materials.size(); ++i) {
            if(!materials[i].diffuse_texname.empty()) {
                geom.textures[FormatString("texture_%",i)] = LoadImage(PathParent(filename) + "/" + materials[i].diffuse_texname);
            }
        }

        const size_t num_verts = attrib.vertices.size() / 3;
//        PANGO_ASSERT(all_of(
//            [&](const size_t& v){return (v == 0) || (v == num_verts);},
//            attrib.normals.size() / 3,
//            attrib.colors.size() / 3));

        // Get rid of color buffer if all elements are equal.
        if ( std::adjacent_find( attrib.colors.begin(), attrib.colors.end(), std::not_equal_to<float>() ) == attrib.colors.end() )
        {
            attrib.colors.clear();
        }

        Image<float> tiny_vs = GetImageWrapper(attrib.vertices, 3);
        Image<float> tiny_ns = GetImageWrapper(attrib.normals, 3);
        Image<float> tiny_cs = GetImageWrapper(attrib.colors, 3);
        Image<float> tinu_ts = GetImageWrapper(attrib.texcoords, 2);


        // Create unified verts attribute
        auto& verts = geom.buffers["geometry"];
        Image<float> new_vs, new_ns, new_cs, new_ts;
        {
            verts.Reinitialise(sizeof(float)*(tiny_vs.w + tiny_ns.w + tiny_cs.w + tinu_ts.w), num_verts);
            size_t float_offset = 0;
            if(tiny_vs.IsValid()) {
                new_vs = verts.UnsafeReinterpret<float>().SubImage(float_offset,0,3,num_verts);
                verts.attributes["vertex"] = new_vs;
                float_offset += 3;
                new_vs.CopyFrom(tiny_vs);
            }
            if(tiny_ns.IsValid()) {
                new_ns = verts.UnsafeReinterpret<float>().SubImage(float_offset,0,3,num_verts);
                verts.attributes["normal"] = new_ns;
                float_offset += 3;
                // Don't copy - we'll have to re-order
            }
            if(tiny_cs.IsValid()) {
                new_cs = verts.UnsafeReinterpret<float>().SubImage(float_offset,0,3,num_verts);
                verts.attributes["color"] = new_cs;
                float_offset += 3;
                new_cs.CopyFrom(tiny_cs);
            }
            if(tinu_ts.IsValid()) {
                new_ts = verts.UnsafeReinterpret<float>().SubImage(float_offset,0,3,num_verts);
                verts.attributes["uv"] = new_ts;
                float_offset += 2;
                // Don't copy - we'll have to re-order
            }
            PANGO_ASSERT(float_offset * sizeof(float) == verts.w);
        }

        for(auto& shape : shapes) {
            if(shape.mesh.indices.size() == 0) {
                continue;
            }

            auto faces = geom.objects.emplace(shape.name, Geometry::Element());

            if(std::all_of( shape.mesh.num_face_vertices.begin(), shape.mesh.num_face_vertices.end(),
                [](unsigned char num){return num==3;}
            )) {
                // tri mesh
                const size_t num_indices = shape.mesh.indices.size() ;
                const size_t num_faces = shape.mesh.indices.size() / 3;
                PANGO_ASSERT(num_indices % 3 == 0);

                Image<uint32_t> ibo_vs((uint32_t*)&shape.mesh.indices[0].vertex_index, 1, num_indices, sizeof(tinyobj::index_t));
                Image<uint32_t> ibo_ns((uint32_t*)&shape.mesh.indices[0].normal_index, 1, num_indices, sizeof(tinyobj::index_t));
                Image<uint32_t> ibo_ts((uint32_t*)&shape.mesh.indices[0].texcoord_index, 1, num_indices, sizeof(tinyobj::index_t));

                // Use vert_ibo as our new IBO
                faces->second.Reinitialise(3*sizeof(uint32_t), num_indices);
                Image<uint32_t> new_ibo = faces->second.UnsafeReinterpret<uint32_t>().SubImage(0,0,3,num_faces);
                for(size_t f=0; f < num_faces; ++f) {
                    for(size_t v=0; v < 3; ++v) {
                        new_ibo(v,f) = ibo_vs(0,3*f+v);
                    }
                }
                faces->second.attributes["vertex_indices"] = new_ibo;

                // Reorder normals
                if(new_ns.IsValid()) {
                    for(size_t f=0; f < num_faces; ++f) {
                        for(size_t v=0; v < 3; ++v) {
                            size_t vi = ibo_vs(0,3*f+v);
                            size_t ni = ibo_ns(0,3*f+v);
                            new_ns.Row(vi).CopyFrom(tiny_ns.Row(ni));
                            Normalise<3>(new_ns.RowPtr(vi));
                        }
                    }
                }

                // Reorder uvs
                if(new_ts.IsValid()) {
                    for(size_t f=0; f < num_faces; ++f) {
                        for(size_t v=0; v < 3; ++v) {
                            size_t vi = ibo_vs(0,3*f+v);
                            size_t ti = ibo_ts(0,3*f+v);
                            new_ts.Row(vi).CopyFrom(tinu_ts.Row(ti));
                        }
                    }
                }
            }else if(std::all_of( shape.mesh.num_face_vertices.begin(), shape.mesh.num_face_vertices.end(),
                [](unsigned char num){return num==4;}
            )) {
                // Quad mesh
                throw std::runtime_error("Do not support quad meshes yet.");
            }else{
                // ???
                throw std::runtime_error("Do not support meshes with mixed triangles and quads.");
            }
        }
    }else{
        throw std::runtime_error(FormatString("Unable to load OBJ file '%'. Error: '%'", filename, err));
    }

    return geom;
}

}

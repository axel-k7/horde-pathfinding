#pragma once

#include "godot_cpp/classes/multi_mesh.hpp"
#include "godot_cpp/classes/rendering_server.hpp"

#include "ecs/core/System.h"

#include "ecs/components/MeshComponent.h"
#include "ecs/components/TransformComponent.h"

using namespace godot;

class RenderingSystem : public System {
public:
    RenderingSystem(std::shared_ptr<Registry> _registry, std::shared_ptr<EntityCommandBuffer> _buffer, RID _world_context)
        : System(_registry, _buffer)
        , query(_registry->query<MeshComponent, TransformComponent>())
        , world_rid(_world_context)
    {}

    Registry::QueryResult<MeshComponent, TransformComponent> query;

    //should honestly remake later this but with manual chunking so separate lookups arent needed for every entity
    //store multimesh id in meshcomponent?
    void update(const float& /*_dt*/) override {
        if (!world_rid.is_valid())
            return;

        RenderingServer* renderer = RenderingServer::get_singleton();

        for (auto [entity, mesh_data, transform_data] : query) {
            if (mesh_data.bucket_index == -1) { 
                //maybe add an "unitialized" list to loop over before actually updating entities?
                //"onQueryUpdate?"
                BucketKey key = {mesh_data.mesh_rid, mesh_data.material_rid};
                MeshBucket& bucket = ensureBucket(key);
                
                mesh_data.bucket_index = addInstance(bucket, entity);
                mesh_data.multimesh_rid = bucket.multimesh_rid;
            }

            renderer->multimesh_instance_set_transform(mesh_data.multimesh_rid, mesh_data.bucket_index, transform_data.transform);
        }
    }

    void setWorldContext(RID _world_context) { world_rid = _world_context; }

    void removeInstance(MeshComponent& _mesh) {
        BucketKey key = {_mesh.mesh_rid, _mesh.material_rid};

        auto it = buckets.find(key);
        if (it == buckets.end() || _mesh.bucket_index == -1)
            return;

        RenderingServer* renderer = RenderingServer::get_singleton();
        MeshBucket& bucket = it->second;

        int to_remove = _mesh.bucket_index;
        int last = bucket.count - 1;

        if (_mesh.bucket_index != last) {
            Entity last_entity = bucket.entities[last];

            Transform3D last_transform = renderer->multimesh_instance_get_transform(bucket.multimesh_rid, last);
            renderer->multimesh_instance_set_transform(bucket.multimesh_rid, to_remove, last_transform);

            auto last_mesh = registry->tryGetComponent<MeshComponent>(last_entity);
            if (last_mesh)
                last_mesh->bucket_index = to_remove;

            bucket.entities[to_remove] = last_entity;
        }

        bucket.count--;
        _mesh.bucket_index = -1;

        renderer->multimesh_set_visible_instances(bucket.multimesh_rid, bucket.count);
    }

private:
    struct MeshBucket {
        std::vector<Entity> entities;

        RID multimesh_rid;
        RID instance_rid;

        size_t count = 0;
    };

    struct BucketKey {
        RID mesh;
        RID material;
        bool operator==(const BucketKey& _other) const {
            return mesh == _other.mesh && material == _other.material;
        }
    };

    struct KeyHasher {
        size_t operator()(const BucketKey& _key) const {
            size_t mesh_hash     = std::hash<int64_t>{}(_key.mesh.get_id());
            size_t material_hash = std::hash<int64_t>{}(_key.material.get_id());
            //stole this lol
            return mesh_hash ^ (material_hash + 0x9e3779b9 + (mesh_hash << 6) + (mesh_hash >> 2));
        }
    };

    std::unordered_map<BucketKey, MeshBucket, KeyHasher> buckets;
    RID world_rid;

    //mesh type id, new mesh = new id
    uint32_t next_id = 0;

    auto ensureBucket(BucketKey _key) -> MeshBucket& {
        auto it = buckets.find(_key);

        if (it != buckets.end())
            return it->second;

        MeshBucket bucket;
        RenderingServer* renderer = RenderingServer::get_singleton();

        bucket.multimesh_rid = renderer->multimesh_create();
        renderer->multimesh_set_mesh(bucket.multimesh_rid, _key.mesh);
        renderer->multimesh_allocate_data(bucket.multimesh_rid, 100, RenderingServer::MULTIMESH_TRANSFORM_3D); //currently setting capacity to 100, need to change this later

        bucket.instance_rid = renderer->instance_create();
        renderer->instance_set_base(bucket.instance_rid, bucket.multimesh_rid);
        renderer->instance_set_scenario(bucket.instance_rid, world_rid);

        auto result = buckets.emplace(_key, std::move(bucket));
        return result.first->second;
    }

    auto addInstance(MeshBucket& _bucket, Entity _entity) -> int {
        int index = _bucket.count;
        _bucket.count++;

        if (_bucket.count > _bucket.entities.size())
            _bucket.entities.resize(_bucket.count);

        _bucket.entities[index] = _entity;

        RenderingServer::get_singleton()->multimesh_set_visible_instances(_bucket.multimesh_rid, _bucket.count);

        return index;
    }

};

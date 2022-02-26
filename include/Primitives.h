#pragma once
#include <precomp.h>
#include <General.h>

struct VertexDescription {
    VkVertexInputBindingDescription bindingDescription;
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
};

struct Vertex2D4C {
    glm::vec2 pos;
    glm::vec4 color;

    static VkVertexInputBindingDescription getBindingDescription(uint32_t binding=0) {
        return {
            .binding = binding,
            .stride = sizeof(Vertex2D4C),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
        };
    }

    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions(uint32_t binding=0) {
        return {
            VkVertexInputAttributeDescription {
                .location = 0,
                .binding = binding,
                .format = VK_FORMAT_R32G32_SFLOAT,
                .offset = offsetof(Vertex2D4C, pos),
            },
            VkVertexInputAttributeDescription {
                .location = 1,
                .binding = binding,
                .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                .offset = offsetof(Vertex2D4C, color),
            }
        };
    }

    static VertexDescription getVertexDescription(uint32_t binding=0) {
        return {
            .bindingDescription = getBindingDescription(binding),
            .attributeDescriptions = getAttributeDescriptions(binding),
        };
    }
};

template<>
struct RandGen<Vertex2D4C> {
    Vertex2D4C operator() () const {
        return { { randf()*2-1, randf()*2-1 }, { randf(), randf(), randf(), randf() } };
    }
};

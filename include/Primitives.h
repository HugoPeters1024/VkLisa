#pragma once
#include <precomp.h>
#include <General.h>

struct VertexDescription {
    VkVertexInputBindingDescription bindingDescription;
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
};

struct Vertex {
    glm::vec4 pos;
    glm::vec4 color;

    static VkVertexInputBindingDescription getBindingDescription(uint32_t binding=0) {
        return {
            .binding = binding,
            .stride = sizeof(Vertex),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
        };
    }

    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions(uint32_t binding=0) {
        return {
            VkVertexInputAttributeDescription {
                .location = 0,
                .binding = binding,
                .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                .offset = offsetof(Vertex, pos),
            },
            VkVertexInputAttributeDescription {
                .location = 1,
                .binding = binding,
                .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                .offset = offsetof(Vertex, color),
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
struct RandGen<Vertex> {
    Vertex operator() () const {
        return { { randf(0.5f)-1, randf(0.5f)-1, 0, 0 }, { randf(0.1f), randf(0.1f), randf(0.1f), randf(0.1f)+0.5f } };
    }
};

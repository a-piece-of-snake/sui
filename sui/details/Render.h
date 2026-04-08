#pragma once
#include <SDL3/SDL.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

inline auto toFColor(SDL_Color c) -> SDL_FColor {
    return {static_cast<float>(c.r) / 255.F, static_cast<float>(c.g) / 255.F,
            static_cast<float>(c.b) / 255.F, static_cast<float>(c.a) / 255.F};
}

void generateRoundedPath(float x, float y, float w, float h, float r, SDL_FColor color,
                         std::vector<SDL_Vertex>& outVertices, int segments = 8) {
    if (w <= 0.F || h <= 0.F) {
        return;
    }

    float maxR = std::max(0.F, std::min(w, h) / 2.F);

    r = std::clamp(r, 0.F, maxR);
    struct Corner {
        float m_cx, m_cy, m_angle;
    };
    std::array<Corner, 4> corners = {
        {{.m_cx = x + w - r, .m_cy = y + r, .m_angle = std::numbers::pi_v<float> * 1.5F},
         {.m_cx = x + w - r, .m_cy = y + h - r, .m_angle = 0.F},
         {.m_cx = x + r, .m_cy = y + h - r, .m_angle = 0.5F * std::numbers::pi_v<float>},
         {.m_cx = x + r, .m_cy = y + r, .m_angle = 1.F * std::numbers::pi_v<float>}}};
    for (const auto& c : corners) {
        for (int j = 0; j <= segments; ++j) {
            float theta = c.m_angle + ((static_cast<float>(j) / static_cast<float>(segments)) *
                                       (std::numbers::pi_v<float> / 2.0F));
            outVertices.push_back(
                {{c.m_cx + (r * std::cos(theta)), c.m_cy + (r * std::sin(theta))}, color, {0, 0}});
        }
    }
}

void drawBorder(SDL_Renderer* renderer, float x, float y, float w, float h, float r,
                float thickness, SDL_Color c) {
    float maxThickness = std::min(w, h) / 2.F;
    if (thickness <= 0.F || thickness >= maxThickness || w <= 0.F || h <= 0.F) {
        return;
    }
    std::vector<SDL_Vertex> outer;
    std::vector<SDL_Vertex> inner;
    SDL_FColor fColor = toFColor(c);

    generateRoundedPath(x, y, w, h, r, fColor, outer);
    generateRoundedPath(x + thickness, y + thickness, w - (thickness * 2), h - (thickness * 2),
                        std::max(0.F, r - thickness), fColor, inner);

    std::vector<SDL_Vertex> combined = outer;
    combined.insert(combined.end(), inner.begin(), inner.end());

    std::vector<int> indices;
    int count = static_cast<int>(outer.size());
    for (int i = 0; i < count; ++i) {
        int next = (i + 1) % count;
        indices.push_back(i);
        indices.push_back(next);
        indices.push_back(i + count);
        indices.push_back(next);
        indices.push_back(next + count);
        indices.push_back(i + count);
    }

    SDL_RenderGeometry(renderer, nullptr, combined.data(), static_cast<int>(combined.size()),
                       indices.data(), static_cast<int>(indices.size()));
}
void roundedRect(float x, float y, float w, float h, float r, SDL_Color c,
                 std::vector<SDL_Vertex>* outVertices, int segments = 8) {
    if (outVertices == nullptr) {
        return;
    }

    r = std::clamp(r, 0.F, std::min(w, h) / 2.F);
    SDL_FColor color = toFColor(c);

    outVertices->push_back({{x + (w / 2.F), y + (h / 2.F)}, color, {0, 0}});

    generateRoundedPath(x, y, w, h, r, color, *outVertices, segments);

    if (outVertices->size() > 1) {
        outVertices->push_back((*outVertices)[1]);
    }
}
void shadowRect(float x, float y, float w, float h, float r, float shadowSize, SDL_Color shadowFill,
                std::vector<SDL_Vertex>* outVertices, int segments = 16) {
    if ((outVertices == nullptr) || w <= 0.F || h <= 0.F) {
        return;
    }

    float maxR = std::max(0.F, std::min(w, h) / 2.F);
    r = std::clamp(r, 0.F, maxR);

    SDL_FColor innerColor = toFColor(shadowFill);
    SDL_FColor outerColor = innerColor;
    outerColor.a = 0.F;

    outVertices->push_back({{x + (w / 2.F), y + (h / 2.F)}, innerColor, {0, 0}});

    generateRoundedPath(x, y, w, h, r, innerColor, *outVertices, segments);
    int innerPathStart = 1;
    int pathPointCount = static_cast<int>(outVertices->size()) - innerPathStart;

    generateRoundedPath(x - shadowSize, y - shadowSize, w + (shadowSize * 2.F),
                        h + (shadowSize * 2.F), r + shadowSize, outerColor, *outVertices, segments);
}
void drawShadow(SDL_Renderer* renderer, float x, float y, float w, float h, float r,
                float shadowSize, SDL_Color shadowFill) {

    if (w <= 0.F || h <= 0.F || shadowFill.a == 0) {
        return;
    }

    std::vector<SDL_Vertex> verts;
    shadowRect(x, y, w, h, r, shadowSize, shadowFill, &verts);

    if (verts.size() < 10) {
        return;
    }

    int pathCount = (static_cast<int>(verts.size()) - 1) / 2;

    if (pathCount <= 0) {
        return;
    }

    std::vector<int> indices;

    for (int i = 1; i < pathCount; ++i) {
        indices.push_back(0);
        indices.push_back(i);
        indices.push_back(i + 1);
    }
    indices.push_back(0);
    indices.push_back(pathCount);
    indices.push_back(1);

    for (int i = 1; i <= pathCount; ++i) {
        int next = (i % pathCount) + 1;
        int inner = i;
        int outer = i + pathCount;
        int nextInner = next;
        int nextOuter = next + pathCount;

        indices.push_back(inner);
        indices.push_back(nextInner);
        indices.push_back(outer);
        indices.push_back(nextInner);
        indices.push_back(nextOuter);
        indices.push_back(outer);
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderGeometry(renderer, nullptr, verts.data(), static_cast<int>(verts.size()),
                       indices.data(), static_cast<int>(indices.size()));
}

// NOLINTBEGIN(readability-identifier-naming)
#pragma once

#include "Render.h"

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <algorithm>
#include <functional>
#include <memory>
#include <ranges>
#include <utility>
#include <vector>

inline auto IDToColor(uint32_t ID) -> SDL_Color {
    return SDL_Color{(Uint8)((ID >> 0) & 0xFF), (Uint8)((ID >> 8) & 0xFF),
                     (Uint8)((ID >> 16) & 0xFF), (Uint8)((ID >> 24) & 0xFF)};
}
inline auto colorToID(SDL_Color c) -> uint32_t {
    return c.r + (c.g << 8) + (c.b << 16) + (c.a << 24);
}

struct Gaps {
    float top, bottom, left, right;
};
enum class Alignment {
    TopLeft,
    TopMiddle,
    TopRight,
    MiddleLeft,
    MiddleMiddle,
    MiddleRight,
    BottomLeft,
    BottomMiddle,
    BottomRight,
    Float
};
struct GridPos {
    int rowStart = 0; // 起始行
    int colStart = 0; // 起始列
    int rowEnd = 1;   // 结束行
    int colEnd = 1;   // 结束列
};
class UI;
class Element {
public:
    bool hasMouseEvents = false;
    std::function<void(int x, int y)> onMouseDown;
    std::function<void(int x, int y)> onMouseClicking;
    std::function<void(int x, int y)> onMouseUp;
    std::function<void(int x, int y)> onHoverIn;
    std::function<void(int x, int y)> onHovering;
    std::function<void(int x, int y)> onHoverOut;

    virtual void onRender(SDL_Renderer* renderer, float absX, float absY) {}

    virtual void onRenderInteraction(SDL_Renderer* renderer, float absX, float absY,
                                     SDL_Color idColor) {}

    virtual ~Element() = default;

    GridPos gridPos{}; // 应该与父元素网格对应
    // 只有对齐方法为float才设置
    float floatX = 0.F; // float情况下的绝对X坐标
    float floatY = 0.F; // float情况下的绝对Y坐标
    float floatW = 0.F; // float情况下的绝对宽度
    float floatH = 0.F; // float情况下的绝对高度
    // 不影响布局只影响渲染
    float offsetX = 0.F;
    float offsetY = 0.F;
    std::vector<Element*> children; // 子节点
    Element* parent;                // 父节点
    uint32_t ID;                    // DO NOT SET IT!!  由ui设置
    UI* uiContext = nullptr;        // DO NOT SET IT!!  从ui创建时会自动设置
    unsigned int hoverTime = 0;
    unsigned int clickTime = 0;
    std::array<bool, 6> mouseState; // DO NOt SET IT!! 左键 中间 右键 侧键1 侧键2 是否悬停
    bool isDirty = true;            // 是否更改
    // 相对于父亲的坐标
    float cacheX = 0.F;
    float cacheY = 0.F;
    float cacheW = 0.F;
    float cacheH = 0.F;

    // 例如{1,2,1 }中间格子为两边格子的两倍大小
    std::vector<float> rowWeights = {1.F};
    std::vector<float> colWeights = {1.F};
    // 格子之间的缝隙
    float rowGap = 0.F;
    float colGap = 0.F;
    Gaps padding{.top = 0.F, .bottom = 0.F, .left = 0.F, .right = 0.F}; // 内边距
    Gaps margin{.top = 0.F, .bottom = 0.F, .left = 0.F, .right = 0.F};  // 外边距

    [[nodiscard]] auto getUI() const -> const UI* { return uiContext; }

    template <typename Self>
    auto setGridPos(this Self&& self, int rs, int cs, int re, int ce) -> auto& {
        self.gridPos = {rs, cs, re, ce};
        self.isDirty = true;
        return self;
    }

    template <typename Self>
    auto setMargin(this Self&& self, float t, float b, float l, float r) -> auto& {
        self.margin = {t, b, l, r};
        self.isDirty = true;
        return self;
    }

    template <typename Self>
    auto setPadding(this Self&& self, float t, float b, float l, float r) -> auto& {
        self.padding = {t, b, l, r};
        self.isDirty = true;
        return self;
    }

    template <typename Self>
    auto setWeights(this Self&& self, std::vector<float> rows, std::vector<float> cols) -> auto& {
        self.rowWeights = std::move(rows);
        self.colWeights = std::move(cols);
        self.isDirty = true;
        return self;
    }

    template <typename Self>
    auto setAlignment(this Self&& self, Alignment align) -> auto& {
        self.alignment = align;
        self.isDirty = true;
        return self;
    }

    template <typename Self>
    auto setGaps(this Self&& self, float row, float col) -> auto& {
        self.rowGap = row;
        self.colGap = col;
        self.isDirty = true;
        return self;
    }
};
class Text : public Element { // outline似乎有换行问题
public:
    ~Text() override {
        if (textHandle != nullptr) {
            TTF_DestroyText(textHandle);
            textHandle = nullptr;
        }
    }
    Alignment alignment = Alignment::TopLeft;
    SDL_Color textFill{};
    SDL_Color outlineFill{};
    int outlineThickness{};
    TTF_FontStyleFlags style{};
    TTF_Font* font{};
    float fontSize{};
    bool autoWarp{};
    TTF_Text* textHandle{};
    bool kerning{};
    template <typename Self>
    auto setAlignment(this Self&& self, Alignment a) -> auto& {
        self.alignment = a;
        return self;
    }

    template <typename Self>
    auto setOutline(this Self&& self, SDL_Color color, int thickness) -> auto& {
        self.outlineFill = color;
        self.outlineThickness = thickness;
        return self;
    }

    template <typename Self>
    auto setText(this Self&& self, const std::string& str) -> auto& {
        if (!self.font) {
            return self;
        }
        if (self.textHandle) {
            TTF_DestroyText(self.textHandle);
        }
        auto* engine = self.uiContext->getTextEngine();
        self.textHandle = TTF_CreateText(engine, self.font, str.c_str(), str.length());
        self.isDirty = true;
        return self;
    }

    template <typename Self>
    auto setStyle(this Self&& self, SDL_Color color, bool kerning, TTF_FontStyleFlags style,
                  TTF_Font* font, float size, bool warp) -> auto& {
        self.textFill = color;
        self.kerning = kerning;
        self.style = style;
        self.font = font;
        self.fontSize = size;
        self.autoWarp = warp;
        return self;
    }
    void onRender(SDL_Renderer* renderer, float absX, float absY) override {
        if (textHandle == nullptr || font == nullptr) {
            return;
        }

        TTF_HorizontalAlignment innerAlignment = TTF_HORIZONTAL_ALIGN_LEFT;
        switch (alignment) {
        case Alignment::TopMiddle:
        case Alignment::MiddleMiddle:
        case Alignment::BottomMiddle:
            innerAlignment = TTF_HORIZONTAL_ALIGN_CENTER;
            break;
        case Alignment::TopRight:
        case Alignment::MiddleRight:
        case Alignment::BottomRight:
            innerAlignment = TTF_HORIZONTAL_ALIGN_RIGHT;
            break;
        default:
            break;
        }

        if (TTF_GetFontWrapAlignment(font) != innerAlignment) {
            TTF_SetFontWrapAlignment(font, innerAlignment);
        }

        if (autoWarp) {
            TTF_SetTextWrapWidth(textHandle, static_cast<int>(cacheW));
        } else {
            TTF_SetTextWrapWidth(textHandle, 0);
        }

        int totalW = 0;
        int totalH = 0;
        TTF_GetTextSize(textHandle, &totalW, &totalH);

        float finalX = absX + offsetX;
        float finalY = absY + offsetY;

        if (alignment >= Alignment::MiddleLeft && alignment <= Alignment::MiddleRight) {
            finalY += (cacheH - static_cast<float>(totalH)) * 0.5F;
        } else if (alignment >= Alignment::BottomLeft) {
            finalY += (cacheH - static_cast<float>(totalH));
        }

        if (!autoWarp) {
            if (innerAlignment == TTF_HORIZONTAL_ALIGN_CENTER) {
                finalX += (cacheW - static_cast<float>(totalW)) * 0.5F;
            } else if (innerAlignment == TTF_HORIZONTAL_ALIGN_RIGHT) {
                finalX += (cacheW - static_cast<float>(totalW));
            }
        }

        if (TTF_GetFontStyle(font) != style) {
            TTF_SetFontStyle(font, style);
        }
        if (outlineThickness > 0) {
            auto orignalOutline = TTF_GetFontOutline(font);
            TTF_SetFontOutline(font, outlineThickness);
            TTF_SetTextColor(textHandle, outlineFill.r, outlineFill.g, outlineFill.b,
                             outlineFill.a);
            TTF_DrawRendererText(textHandle, finalX, finalY);
            TTF_SetFontOutline(font, orignalOutline);
            TTF_SetTextColor(textHandle, textFill.r, textFill.g, textFill.b, textFill.a);
            TTF_DrawRendererText(textHandle, finalX, finalY);
        } else {
            TTF_SetTextColor(textHandle, textFill.r, textFill.g, textFill.b, textFill.a);
            TTF_DrawRendererText(textHandle, finalX, finalY);
        }
    }
};
class Box : public Element {
public: // TODO:内阴影
    SDL_BlendMode blendMode = SDL_BLENDMODE_BLEND;
    SDL_Color backgroundFill{};
    SDL_Color borderFill{};
    SDL_Color shadowFill{};
    float shadowSize{};
    float shadowOffsetX{};
    float shadowOffsetY{};
    float borderThickness{};
    float radius{};

    template <typename Self>
    auto setBgColor(this Self&& self, SDL_Color color) -> auto& {
        self.backgroundFill = color;
        return self;
    }

    template <typename Self>
    auto setBorder(this Self&& self, SDL_Color color, float thickness) -> auto& {
        self.borderFill = color;
        self.borderThickness = thickness;
        return self;
    }

    template <typename Self>
    auto setShadow(this Self&& self, SDL_Color color, float size, float x, float y) -> auto& {
        self.shadowFill = color;
        self.shadowSize = size;
        self.shadowOffsetX = x;
        self.shadowOffsetY = y;
        return self;
    }

    template <typename Self>
    auto setRadius(this Self&& self, float r) -> auto& {
        self.radius = r;
        return self;
    }

    void onRender(SDL_Renderer* renderer, float absX, float absY) override {
        float finalX = absX + offsetX;
        float finalY = absY + offsetY;
        if (shadowSize > 0) {
            drawShadow(renderer, finalX + shadowOffsetX, finalY + shadowOffsetY, cacheW, cacheH,
                       radius, shadowSize, shadowFill);
        }
        std::vector<SDL_Vertex> verts;
        roundedRect(absX + offsetX, absY + offsetY, cacheW, cacheH, radius, backgroundFill, &verts);

        if (verts.size() < 3) {
            return;
        }
        std::vector<int> indices;
        for (int i = 1; i < (int)verts.size() - 1; ++i) {
            indices.push_back(0);     // 中心
            indices.push_back(i);     // 当前边缘点
            indices.push_back(i + 1); // 下一个边缘点
        }

        SDL_SetRenderDrawBlendMode(renderer, blendMode);
        SDL_RenderGeometry(renderer, nullptr, verts.data(), static_cast<int>(verts.size()),
                           indices.data(), static_cast<int>(indices.size()));
        if (borderThickness > 0 && borderFill.a > 0) {
            drawBorder(renderer, finalX, finalY, cacheW, cacheH, radius, borderThickness,
                       borderFill);
        }
    }
    void onRenderInteraction(SDL_Renderer* renderer, float absX, float absY,
                             SDL_Color idColor) override {
        float finalX = absX + offsetX;
        float finalY = absY + offsetY;
        std::vector<SDL_Vertex> verts;
        roundedRect(absX + offsetX, absY + offsetY, cacheW, cacheH, radius, idColor, &verts);
        if (verts.size() < 3) {
            return;
        }
        std::vector<int> indices;
        for (int i = 1; i < (int)verts.size() - 1; ++i) {
            indices.push_back(0);     // 中心
            indices.push_back(i);     // 当前边缘点
            indices.push_back(i + 1); // 下一个边缘点
        }

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        SDL_RenderGeometry(renderer, nullptr, verts.data(), (int)verts.size(), indices.data(),
                           static_cast<int>(indices.size()));
    }
};

class UIRenderer {
public:
    void renderUI(SDL_Renderer* renderer, Element* root, float absX, float absY) {
        absX += root->cacheX;
        absY += root->cacheY;

        root->onRender(renderer, absX, absY);

        for (auto* child : root->children) {
            renderUI(renderer, child, absX, absY);
        }
    }

    void renderInteraction(SDL_Renderer* renderer, Element* root, float absX, float absY) {
        absX += root->cacheX;
        absY += root->cacheY;

        root->onRenderInteraction(renderer, absX, absY, IDToColor(root->ID));

        for (auto* child : root->children) {
            renderInteraction(renderer, child, absX, absY);
        }
    }
};

class LayoutEngine {
public:
    void calculate(Element* root) { // TODO:给这坨写注释
        if (!root->isDirty) {       // 如果不脏就不用计算
            return;
        }
        auto* parent = root->parent; // 获取父元素

        // 权重和
        float kr = std::ranges::fold_left(parent->rowWeights, 0.F, std::plus<>());
        float kc = std::ranges::fold_left(parent->colWeights, 0.F, std::plus<>());
        // 处理权重为0的情况
        if (kr <= 0 || kc <= 0) {
            return;
        }
        // 计算横向单位长度
        float weightUnit = (parent->cacheW - parent->padding.left - parent->padding.right -
                            static_cast<float>(parent->colWeights.size() - 1) * parent->colGap) /
                           kc;
        // 计算竖向单位长度
        float heightUnit = (parent->cacheH - parent->padding.top - parent->padding.bottom -
                            static_cast<float>(parent->rowWeights.size() - 1) * parent->rowGap) /
                           kr;
        // 计算x位置
        root->cacheX =
            parent->padding.left + // 父亲的左内边距
            (weightUnit * std::ranges::fold_left(
                              parent->colWeights | std::ranges::views::take(root->gridPos.colStart),
                              0.F, std::plus<>())) + // 计算前面元素的宽度
            (static_cast<float>(root->gridPos.colStart) *
             parent->colGap) + // 计算前面元素之间的缝隙宽度
            root->margin.left; // 左外边距

        // 计算宽度
        root->cacheW =
            (weightUnit *
             std::ranges::fold_left(
                 parent->colWeights | std::ranges::views::drop(root->gridPos.colStart) |
                     std::ranges::views::take(root->gridPos.colEnd - root->gridPos.colStart),
                 0.F, std::plus<>())) +
            (static_cast<float>(std::max((root->gridPos.colEnd - root->gridPos.colStart - 1), 0)) *
             parent->colGap) -
            root->margin.left - root->margin.right;

        root->cacheY =
            parent->padding.top +
            (heightUnit * std::ranges::fold_left(
                              parent->rowWeights | std::ranges::views::take(root->gridPos.rowStart),
                              0.F, std::plus<>())) +
            (static_cast<float>(root->gridPos.rowStart) * parent->rowGap) + root->margin.top;

        root->cacheH =
            (heightUnit *
             std::ranges::fold_left(
                 parent->rowWeights | std::ranges::views::drop(root->gridPos.rowStart) |
                     std::ranges::views::take(root->gridPos.rowEnd - root->gridPos.rowStart),
                 0.F, std::plus<>())) +
            (static_cast<float>(std::max((root->gridPos.rowEnd - root->gridPos.rowStart - 1), 0)) *
             parent->rowGap) -
            root->margin.top - root->margin.bottom;

        for (auto* child : root->children) {
            child->isDirty = true;
            calculate(child);
        }
        root->isDirty = false;
        root->cacheW = std::max(0.F, root->cacheW);
        root->cacheH = std::max(0.F, root->cacheH);
    }
};

enum AnimationType { Liner };
class Animation {
public:
    void* target;
    std::function<void(float f)> howToChange;
    float duration; // ms
    AnimationType type;
    void start(); // 开始播放
    void stop();  // 停止播放
    void update();

private:
    float startTime;
};

class UI {
private:
    std::vector<std::unique_ptr<Element>> registry; // 所有元素列表
    std::vector<uint32_t> emptyPool;                // DO NOT SET IT!!  用于记录registry空位复用
    std::unique_ptr<Element> rootParent;
    SDL_Texture* interactionTexture{};
    LayoutEngine layoutEngine{};
    UIRenderer uiRenderer{};
    TTF_TextEngine* textEngine; // TODO :增加文字
public:
    UI(SDL_Renderer* r) : renderer(r) {

        textEngine = TTF_CreateRendererTextEngine(renderer);
        rootParent = std::make_unique<Element>();
        rootParent->rowWeights = {1.F};
        rootParent->colWeights = {1.F};
        rootParent->padding = {.top = 0.F, .bottom = 0.F, .left = 0.F, .right = 0.F};
    }
    ~UI() {
        if (textEngine != nullptr) {
            TTF_DestroyRendererTextEngine(textEngine);
            textEngine = nullptr;
        }
    }
    Element* root{}; // 根元素
    SDL_Renderer* renderer;
    bool isDirty = true;
    [[nodiscard]] auto getTextEngine() const { return textEngine; }
    void handleInteractions(int mouseX, int mouseY, std::array<bool, 6> states) {
        static uint32_t lastHoveredElementID = -1;
        static int lastMX = -1;
        static int lastMY = -1;
        static uint32_t lastID = -1;

        uint32_t id = -1;
        if (lastMX == mouseX && lastMY == mouseY) {
            id = lastID;
        } else {
            id = getInteractionTextrueID(mouseX, mouseY);
            lastMX = mouseX;
            lastMY = mouseY;
            lastID = id;
        }

        Element* currentElement = nullptr;
        if (id < static_cast<uint32_t>(registry.size()) && registry[id]) {
            currentElement = registry[id].get();
        }

        if (lastHoveredElementID != id) {
            if (lastHoveredElementID < static_cast<uint32_t>(registry.size()) &&
                registry[lastHoveredElementID]) {
                auto* lastElePtr = registry[lastHoveredElementID].get();
                lastElePtr->mouseState = {false, false, false, false, false, false};

                if (lastElePtr->hoverTime != 0 && lastElePtr->onHoverOut) {
                    lastElePtr->onHoverOut(mouseX, mouseY);
                }
                if (lastElePtr->clickTime != 0 && lastElePtr->onMouseUp) {
                    lastElePtr->onMouseUp(mouseX, mouseY);
                }
                lastElePtr->clickTime = 0;
                lastElePtr->hoverTime = 0;
            }
            lastHoveredElementID = id;
        }

        if (currentElement == nullptr) {
            return;
        }

        if (currentElement->hasMouseEvents) {
            currentElement->mouseState = states;

            if (currentElement->hoverTime == 0 && currentElement->onHoverIn) {
                currentElement->onHoverIn(mouseX, mouseY);
            }
            if (currentElement->onHovering) {
                currentElement->onHovering(mouseX, mouseY);
            }

            currentElement->hoverTime++;

            if (states[5]) {
                if (currentElement->clickTime == 0 && currentElement->onMouseDown) {
                    currentElement->onMouseDown(mouseX, mouseY);
                }
                if (currentElement->onMouseClicking) {
                    currentElement->onMouseClicking(mouseX, mouseY);
                }
                currentElement->clickTime++;
            } else {
                if (currentElement->clickTime > 0) {
                    if (currentElement->onMouseUp) {
                        currentElement->onMouseUp(mouseX, mouseY);
                    }
                    currentElement->clickTime = 0;
                }
            }
        }
    }
    void render(int windowW, int windowH) {
        if (textEngine == nullptr) {
            textEngine = TTF_CreateRendererTextEngine(renderer);
        }
        bool sizeChanged = false;
        if (rootParent->cacheW != static_cast<float>(windowW)) {
            rootParent->cacheW = static_cast<float>(windowW);
            sizeChanged = true;
        }
        if (rootParent->cacheH != static_cast<float>(windowH)) {
            rootParent->cacheH = static_cast<float>(windowH);
            sizeChanged = true;
        }

        if (sizeChanged && (root != nullptr)) {
            root->isDirty = true;
            calculate();
        }

        uiRenderer.renderUI(renderer, root, 0, 0);
    }
    void renderInteractionTexture(int windowW, int windowH) {
        // if (root->isDirty) {
        SDL_DestroyTexture(interactionTexture);
        interactionTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                                               SDL_TEXTUREACCESS_TARGET, windowW, windowH);
        auto* orignalTarget{SDL_GetRenderTarget(renderer)};
        SDL_SetRenderTarget(renderer, interactionTexture);
        uiRenderer.renderInteraction(renderer, root, 0, 0);
        SDL_SetRenderTarget(renderer, orignalTarget);
        // }
    }
    auto getInteractionTextrueID(int mouseX, int mouseY) -> uint32_t {
        SDL_Rect mouseRect{mouseX, mouseY, 1, 1};
        SDL_Texture* orignalTarget = SDL_GetRenderTarget(renderer);
        SDL_SetRenderTarget(renderer, interactionTexture);
        SDL_Surface* pixelSurface = SDL_RenderReadPixels(renderer, &mouseRect);

        SDL_Color color = {0, 0, 0, 0};
        if (pixelSurface != nullptr) {
            SDL_ReadSurfacePixel(pixelSurface, 0, 0, &color.r, &color.g, &color.b, &color.a);
            SDL_DestroySurface(pixelSurface);
        }
        SDL_SetRenderTarget(renderer, orignalTarget);

        return colorToID(color);
    }
    void calculate() { layoutEngine.calculate(root); }

    template <typename T, typename... Args>
    auto AddElement(uint32_t parentID, std::function<void(T&)> config = nullptr, Args&&... args)
        -> T& {
        auto newEle = std::make_unique<T>(std::forward<Args>(args)...);
        T* rawPtr = newEle.get();

        uint32_t targetID = 0;
        // 获取父节点
        Element* parentPtr = (parentID < registry.size()) ? registry[parentID].get() : nullptr;

        if (!emptyPool.empty()) {
            targetID = emptyPool.back();
            emptyPool.pop_back();
            registry[targetID] = std::move(newEle);
        } else {
            targetID = static_cast<uint32_t>(registry.size());
            registry.push_back(std::move(newEle));
        }

        rawPtr->ID = targetID;
        rawPtr->parent = parentPtr;
        rawPtr->uiContext = this;

        if (parentPtr) {
            parentPtr->children.push_back(rawPtr);
        } else if (!root) {
            // 如果没父节点且root还没设置，它就是root
            root = rawPtr;
            root->parent = rootParent.get();
            rootParent->children = {root};
        }

        if (config) {
            config(*rawPtr);
        }
        return *rawPtr;
    }

    template <typename T, typename... Args>
    auto AddElement(Element* parent, std::function<void(T&)> config = nullptr, Args&&... args)
        -> T& {
        uint32_t pID = parent ? parent->ID : 0xFFFFFFFF;
        return AddElement<T>(pID, config, std::forward<Args>(args)...);
    }

    void RemoveElement(uint32_t targetID) {
        if (targetID >= static_cast<uint32_t>(registry.size()) || !registry[targetID]) {
            return; // ID无效或被删除
        }

        Element* element = registry[targetID].get();

        // 从父节点列表中移除自己
        if (element->parent != nullptr) {
            auto& siblings = element->parent->children;
            auto [first, last] = std::ranges::remove(siblings, element);
            siblings.erase(first, last);
        } else if (element == root) {
            root = nullptr;
        }

        // 处理所有子节点
        std::vector<uint32_t> childrenIDs;
        childrenIDs.reserve(static_cast<uint32_t>(element->children.size()));
        for (auto* child : element->children) {
            childrenIDs.push_back(child->ID);
        }

        for (uint32_t childID : childrenIDs) {
            RemoveElement(childID);
        }

        // 回收当前节点
        emptyPool.push_back(targetID);
        registry[targetID].reset(); // 释放内存并设为nullptr
    }
    void RemoveElement(const Element& element) {
        uint32_t id = element.ID;
        RemoveElement(id);
    }
};
// NOLINTEND(readability-identifier-naming)

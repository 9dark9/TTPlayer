#pragma once
#include "ttplayer/skin/skin.h"

namespace ttplayer::skin {
// A native child owns its entire client rectangle, except SkinButton's
// normal-frame window region. Hit testing follows HWND z-order. Some legacy
// main-window children omit WS_CLIPSIBLINGS and overpaint earlier siblings.
struct SkinLayer {
    const SkinElement* element{};
    RECT bounds{};
    int id{};
    bool clip_siblings{true};
};
class SkinLayers {
public:
    SkinLayers(std::vector<SkinLayer> front_to_back, COLORREF key,
        const std::vector<std::wstring>* repaint = nullptr);
    ~SkinLayers();
    SkinLayers(const SkinLayers&) = delete;
    SkinLayers& operator=(const SkinLayers&) = delete;
    const SkinLayer* Hit(POINT point) const;
    // Returned region belongs to the caller, in parent client coordinates.
    HRGN Region(const SkinElement* element) const;
    void Clip(HDC dc, const SkinElement* element) const;
    // Preserve siblings excluded by WS_CLIPSIBLINGS when transferring a
    // partial parent buffer, including its copied background.
    void ClipRepaint(HDC dc) const;
private:
    std::vector<SkinLayer> layers_;
    std::vector<HRGN> regions_;
    std::vector<HRGN> paint_regions_;
};
class SkinLayerClip {
public:
    SkinLayerClip(HDC dc, const SkinLayers& layers, const SkinElement* element)
        : dc_(dc), saved_(SaveDC(dc)) { if (saved_) layers.Clip(dc, element); }
    ~SkinLayerClip() { if (saved_) RestoreDC(dc_, saved_); }
    SkinLayerClip(const SkinLayerClip&) = delete;
    SkinLayerClip& operator=(const SkinLayerClip&) = delete;
private:
    HDC dc_{};
    int saved_{};
};
} // namespace ttplayer::skin

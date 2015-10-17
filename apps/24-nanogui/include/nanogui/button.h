#pragma once

#include <nanogui/widget.h>

NANOGUI_NAMESPACE_BEGIN

class NANOGUI_EXPORT Button : public Widget {
public:
    /// Flags to specify the button behavior (can be combined with binary OR)
    enum ButtonFlags {
        NormalButton = 1,
        RadioButton  = 2,
        ToggleButton = 4,
        PopupButton  = 8
    };

    enum IconPosition {
        Left,
        LeftCentered,
        RightCentered,
        Right
    };

    Button(Widget *parent, const std::string &caption = "Untitled", int icon = 0);

    const std::string &caption() const { return mCaption; }
    void setCaption(const std::string &caption) { mCaption = caption; }

    const Color &backgroundColor() const { return mBackgroundColor; }
    void setBackgroundColor(const Color &backgroundColor) { mBackgroundColor = backgroundColor; }

    const Color &textColor() const { return mTextColor; }
    void setTextColor(const Color &textColor) { mTextColor = textColor; }

    int icon() const { return mIcon; }
    void setIcon(int icon) { mIcon = icon; }

    int buttonFlags() const { return mButtonFlags; }
    void setButtonFlags(int buttonFlags) { mButtonFlags = buttonFlags; }

    int fontSize() const { return mFontSize; }
    void setFontSize(int fontSize) { mFontSize = fontSize; }

    IconPosition iconPosition() const { return mIconPosition; }
    void setIconPosition(IconPosition iconPosition) { mIconPosition = iconPosition; }

    bool pushed() const { return mPushed; }
    void setPushed(bool pushed) { mPushed = pushed; }

    /// Set the push callback (for any type of button)
    std::function<void()> callback() const { return mCallback; }
    void setCallback(std::function<void()> callback) { mCallback = callback; }

    /// Set the change callback (for toggle buttons)
    std::function<void(bool)> changeCallback() const { return mChangeCallback; }
    void setChangeCallback(std::function<void(bool)> callback) { mChangeCallback = callback; }

    /// Set the button group (for radio buttons)
    void setButtonGroup(const std::vector<Button *> &buttonGroup) { mButtonGroup = buttonGroup; }
    const std::vector<Button *> &buttonGroup() const { return mButtonGroup; }

    virtual Vector2i preferredSize(NVGcontext *ctx) const;
    virtual bool mouseButtonEvent(const Vector2i &p, int button, bool down, int modifiers);
    virtual void draw(NVGcontext *ctx);
protected:
    std::string mCaption;
    int mIcon;
    IconPosition mIconPosition;
    bool mPushed;
    int mButtonFlags;
    Color mBackgroundColor;
    Color mTextColor;
    std::function<void()> mCallback;
    std::function<void(bool)> mChangeCallback;
    std::vector<Button *> mButtonGroup;
    int mFontSize;
};

NANOGUI_NAMESPACE_END

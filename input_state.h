#pragma once

class InputState
{
public:
    enum MouseState
    {
        CameraControl,
        GUIMouseControl,
        GUIResizeControl
    };

    enum KeyboardState
    {
        MovementControl,
        GUIKeyboardControl
    };

    InputState();
    ~InputState();

    void setMouseState(MouseState state);
    void setKeyboardState(KeyboardState state);

    MouseState getMouseState() const;
    KeyboardState getKeyboardState() const;

private:
    MouseState mouseState;
    KeyboardState keyboardState;
};
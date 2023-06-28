#include "input_state.h"

InputState::InputState() : mouseState(CameraControl), keyboardState(MovementControl) {}

InputState::~InputState() {}

void InputState::setMouseState(MouseState state) {
    mouseState = state;
}

void InputState::setKeyboardState(KeyboardState state) {
    keyboardState = state;
}

InputState::MouseState InputState::getMouseState() const {
    return mouseState;
}

InputState::KeyboardState InputState::getKeyboardState() const {
    return keyboardState;
}

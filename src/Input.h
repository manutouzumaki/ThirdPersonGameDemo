#ifndef _INPUT_H_
#define _INPUT_H_

struct ButtonState {
    bool mIsDown;
    bool mWasDown;
};

struct Input {
    ButtonState mKeys[350];
    int mMouseX;
    int mMouseY;

    float mLeftStickX;
    float mLeftStickY;
    float mRightStickX;
    float mRightStickY;

    union {
        struct {
            ButtonState mJoyUp;
            ButtonState mJoyDown;
            ButtonState mJoyLeft;
            ButtonState mJoyRight;
            ButtonState mJoyStart;
            ButtonState mJoyBack;
            ButtonState mJoyA;
            ButtonState mJoyB;
            ButtonState mJoyX;
            ButtonState mJoyY;
        };
        ButtonState mJoyButtons[10];  
    };

    union {
        struct {
            ButtonState mMouseLeft;
            ButtonState mMouseMiddle;
            ButtonState mMouseRight;
        };
        ButtonState mMouseButtons[3];
    };
};

#endif

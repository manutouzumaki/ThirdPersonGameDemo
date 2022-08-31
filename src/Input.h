#ifndef _INPUT_H_
#define _INPUT_H_

#define JOYSTICK_BUTTON_UP     0
#define JOYSTICK_BUTTON_DOWN   1
#define JOYSTICK_BUTTON_LEFT   2
#define JOYSTICK_BUTTON_RIGHT  3
#define JOYSTICK_BUTTON_START  4
#define JOYSTICK_BUTTON_BACK   5
#define JOYSTICK_BUTTON_A      6
#define JOYSTICK_BUTTON_B      7
#define JOYSTICK_BUTTON_X      8
#define JOYSTICK_BUTTON_Y      9

#define KEYBOARD_KEY_BACKESPACE	 0x08
#define KEYBOARD_KEY_ESCAPE      0x1B
#define KEYBOARD_KEY_0  0x30 
#define KEYBOARD_KEY_1  0x31 
#define KEYBOARD_KEY_2  0x32 
#define KEYBOARD_KEY_3  0x33 
#define KEYBOARD_KEY_4  0x34 
#define KEYBOARD_KEY_5  0x35 
#define KEYBOARD_KEY_6  0x36 
#define KEYBOARD_KEY_7  0x37 
#define KEYBOARD_KEY_8  0x38 
#define KEYBOARD_KEY_9  0x39 
#define KEYBOARD_KEY_A  0x41 
#define KEYBOARD_KEY_B  0x42 
#define KEYBOARD_KEY_C  0x43 
#define KEYBOARD_KEY_D  0x44 
#define KEYBOARD_KEY_E  0x45 
#define KEYBOARD_KEY_F  0x46 
#define KEYBOARD_KEY_G  0x47 
#define KEYBOARD_KEY_H  0x48 
#define KEYBOARD_KEY_I  0x49 
#define KEYBOARD_KEY_J  0x4A 
#define KEYBOARD_KEY_K  0x4B 
#define KEYBOARD_KEY_L  0x4C 
#define KEYBOARD_KEY_M  0x4D 
#define KEYBOARD_KEY_N  0x4E 
#define KEYBOARD_KEY_O  0x4F 
#define KEYBOARD_KEY_P  0x50 
#define KEYBOARD_KEY_Q  0x51 
#define KEYBOARD_KEY_R  0x52 
#define KEYBOARD_KEY_S  0x53 
#define KEYBOARD_KEY_T  0x54 
#define KEYBOARD_KEY_U  0x55 
#define KEYBOARD_KEY_V  0x56 
#define KEYBOARD_KEY_W  0x57 
#define KEYBOARD_KEY_X  0x58 
#define KEYBOARD_KEY_Y  0x59 
#define KEYBOARD_KEY_Z  0x5A 
#define KEYBOARD_KEY_NUMPAD0  0x60	    
#define KEYBOARD_KEY_NUMPAD1  0x61	    
#define KEYBOARD_KEY_NUMPAD2  0x62	    
#define KEYBOARD_KEY_NUMPAD3  0x63	    
#define KEYBOARD_KEY_NUMPAD4  0x64	    
#define KEYBOARD_KEY_NUMPAD5  0x65	    
#define KEYBOARD_KEY_NUMPAD6  0x66	    
#define KEYBOARD_KEY_NUMPAD7  0x67	    
#define KEYBOARD_KEY_NUMPAD8  0x68	    
#define KEYBOARD_KEY_NUMPAD9  0x69	    
#define KEYBOARD_KEY_RETURN   0x0D
#define KEYBOARD_KEY_SPACE    0x20
#define KEYBOARD_KEY_TAB      0x09
#define KEYBOARD_KEY_CONTROL  0x11
#define KEYBOARD_KEY_SHIFT    0x10
#define KEYBOARD_KEY_ALT      0x12
#define KEYBOARD_KEY_CAPS     0x14
#define KEYBOARD_KEY_LEFT     0x25
#define KEYBOARD_KEY_UP       0x26
#define KEYBOARD_KEY_RIGHT    0x27
#define KEYBOARD_KEY_DOWN     0x28

#define MOUSE_BUTTON_LEFT    0
#define MOUSE_BUTTON_MIDDLE  1
#define MOUSE_BUTTON_RIGHT   2


struct ButtonState {
    bool mIsDown;
    bool mWasDown;
};

struct Input {
    ButtonState mKeys[350];
    int mMouseX;
    int mMouseY;
    int mMouseWheel;

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

bool KeyboardGetKeyDown(int key);
bool KeyboardGetKeyJustDown(int key);
bool KeyboardGetKeyJustUp(int key);
bool KeyboardGetKeyUp(int key);

bool MouseGetButtonDown(int button);
bool MouseGetButtonJustDown(int button);
bool MouseGetButtonJustUp(int button);
bool MouseGetButtonUp(int button);

int MouseGetCursorX();
int MouseGetCursorY();
int MouseGetWheel();
int MouseGetLastCursorX();
int MouseGetLastCursorY();

bool JoysickGetButtonDown(int button);
bool JoysickGetButtonJustDown(int button);
bool JoysickGetButtonJustUp(int button);
bool JoysickGetButtonUp(int button);

float JoysickGetLeftStickX();
float JoysickGetLeftStickY();
float JoysickGetRightStickX();
float JoysickGetRightStickY();

#endif

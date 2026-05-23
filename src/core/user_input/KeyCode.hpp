#pragma once

namespace Core {

// Represents physical key codes mapped directly to GLFW constants
enum class KeyCode : int
{
    Unknown = -1,
    
    // Control keys
    Backspace = 259,
    Tab = 258,
    Return = 257,
    Pause = 284,
    Escape = 256,
    Space = 32,
    
    // Symbols
    Quote = 39,
    Comma = 44,
    Minus = 45,
    Period = 46,
    Slash = 47,
    Semicolon = 59,
    Equals = 61,
    LeftBracket = 91,
    BackSlash = 92,
    RightBracket = 93,
    Backquote = 96,
    Delete = 261,
    
    // Digits
    Zero = 48,
    One = 49,
    Two = 50,
    Three = 51,
    Four = 52,
    Five = 53,
    Six = 54,
    Seven = 55,
    Eight = 56,
    Nine = 57,
    
    // Letters (Uppercase to match GLFW)
    A = 65,
    B = 66,
    C = 67,
    D = 68,
    E = 69,
    F = 70,
    G = 71,
    H = 72,
    I = 73,
    J = 74,
    K = 75,
    L = 76,
    M = 77,
    N = 78,
    O = 79,
    P = 80,
    Q = 81,
    R = 82,
    S = 83,
    T = 84,
    U = 85,
    V = 86,
    W = 87,
    X = 88,
    Y = 89,
    Z = 90,
    
    // Keypad controls
    KeypadZero = 320,
    KeypadOne = 321,
    KeypadTwo = 322,
    KeypadThree = 323,
    KeypadFour = 324,
    KeypadFive = 325,
    KeypadSix = 326,
    KeypadSeven = 327,
    KeypadEight = 328,
    KeypadNine = 329,
    KeypadPeriod = 330,
    KeypadDivide = 331,
    KeypadMultiply = 332,
    KeypadMinus = 333,
    KeypadPlus = 334,
    KeypadEnter = 335,
    KeypadEquals = 336,
    
    // Navigation / Arrows
    Up = 265,
    Down = 264,
    Right = 262,
    Left = 263,
    Insert = 260,
    Home = 268,
    End = 269,
    PageUp = 266,
    PageDown = 267,
    
    // Function keys
    F1 = 290,
    F2 = 291,
    F3 = 292,
    F4 = 293,
    F5 = 294,
    F6 = 295,
    F7 = 296,
    F8 = 297,
    F9 = 298,
    F10 = 299,
    F11 = 300,
    F12 = 301,
    F13 = 302,
    F14 = 303,
    F15 = 304,
    
    // Modifiers & Toggle
    NumLock = 282,
    CapsLock = 280,
    ScrollLock = 281,
    RightShift = 344,
    LeftShift = 340,
    RightControl = 345,
    LeftControl = 341,
    RightAlt = 346,
    LeftAlt = 342,
    RightSuper = 347,
    LeftSuper = 343,
    Menu = 348,
    Print = 283
};

} // namespace Core

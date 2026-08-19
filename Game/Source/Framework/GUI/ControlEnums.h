#pragma once

enum class ControlType
{
    EMPTY_CONTROL = -1,
    BUTTON,
    TOGGLE,
    CHECKBOX,
    SLIDER,
    SLIDERBAR,
    COMBOBOX,
    DROPDOWNBOX,
    INPUTBOX,
    VALUEBOX,
    SPINNER,
    TEXT,
    IMAGE
    //TODO: Què m'atreveixo a fer gui d'això?
    // TextField // una línia editable
    // TextArea  // multilínia editable
    // I tots els altres
};

enum class ControlState
{
    NORMAL,
    FOCUSED,
    PRESSED,
    SELECTED,
    DISABLED
};

#ifndef PROCYON_H
#define PROCYON_H

/*
 * Header file that includes all that should be necessary to use the library.
 */

#ifdef WIN32

// when NVIDIA Optimus is present, we need to indicate that the device should
// use the discrete GPU for running this

#include <windows.h>

__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;

#endif

#include "color.h"
#include "drawing.h"
#include "keys.h"
#include "state.h"
#include "window.h"

#endif

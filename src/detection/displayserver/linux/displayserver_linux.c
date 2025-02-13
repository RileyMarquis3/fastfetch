#include "displayserver_linux.h"
#include "common/io/io.h"
#include "util/stringUtils.h"

#ifdef __FreeBSD__
    #include "common/settings.h"
#endif

static void getWMProtocolNameFromEnv(FFDisplayServerResult* result)
{
    const char* env = getenv("XDG_SESSION_TYPE");
    if(env)
    {
        if(ffStrEqualsIgnCase(env, "wayland"))
            ffStrbufSetS(&result->wmProtocolName, FF_WM_PROTOCOL_WAYLAND);
        else if(ffStrEqualsIgnCase(env, "x11") || ffStrEqualsIgnCase(env, "xorg"))
            ffStrbufSetS(&result->wmProtocolName, FF_WM_PROTOCOL_X11);
        else if(ffStrEqualsIgnCase(env, "tty"))
            ffStrbufSetS(&result->wmProtocolName, FF_WM_PROTOCOL_TTY);
        else
            ffStrbufSetS(&result->wmProtocolName, env);

        return;
    }

    if(getenv("WAYLAND_DISPLAY") != NULL || getenv("WAYLAND_SOCKET") != NULL)
    {
        ffStrbufSetStatic(&result->wmProtocolName, FF_WM_PROTOCOL_WAYLAND);
        return;
    }

    if(getenv("DISPLAY") != NULL) // XWayland also set this
    {
        ffStrbufSetStatic(&result->wmProtocolName, FF_WM_PROTOCOL_X11);
        return;
    }

    env = getenv("TERM");
    if(ffStrSet(env) && ffStrEqualsIgnCase(env, "linux"))
    {
        ffStrbufSetStatic(&result->wmProtocolName, FF_WM_PROTOCOL_TTY);
        return;
    }
}

void ffConnectDisplayServerImpl(FFDisplayServerResult* ds)
{
    if (instance.config.general.dsForceDrm == FF_DS_FORCE_DRM_TYPE_FALSE)
    {
        //We try wayland as our preferred display server, as it supports the most features.
        //This method can't detect the name of our WM / DE
        ffdsConnectWayland(ds);

        //Try the x11 libs, from most feature rich to least.
        //We use the display list to detect if a connection is needed.
        //They respect wmProtocolName, and only detect display if it is set.
        if(ds->displays.length == 0)
            ffdsConnectXcbRandr(ds);

        if(ds->displays.length == 0)
            ffdsConnectXrandr(ds);
    }

    //This display detection method is display server independent.
    //Use it if all connections failed
    if(ds->displays.length == 0)
        ffdsConnectDrm(ds);

    #ifdef __FreeBSD__
    if(ds->displays.length == 0)
    {
        FF_STRBUF_AUTO_DESTROY buf = ffStrbufCreate();
        if (ffSettingsGetFreeBSDKenv("screen.width", &buf))
        {
            uint32_t width = (uint32_t) ffStrbufToUInt(&buf, 0);
            if (width)
            {
                ffStrbufClear(&buf);
                if (ffSettingsGetFreeBSDKenv("screen.height", &buf))
                {
                    uint32_t height = (uint32_t) ffStrbufToUInt(&buf, 0);
                    ffdsAppendDisplay(ds, width, height, 0, 0, 0, 0, 0, 0, 0, NULL, FF_DISPLAY_TYPE_UNKNOWN, false, 0, 0, 0, "kenv");
                }
            }
        }
    }
    #endif

    if (ds->wmProtocolName.length == 0)
        getWMProtocolNameFromEnv(ds);

    if(!ffStrbufEqualS(&ds->wmProtocolName, FF_WM_PROTOCOL_TTY))
    {
        //This fills in missing information about WM / DE by using env vars and iterating processes
        ffdsDetectWMDE(ds);
    }
}

FFDisplayType ffdsGetDisplayType(const char* name)
{
    if(ffStrStartsWith(name, "eDP-") || ffStrStartsWith(name, "LVDS-"))
        return FF_DISPLAY_TYPE_BUILTIN;
    else if(ffStrStartsWith(name, "HDMI-") ||
            ffStrStartsWith(name, "DP-") ||
            ffStrStartsWith(name, "DisplayPort-") ||
            ffStrStartsWith(name, "DVI-") ||
            ffStrStartsWith(name, "VGA-"))
        return FF_DISPLAY_TYPE_EXTERNAL;

    return FF_DISPLAY_TYPE_UNKNOWN;
}

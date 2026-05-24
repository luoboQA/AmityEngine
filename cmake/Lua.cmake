# ==============================================================================
# Lua Dependency Configuration (Statically compiled from official source)
# ==============================================================================

include(FetchContent)
FetchContent_Declare(
    lua
    URL "https://www.lua.org/ftp/lua-5.4.6.tar.gz"
)
FetchContent_GetProperties(lua)
if(NOT lua_POPULATED)
    FetchContent_Populate(lua)
    
    # Official Lua 5.4 source files (excluding standalone interpreter/compiler lua.c/luac.c)
    file(GLOB LUA_SOURCES 
        "${lua_SOURCE_DIR}/src/lapi.c"
        "${lua_SOURCE_DIR}/src/lcode.c"
        "${lua_SOURCE_DIR}/src/lctype.c"
        "${lua_SOURCE_DIR}/src/ldebug.c"
        "${lua_SOURCE_DIR}/src/ldo.c"
        "${lua_SOURCE_DIR}/src/ldump.c"
        "${lua_SOURCE_DIR}/src/lfunc.c"
        "${lua_SOURCE_DIR}/src/lgc.c"
        "${lua_SOURCE_DIR}/src/llex.c"
        "${lua_SOURCE_DIR}/src/lmem.c"
        "${lua_SOURCE_DIR}/src/lobject.c"
        "${lua_SOURCE_DIR}/src/lopcodes.c"
        "${lua_SOURCE_DIR}/src/lparser.c"
        "${lua_SOURCE_DIR}/src/lstate.c"
        "${lua_SOURCE_DIR}/src/lstring.c"
        "${lua_SOURCE_DIR}/src/ltable.c"
        "${lua_SOURCE_DIR}/src/ltm.c"
        "${lua_SOURCE_DIR}/src/lundump.c"
        "${lua_SOURCE_DIR}/src/lvm.c"
        "${lua_SOURCE_DIR}/src/lzio.c"
        "${lua_SOURCE_DIR}/src/lauxlib.c"
        "${lua_SOURCE_DIR}/src/lbaselib.c"
        "${lua_SOURCE_DIR}/src/lcorolib.c"
        "${lua_SOURCE_DIR}/src/ldblib.c"
        "${lua_SOURCE_DIR}/src/liolib.c"
        "${lua_SOURCE_DIR}/src/lmathlib.c"
        "${lua_SOURCE_DIR}/src/loslib.c"
        "${lua_SOURCE_DIR}/src/lstrlib.c"
        "${lua_SOURCE_DIR}/src/ltablib.c"
        "${lua_SOURCE_DIR}/src/lutf8lib.c"
        "${lua_SOURCE_DIR}/src/loadlib.c"
        "${lua_SOURCE_DIR}/src/linit.c"
    )
    
    add_library(lua STATIC ${LUA_SOURCES})
    target_include_directories(lua PUBLIC "${lua_SOURCE_DIR}/src")
    if(UNIX AND NOT APPLE)
        target_link_libraries(lua PRIVATE dl m)
    endif()
endif()

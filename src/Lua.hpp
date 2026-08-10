//
// Created by penggrin on 10.08.2026.
//

#ifndef VOXELGAME_LUA_HPP
#define VOXELGAME_LUA_HPP

#include <string>
#include <glm/glm.hpp>
#include <spdlog/spdlog.h>

#include "lua.hpp"
#include "tracy/Tracy.hpp"

class Lua {
private:
    lua_State *state;

    static int luaVec3(lua_State *state);
    static int luaIVec3(lua_State *state);

    static int luaInputKeyDown(lua_State *state);
    static int luaPlayerGetPos(lua_State *state);
    static int luaPlayerSetPos(lua_State *state);
    static int luaLevelGetVoxel(lua_State *state);
    static int luaLevelSetVoxel(lua_State *state);

    static glm::vec3 luaCheckVec3(lua_State *state, int xArg);
    static glm::ivec3 luaCheckIVec3(lua_State *state, int xArg);

    static int luaVec3ToString(lua_State *state) {
        lua_getfield(state, 1, "x");
        lua_getfield(state, 1, "y");
        lua_getfield(state, 1, "z");

        const lua_Number x = lua_tonumber(state, -3);
        const lua_Number y = lua_tonumber(state, -2);
        const lua_Number z = lua_tonumber(state, -1);

        lua_pushfstring(state, "vec3(%f, %f, %f)", x, y, z);
        return 1;
    }
    static int luaIVec3ToString(lua_State *state) {
        lua_getfield(state, 1, "x");
        lua_getfield(state, 1, "y");
        lua_getfield(state, 1, "z");

        const lua_Integer x = lua_tointeger(state, -3);
        const lua_Integer y = lua_tointeger(state, -2);
        const lua_Integer z = lua_tointeger(state, -1);

        lua_pushfstring(state, "ivec3(%d, %d, %d)", x, y, z);
        return 1;
    }

    static void registerVec3Meta(lua_State *state) {
        luaL_newmetatable(state, "Vec3Meta");

        lua_pushcfunction(state, luaVec3ToString);
        lua_setfield(state, -2, "__tostring");

        lua_pop(state, 1);
    }
    static void registerIVec3Meta(lua_State *state) {
        luaL_newmetatable(state, "IVec3Meta");

        lua_pushcfunction(state, luaIVec3ToString);
        lua_setfield(state, -2, "__tostring");

        lua_pop(state, 1);
    }

    static void pushToLua(lua_State *state, const glm::vec3 &vec) {
        lua_createtable(state, 0, 3);
        lua_pushnumber(state, vec.x);
        lua_setfield(state, -2, "x");
        lua_pushnumber(state, vec.y);
        lua_setfield(state, -2, "y");
        lua_pushnumber(state, vec.z);
        lua_setfield(state, -2, "z");

        luaL_setmetatable(state, "Vec3Meta");
    }

    static void pushToLua(lua_State *state, const glm::ivec3 &vec) {
        lua_createtable(state, 0, 3);
        lua_pushinteger(state, vec.x);
        lua_setfield(state, -2, "x");
        lua_pushinteger(state, vec.y);
        lua_setfield(state, -2, "y");
        lua_pushinteger(state, vec.z);
        lua_setfield(state, -2, "z");

        luaL_setmetatable(state, "IVec3Meta");
    }

    static void pushToLua(lua_State *state, const int value) {
        lua_pushinteger(state, value);
    }

    static void pushToLua(lua_State *state, const double value) {
        lua_pushnumber(state, value);
    }

    static void pushToLua(lua_State *state, const bool value) {
        lua_pushboolean(state, value);
    }

    static void pushToLua(lua_State *state, const char *value) {
        lua_pushstring(state, value);
    }

    static void pushToLua(lua_State *state, const std::string &value) {
        lua_pushlstring(state, value.data(), value.size());
    }
public:
    Lua() : state(luaL_newstate()) {
        luaL_openlibs(state);

        registerVec3Meta(state);
        registerIVec3Meta(state);

        lua_register(state, "vec3", luaVec3);
        lua_register(state, "ivec3", luaIVec3);

        lua_createtable(state, 0, 2);
        lua_pushcfunction(state, luaPlayerGetPos);
        lua_setfield(state, -2, "getPos");
        lua_pushcfunction(state, luaPlayerSetPos);
        lua_setfield(state, -2, "setPos");
        lua_setglobal(state, "Player");

        lua_createtable(state, 0, 2);
        lua_pushcfunction(state, luaLevelGetVoxel);
        lua_setfield(state, -2, "getVoxel");
        lua_pushcfunction(state, luaLevelSetVoxel);
        lua_setfield(state, -2, "setVoxel");
        lua_setglobal(state, "Level");

        lua_createtable(state, 0, 1);
        lua_pushcfunction(state, luaInputKeyDown);
        lua_setfield(state, -2, "keyDown");
        lua_setglobal(state, "Input");
    }
    ~Lua() { lua_close(state); }

    [[nodiscard]] int getRef(const char *name) const {
        lua_getglobal(state, name);
        return luaL_ref(state, LUA_REGISTRYINDEX);
    }

    void open(const char *fileName) const {
        if (luaL_dofile(state, fileName) != LUA_OK) {
            SPDLOG_ERROR("{}", lua_tostring(state, -1));
            lua_pop(state, 1);
        }
    }

    template <typename... Args>
    void call(const char *name, Args&&... args) const { ZoneScoped;
        lua_getglobal(state, name);

        if (!lua_isfunction(state, -1)) {
            lua_pop(state, 1);
            return;
        }

        (pushToLua(state, std::forward<Args>(args)), ...);

        if (lua_pcall(state, sizeof...(Args), 0, 0) != LUA_OK) {
            SPDLOG_ERROR("{}", lua_tostring(state, -1));
            lua_pop(state, 1);
        }
    }

    template <typename... Args>
    void callFor(const int luaRef, const char* name, Args&&... args) const { ZoneScoped;
        if (luaRef <= 0)
            return;

        lua_rawgeti(state, LUA_REGISTRYINDEX, luaRef);
        lua_getfield(state, -1, name);

        if (!lua_isfunction(state, -1)) {
            lua_pop(state, 2);
            return;
        }

        lua_pushvalue(state, -2); // self

        (pushToLua(state, std::forward<Args>(args)), ...);

        if (lua_pcall(state, 1 + sizeof...(Args), 0, 0) != LUA_OK) {
            SPDLOG_ERROR("{}", lua_tostring(state, -1));
            lua_pop(state, 2);
            return;
        }

        lua_pop(state, 1);
    }
};


#endif //VOXELGAME_LUA_HPP

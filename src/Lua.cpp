//
// Created by penggrin on 10.08.2026.
//

#include "Lua.hpp"

#include "Game.hpp"
#include "../cmake-build-debug/_deps/lua-src/lua-5.4.7/include/lauxlib.h"
#include "../cmake-build-release/_deps/lua-src/lua-5.4.7/include/lua.h"

int Lua::luaPlayerGetPos(lua_State *state) {
    pushToLua(state, game->getPlayer().getPos());
    return 1;
}

int Lua::luaPlayerSetPos(lua_State *state) {
    const ivec3 pos = luaCheckIVec3(state, 1);
    game->getPlayer().setPos(pos);
    return 0;
}

int Lua::luaLevelGetVoxel(lua_State *state) {
    const ivec3 pos = luaCheckIVec3(state, 1);
    pushToLua(state, game->getCurrLevel()->getVoxel(pos));
    return 1;
}

int Lua::luaLevelSetVoxel(lua_State *state) {
    const ivec3 pos = luaCheckIVec3(state, 1);
    const auto voxelId = static_cast<Voxel::Id>(luaL_checkinteger(state, 4));
    game->getCurrLevel()->setVoxel(pos, voxelId);
    return 0;
}

glm::vec3 Lua::luaCheckVec3(lua_State *state, const int xArg) {
    luaL_checktype(state, xArg, LUA_TTABLE);

    lua_getfield(state, xArg, "x");
    lua_getfield(state, xArg, "y");
    lua_getfield(state, xArg, "z");

    const auto x = static_cast<float>(luaL_checknumber(state, -3));
    const auto y = static_cast<float>(luaL_checknumber(state, -2));
    const auto z = static_cast<float>(luaL_checknumber(state, -1));

    lua_pop(state, 3);

    return glm::vec3{x, y, z};
}

glm::ivec3 Lua::luaCheckIVec3(lua_State *state, const int xArg) {
    luaL_checktype(state, xArg, LUA_TTABLE);

    lua_getfield(state, xArg, "x");
    lua_getfield(state, xArg, "y");
    lua_getfield(state, xArg, "z");

    const auto x = static_cast<int>(luaL_checkinteger(state, -3));
    const auto y = static_cast<int>(luaL_checkinteger(state, -2));
    const auto z = static_cast<int>(luaL_checkinteger(state, -1));

    lua_pop(state, 3);

    return glm::ivec3{x, y, z};
}

int Lua::luaVec3(lua_State *state) {
    const auto x = static_cast<float>(luaL_checknumber(state, -3));
    const auto y = static_cast<float>(luaL_checknumber(state, -2));
    const auto z = static_cast<float>(luaL_checknumber(state, -1));
    pushToLua(state, glm::vec3{x, y, z});
    return 1;
}

int Lua::luaIVec3(lua_State *state) {
    const auto x = static_cast<int>(luaL_checkinteger(state, -3));
    const auto y = static_cast<int>(luaL_checkinteger(state, -2));
    const auto z = static_cast<int>(luaL_checkinteger(state, -1));
    pushToLua(state, glm::ivec3{x, y, z});
    return 1;
}

int Lua::luaInputKeyDown(lua_State *state) {
    pushToLua(state, GameInput::keyDown(static_cast<KeyboardKey>(luaL_checkinteger(state, 1))));
    return 1;
}

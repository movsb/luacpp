#include <iostream>

extern "C" {
#include "lua/luaconf.h"
#include "lua/lua.h"
#include "lua/lauxlib.h"
#include "lua/lualib.h"
}

lua_State* L;

class MyObject
{
public:
    MyObject()
        : n(8)
    {}

public:
    // 从第 1 个参数中取出对象指针
    static MyObject* check_this(lua_State* L)
    {
       return (MyObject*)luaL_checkudata(L, 1, "MyObject");
    }

public:
    // 静态成员函数
    static int sf(lua_State* L)
    {
        MyObject* self = check_this(L);
        std::cout << __FUNCTION__ << ": " << self->n << std::endl;
        std::cout << "  argc: " << lua_gettop(L) << ": "
            << luaL_checknumber(L, 2) << "," 
            << luaL_checkstring(L, 3) 
            << std::endl;
        return 0;
    }

    // 一个标准调用方法
    int __stdcall mf(lua_State* L)
    {
        std::cout << __FUNCTION__ << ": " << n << std::endl;
        std::cout << "  argc: " << lua_gettop(L) << ": "
            << luaL_checknumber(L, 2) << "," 
            << luaL_checkstring(L, 3) 
            << std::endl;
        return 0;
    }

protected:
    int n;
};

template<typename T>
lua_CFunction tolcf(T t)
{
    union
    {
        T t;
        lua_CFunction f;
    } u;

    u.t = t;
    *(int*)&u.f |= 0x80000000;

    return u.f;
}

int main()
{
    L = luaL_newstate();
    luaL_openlibs(L);

    void* mem = lua_newuserdata(L, sizeof(MyObject));
    auto pp = new (mem) MyObject;

    luaL_newmetatable(L, "MyObject");
    luaL_Reg api[] {
        {"sf",      &MyObject::sf},
        {"mf",      tolcf(&MyObject::mf)},
        {nullptr,   nullptr}
    };
    luaL_setfuncs(L, api, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    lua_setmetatable(L, -2);

    lua_setglobal(L, "x");

    auto script = R"(
print(x)
print(getmetatable(x))
print(x.sf)
x:sf(123, "456")
x.sf(x, 123, "456")
print(x.mf)
x:mf(123, "456")
x.mf(x, 123, "456")
)";

    if(luaL_dostring(L, script) != LUA_OK) {
        printf("error: %s\n", lua_tostring(L, -1));
    }

    lua_close(L);
}

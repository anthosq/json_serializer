#include <iostream>
#include <string>
#include <json/json.h>

#define REFLECT_PP_FOREACH_1(f, _1) f(_1)
#define REFLECT_PP_FOREACH_2(f, _1, _2) f(_1) f(_2)
#define REFLECT_PP_FOREACH_3(f, _1, _2, _3) f(_1) f(_2) f(_3)
#define REFLECT_PP_FOREACH_4(f, _1, _2, _3, _4) f(_1) f(_2) f(_3) f(_4)
#define REFLECT_PP_FOREACH_5(f, _1, _2, _3, _4, _5) f(_1) f(_2) f(_3) f(_4) f(_5)
#define REFLECT_PP_FOREACH_NARGS_IMPL(_1, _2, _3, _4, _5, N, ...) N
#define REFLECT_PP_FOREACH_NARGS(...) REFLECT_PP_FOREACH_NARGS_IMPL(__VA_ARGS__, 5, 4, 3, 2, 1)

#define REFLECT_EXPAND(x) x
#define REFLECT_CONCAT_2(x, y) x##y 
#define REFLECT_CONCAT(x, y) REFLECT_CONCAT_2(x, y)
#define REFLECT_PP_FOREACH(f, ...) REFLECT_EXPAND(REFLECT_CONCAT(REFLECT_PP_FOREACH_, REFLECT_PP_FOREACH_NARGS(__VA_ARGS__))(f, __VA_ARGS__))
// 考虑宏的求值顺序，定义REFLECT_CONCAT
// 宏的求值顺序问题，定义REFLECT_EXPAND,用于展开宏,进行宏求值

template <class T>
struct reflect_trait {};

#define REFLECT_BEGIN(Type) \
template <> \
struct reflect_trait<Type> { \
    template <class Func> \
    static constexpr void for_each_members(Type &obj, Func &&func) {

#define REFLECT_PER_MEMBER(x) \
    func(#x, obj.x);

#define REFLECT_END() \
    } \
};

#define REFLECT(Type, ...) \
REFLECT_BEGIN(Type) \
REFLECT_PP_FOREACH(REFLECT_PER_MEMBER, __VA_ARGS__) \
REFLECT_END();

template <class T>
std::string serializer(T &obj) {
    Json::Value root;
    reflect_trait<T>::for_each_members(obj, [&] (const char *key, auto &value) {
        root[key] = value;
    });
    return root.toStyledString();
}

struct GObject {
    std::string m_name;
    std::string m_definition_url;
    int m_id;
};

REFLECT(GObject, m_name, m_definition_url, m_id);

int main() {
    GObject m_go = {
        .m_name = "TestObj",
        .m_definition_url = "Testurl",
        .m_id = 1,
    };
    std::string bin = serializer(m_go);
    std::cout << bin << '\n';
    return 0;
}

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

#define REFLECT_EXPAND_2(...) __VA_ARGS__
#define REFLECT_EXPAND(...) REFLECT_EXPAND_2(__VA_ARGS__)
#define REFLECT_CONCAT_2(x, y) x##y 
#define REFLECT_CONCAT(x, y) REFLECT_CONCAT_2(x, y)
#define REFLECT_PP_FOREACH(f, ...) REFLECT_EXPAND(REFLECT_CONCAT(REFLECT_PP_FOREACH_, REFLECT_PP_FOREACH_NARGS(__VA_ARGS__))(f, __VA_ARGS__))
// 考虑宏的求值顺序，定义REFLECT_CONCAT
// 宏的求值顺序问题，定义REFLECT_EXPAND,用于展开宏,进行宏求值

template <class T>
struct reflect_trait {
    template <class Func>
    static constexpr void for_each_members(T &self, Func &&func) {
        self.for_each_members(std::forward<Func>(func));
    }
};

#define REFLECT_TYPE_BEGIN(Type) \
template <> \
struct reflect_trait<Type> { \
    template <class Func> \
    static constexpr void for_each_members(Type &self, Func &&func) {

#define REFLECT_TYPE_TEMPLATED_BEGIN(Type, ...) \
template <__VA_ARGS__> \
struct reflect_trait<REFLECT_EXPAND(REFLECT_EXPAND Type)> { \
    template <class Func> \
    static constexpr void for_each_members(REFLECT_EXPAND(REFLECT_EXPAND Type) &self, Func &&func) {

#define REFLECT_TYPE_PER_MEMBER(x) \
    func(#x, self.x);

#define REFLECT_END() \
    } \
};

#define REFLECT_TYPE(Type, ...) \
REFLECT_TYPE_BEGIN(Type) \
REFLECT_PP_FOREACH(REFLECT_TYPE_PER_MEMBER, __VA_ARGS__) \
REFLECT_END();

#define REFLECT_TYPE_TEMPLATED(Type, ...) \
REFLECT_EXPAND(REFLECT_TYPE_TEMPLATED_BEGIN Type) \
REFLECT_PP_FOREACH(REFLECT_TYPE_PER_MEMBER, __VA_ARGS__) \
REFLECT_END();

#define REFLECT_PER_MEMBER(x) \
    func(#x, x);

#define REFLECT(...) \
template <class Func> \
constexpr void for_each_members(Func &&func) { \
    REFLECT_PP_FOREACH(REFLECT_PER_MEMBER, __VA_ARGS__) \
}


template <class T>
std::string serializer(T &obj) {
    Json::Value root;
    reflect_trait<T>::for_each_members(obj, [&] (const char *key, auto &value) {
        root[key] = value;
    });
    return root.toStyledString();
}

template <class T>
struct GObject {
    std::string m_name;
    std::string m_definition_url;
    int m_id;
    T m_components;
};


REFLECT_TYPE_TEMPLATED(((GObject<T>), class T), m_name, m_definition_url, m_id, m_components);

int main() {
    GObject<float> m_go = {
        .m_name = "TestObj",
        .m_definition_url = "Testurl",
        .m_id = 1,
        .m_components = 1.0f,
    };
    std::string bin = serializer(m_go);
    std::cout << bin << '\n';
    return 0;
}

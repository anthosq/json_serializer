#include <iostream>
#include <string>
#include <json/json.h>
#include <type_traits>
#include <pybind11/pybind11.h>

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
    static constexpr bool has_members() {
        return requires (T t) {
            t.for_each_members([] (const char *, auto &) {});
        };
    } 
    template <class Func>
    static constexpr void for_each_members(T const &cself, Func &&func) {
        const_cast<T &>(cself).for_each_members(std::forward<Func>(func));
    }

    template <class Func>
    static constexpr void for_each_members_ptrs(Func &&func) {
        T::template for_each_members_ptrs<T>(std::forward<Func>(func));
    }
};

#define REFLECT_TYPE_PER_MEMBER(x) \
    func(#x, self.x);

#define REFLECT_TYPE_PER_MEMBER_PTR(x) \
    func(#x, &This::x);


#define REFLECT_TYPE(Type, ...) \
template <> \
struct reflect_trait<Type> { \
    using This = Type; \
    static constexpr bool has_members() { return true; } \
    template <class Func> \
    static constexpr void for_each_members_ptrs(Func &&func) { \
        REFLECT_PP_FOREACH(REFLECT_TYPE_PER_MEMBER_PTR, __VA_ARGS__) \
    } \
};

#define REFLECT_TYPE_TEMPLATED(Type, ...) \
template <__VA_ARGS__> \
struct reflect_trait<REFLECT_EXPAND(REFLECT_EXPAND Type)> { \
    using This = REFLECT_EXPAND(REFLECT_EXPAND Type); \
    static constexpr bool has_members() { return true; }; \
    template <class Func> \
    static constexpr void for_each_members(This const &cself, Func &&func) { \
        Type &self = const_cast<Type &>(cself); \
        REFLECT_PP_FOREACH(REFLECT_TYPE_PER_MEMBER, __VA_ARGS__) \
    } \
    template <class Func> \
    static constexpr void for_each_members_ptrs(Func &&func) { \
        REFLECT_PP_FOREACH(REFLECT_TYPE_PER_MEMBER_PTR, __VA_ARGS__) \
    } \
};


#define REFLECT_PER_MEMBER(x) \
    func(#x, x);

#define REFLECT_PER_MEMBER_PTR(x) \
    func(#x, &This::x);

#define REFLECT(...) \
template <class Func> \
constexpr void for_each_members(Func &&func) { \
    REFLECT_PP_FOREACH(REFLECT_PER_MEMBER, __VA_ARGS__) \
} \
template <class This, class Func> \
static constexpr void for_each_members_ptrs(Func &&func) { \
    REFLECT_PP_FOREACH(REFLECT_PER_MEMBER_PTR, __VA_ARGS__) \
}

#define REFLECT_FUNC(...) \
    template <class This, class Func> \
    static constexpr void for_each_members_func_ptrs(Func &&func) { \
        REFLECT_PP_FOREACH(REFLECT_PER_MEMBER_PTR, __VA_ARGS__) \
}

std::string toString(Json::Value root) {
    Json::StreamWriterBuilder builder;
    builder["indentation"] = " ";
    std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    std::ostringstream os;
    writer->write(root, &os);
    return os.str();
}

// template <class T>
// std::string serializer(T &obj) {
//     Json::Value root;
//     reflect_trait<T>::for_each_members(obj, [&] (const char *key, auto &value) {
//         root[key] = value;
//     });
//     return root.toStyledString();
// }

Json::Value fromString(std::string const &json) {
    Json::Value root;
    Json::Reader reader;
    reader.parse(json, root);
    return root;
}

template <class T> requires (!reflect_trait<T>::has_members())
Json::Value serializer(T const &obj) {
    return obj;
}

template <class T> requires (reflect_trait<T>::has_members())
Json::Value serializer(T const &obj) {
    Json::Value root;
    reflect_trait<T>::for_each_members(obj, [&] (const char *key, auto &value) {
        root[key] = serializer(value);
    });
    return root;
}

template <class T> requires (!reflect_trait<T>::has_members())
T deserialize(Json::Value const &root) {
    return root.as<T>();
};

template <class T> requires (reflect_trait<T>::has_members())
T deserialize(Json::Value const &root) {
    T obj;
    reflect_trait<T>::for_each_members(obj, [&] (const char *key, auto &value) {
        value = deserialize<std::decay_t<decltype(value)>>(root[key]);
    });
    return obj;
}

struct Components {
    float m_value;
    REFLECT(m_value);
};

struct GObject {
    std::string m_name;
    std::string m_definition_url;
    int m_id;
    Components m_components;
    int getid() {
        return m_id;
    }
    REFLECT(m_name, m_definition_url, m_id, m_components);
    REFLECT_FUNC(getid);
};


// REFLECT_TYPE_TEMPLATED(((GObject<T>), class T), m_name, m_definition_url, m_id, m_components);

int main() {
    GObject m_go = {
        .m_name = "TestObj",
        .m_definition_url = "Testurl",
        .m_id = 1,
        .m_components = {
            .m_value = 1.5f,
        }
    };
    Components test = {
        .m_value = 1.7f,
    };
    auto a = pybind::bind<GObject>();
    std::string bin = toString(serializer(m_go));
    std::string bin2 = toString(serializer(test));
    std::cout << bin << '\n';
    std::cout << bin2 << '\n';
    auto GoDes = deserialize<GObject>(fromString(bin));
    auto ComDes = deserialize<Components>(fromString(bin2));
    std::cout << GoDes.m_name << '\n';
    std::cout << GoDes.m_definition_url << '\n';
    std::cout << GoDes.m_id << '\n';
    std::cout << GoDes.m_components.m_value << '\n';
    std::cout << ComDes.m_value << '\n';
    return 0;
}

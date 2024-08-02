#include <iostream>
#include <string>
#include <json/json.h>

struct GObject {
    std::string m_name;
    int m_id;
};

template <class T>
struct reflect_trait {};

template <>
struct reflect_trait<GObject> {
    template <class Func>
    static constexpr void for_each_members(GObject &go, Func &&func) {
        func("id", go.m_id);
        func("name", go.m_name);
    }
};

template <class T>
std::string serializer(T &obj) {
    Json::Value root;
    reflect_trait<T>::for_each_members(obj, [&] (const char *key, auto &value) {
        root[key] = value;
    });
    return root.toStyledString();
}

int main() {
    GObject m_go = {
        .m_name = "TestObj",
        .m_id = 1,
    };
    std::string bin = serializer(m_go);
    std::cout << bin << '\n';
    return 0;
}

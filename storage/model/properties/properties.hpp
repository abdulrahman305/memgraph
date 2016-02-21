#pragma once

#include <map>

#include "property.hpp"
#include "all.hpp"

class Properties
{
public:
    using sptr = std::shared_ptr<Properties>;

    const Property& at(const std::string& key) const
    {
        auto it = props.find(key);

        if(it == props.end())
            return Property::Null;

        return *it->second.get();
    }

    template <class T, class... Args>
    void set(const std::string& key, Args&&... args)
    {
        auto value = std::make_shared<T>(std::forward<Args>(args)...);

        // try to emplace the item
        auto result = props.emplace(std::make_pair(key, value));

        // return if we succedded
        if(result.second)
            return;

        // the key already exists, replace the value it holds
        result.first->second = std::move(value);
    }

    void set(const std::string& key, Property::sptr value)
    {
        props[key] = std::move(value);
    }

    void clear(const std::string& key)
    {
        props.erase(key);
    }

    template <class Handler>
    void accept(Handler& handler) const
    {
        for(auto& kv : props)
            handler.handle(kv.first, *kv.second);

        handler.finish();
    }

private:
    using props_t = std::map<std::string, Property::sptr>;

    props_t props;
};

template<>
void Properties::set<Null>(const std::string& key)
{
    clear(key);
}


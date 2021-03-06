#pragma once

namespace moon
{
    struct lua_scope_pop
    {
        lua_State* lua;
        lua_scope_pop(lua_State* L)
            :lua(L)
        {
        }

        lua_scope_pop(const lua_scope_pop&) = delete;

        lua_scope_pop& operator=(const lua_scope_pop&) = delete;
       
        ~lua_scope_pop()
        {
            lua_pop(lua, 1);
        }
    };

    template<class LuaArrayView>
    class lua_array_view_iterator
    {
        const LuaArrayView* lav_;
        size_t pos_;
    public:
        using iterator_category = std::random_access_iterator_tag;

        explicit lua_array_view_iterator(const LuaArrayView* lav, size_t pos)
            :lav_(lav)
            , pos_(pos)
        {
        }

        auto operator*() const
        {
            return lav_->get_lua_value(pos_ + 1);
        }

        lua_array_view_iterator& operator++()
        {
            assert(pos_ != lav_->size());
            ++pos_;
            return *this;
        }

        lua_array_view_iterator operator++(int)
        {
            assert(pos_ != lav_->size());
            auto old = pos_;
            pos_++;
            return lua_array_view_iterator{ lav_, old };
        }

        bool operator!=(const lua_array_view_iterator& other) const {
            return pos_ != other.pos_;
        }
    };

    template<typename ValueType>
    class lua_array_view
    {
        int index_;
        lua_State* L_;
        size_t size_ = 0;


        auto get_lua_value(size_t pos) const
        {
            assert(pos <= size());
            lua_rawgeti(L_, index_, pos);
            lua_scope_pop lsp{ L_ };
            if constexpr (std::is_integral_v<value_type>)
            {
                return static_cast<value_type>(luaL_checkinteger(L_, -1));
            }
            else if constexpr (std::is_floating_point_v<value_type>)
            {
                return static_cast<value_type>(luaL_checknumber(L_, -1));
            }
            else
            {
                size_t len;
                const char* data = luaL_checklstring(L_, -1, &len);
                return std::string{ data, len };
            }
        }
    public:
        using value_type = ValueType;

        using const_iterator = lua_array_view_iterator<lua_array_view>;

        friend const_iterator;

        lua_array_view(lua_State* L, int idx)
            :index_(idx)
            , L_(L)
        {
            size_ = lua_rawlen(L, idx);
        }

        size_t size() const
        {
            return size_;
        }

        bool empty() const
        {
            return size_ == 0;
        }

        lua_State* lua_state() const
        {
            return L_;
        }

        value_type operator[](size_t pos) const
        {
            return get_lua_value(pos + 1);
        }

        const_iterator begin() const
        {
            return const_iterator{ this,  0 };
        }

        const_iterator end() const
        {
            return const_iterator{ this,  size() };
        }
    };
}

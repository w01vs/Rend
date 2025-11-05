#ifndef TYPE_HPP
#define TYPE_HPP

#pragma once

#include <cstdlib>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>
namespace type
{
    struct BuiltinType {

        int ID();
        bool operator==(const std::shared_ptr<BuiltinType>& other) const;
        virtual ~BuiltinType() = default;
        bool is_compatible(const std::shared_ptr<BuiltinType>& other) const;

      protected:
        BuiltinType(std::string_view name, int bytes);
        BuiltinType(std::string_view name);
        std::string_view name;
        size_t bytes;
        size_t id;
        inline static int id_count = 0;
        friend class TypeRegistry;
        friend class UDType;
    };

    struct UDType : BuiltinType {
        std::vector<std::string_view> member_names;
        std::map<std::string_view, std::shared_ptr<BuiltinType>, std::less<>> members;
        std::map<std::string_view, int, std::less<>> offsets;

      protected:
        size_t alignment = 0;
        UDType(std::string_view name);
        UDType(
            std::string_view name,
            std::map<std::string_view, std::shared_ptr<BuiltinType>, std::less<>>&& members);
        void calculate_offsets();
        friend class TypeRegistry;
    };

    class TypeRegistry {
      public:
        static TypeRegistry& instance();
        void declare_type(std::string_view name);
        std::shared_ptr<UDType> define_type(
            std::string_view name,
            std::map<std::string_view, std::shared_ptr<BuiltinType>, std::less<>>&& members);
        bool unregister_type(std::shared_ptr<UDType> type);
        bool unregister_type(std::string_view name);
        std::shared_ptr<UDType> find_type(std::string_view name) const;
        bool is_builtin(std::shared_ptr<BuiltinType> type) const;
        TypeRegistry(const TypeRegistry& other) = delete;
        TypeRegistry& operator=(const TypeRegistry& other) = delete;
        std::shared_ptr<BuiltinType> _int_();
        std::shared_ptr<BuiltinType> _bool_();
        std::shared_ptr<BuiltinType> _void_();
        std::shared_ptr<BuiltinType> _undefined_();
        
        private:
        static TypeRegistry* instance_;
        TypeRegistry();
        ~TypeRegistry() = default;
        std::map<std::string_view, std::shared_ptr<UDType>, std::less<>> typenames_;
        std::shared_ptr<BuiltinType> int_;
        std::shared_ptr<BuiltinType> bool_;
        std::shared_ptr<BuiltinType> void_;
        std::shared_ptr<BuiltinType> undefined_;
        static std::mutex mutex_;
        std::shared_ptr<BuiltinType> find_builtin(std::string_view name) const;
    };
} // namespace type

#endif // TYPE_HPP

#include "type.hpp"

int type::BuiltinType::ID()
{
    return id;
}

bool type::BuiltinType::operator==(const std::shared_ptr<BuiltinType>& other) const
{
    return id == other->id;
}

bool type::BuiltinType::is_compatible(const std::shared_ptr<BuiltinType>& other) const
{
    if(this->name == other->name)
        return true;
    if(this->name == "int" && other->name == "bool")
        return true;
    if(this->name == "bool" && other->name == "int")
        return true;
    return false;
}

type::BuiltinType::BuiltinType(std::string_view name, int bytes) : name(name), bytes(bytes)
{
    id = id_count++;
}

type::BuiltinType::BuiltinType(std::string_view name) : name(name)
{
    id = id_count++;
}

type::UDType::UDType(std::string_view) : BuiltinType(name) {}

type::UDType::UDType(
    std::string_view name,
    std::map<std::string_view, std::shared_ptr<BuiltinType>, std::less<>>&& members)
    : members(std::move(members)), BuiltinType(name)
{
    for(const auto& p : this->members)
    {
        if(p.second->bytes <= 0)
        {
            std::cerr << "Error: Member '" << p.first << "' in type '" << name
                      << "' has invalid size." << std::endl;
            exit(EXIT_FAILURE);
        }
    }
    calculate_offsets();
}

void type::UDType::calculate_offsets()
{
    // find largest offset:
    int total_offset = 0;
    for(const auto& [name, type] : members)
    {
        member_names.emplace_back(name);
        const UDType* udtype = dynamic_cast<const UDType*>(type.get());
        if(udtype)
        {

            alignment = udtype->alignment >= alignment ? alignment : udtype->alignment;
            if(total_offset % alignment == 0)
            {
                offsets[name] = total_offset;
            }
            else
            {
                total_offset += alignment - (total_offset % alignment);
                offsets[name] = total_offset;
            }
        }
        else
        {
            alignment = type->bytes >= alignment ? alignment : type->bytes;
            if(total_offset % type->bytes == 0)
            {
                offsets[name] = total_offset;
            }
            else
            {
                total_offset += type->bytes - (total_offset % type->bytes);
                offsets[name] = total_offset;
            }
            total_offset += type->bytes;
        }
    }
    bytes = total_offset;
}

type::TypeRegistry* type::TypeRegistry::instance_ = nullptr;
std::mutex type::TypeRegistry::mutex_;

type::TypeRegistry& type::TypeRegistry::instance()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if(!TypeRegistry::instance_)
    {
        instance_ = new TypeRegistry();
    }
    return *instance_;
}

void type::TypeRegistry::declare_type(std::string_view name)
{
    struct temp : type::UDType {
        temp(std::string_view name) : type::UDType(name) {}
    };
    typenames_[name.data()] = std::make_shared<temp>(name);
}

std::shared_ptr<type::UDType> type::TypeRegistry::define_type(
    std::string_view name,
    std::map<std::string_view, std::shared_ptr<BuiltinType>, std::less<>>&& members)
{
    auto type = find_type(name);
    type->members = std::move(members);
    for(const auto& [name, member] : type->members)
    {
        if(member->bytes <= 0)
        {
            std::cerr << "Error: Member '" << name << "' in type '" << name << "' has invalid size."
                      << std::endl;
            exit(EXIT_FAILURE);
        }
        type->member_names.emplace_back(name);
    }
    type->calculate_offsets();
    return type;
}

bool type::TypeRegistry::unregister_type(const std::shared_ptr<UDType> type)
{
    return unregister_type(type->name);
}

bool type::TypeRegistry::unregister_type(std::string_view name)
{
    if(typenames_.count(name.data()) > 0)
    {
        typenames_.erase(name.data());
        return true;
    }
    return false;
}

std::shared_ptr<type::UDType> type::TypeRegistry::find_type(std::string_view name) const
{
    auto builtin = find_builtin(name);
    if(builtin != undefined_)
        return std::static_pointer_cast<UDType>(builtin);

    if(typenames_.count(name.data()) > 0)
    {
        return typenames_.at(name.data());
    }
    return std::static_pointer_cast<UDType>(undefined_);
}

std::shared_ptr<type::BuiltinType> type::TypeRegistry::find_builtin(std::string_view name) const
{
    if(name == "int")
        return int_;
    else if(name == "bool")
        return bool_;
    else if(name == "void")
        return void_;
    else
        return undefined_;
}

bool type::TypeRegistry::is_builtin(std::shared_ptr<BuiltinType> type) const
{
    return (type == int_ || type == bool_ || type == void_ || type == undefined_);
}

std::shared_ptr<type::BuiltinType> type::TypeRegistry::_int_()
{
    return int_;
}

std::shared_ptr<type::BuiltinType> type::TypeRegistry::_bool_()
{
    return bool_;
}

std::shared_ptr<type::BuiltinType> type::TypeRegistry::_void_()
{
    return void_;
}

std::shared_ptr<type::BuiltinType> type::TypeRegistry::_undefined_()
{
    return undefined_;
}

type::TypeRegistry::TypeRegistry()
{
    struct temp : type::BuiltinType {
        temp(std::string_view name, int bytes) : type::BuiltinType(name, bytes) {}
    };
    std::cout << "size of derived temp:gtype: " << sizeof(temp) << "\n";
    int_ = std::make_shared<temp>("int", 4);
    bool_ = std::make_shared<temp>("bool", 8);
    void_ = std::make_shared<temp>("void", 0);
    undefined_ = std::make_shared<temp>("undefined", 0);
}

#pragma once

#include <example/NameService.hh>

class NameGetterProxy : public NameGetterBase
{
public:
    NameGetterProxy() = default;
    virtual ~NameGetterProxy() = default;
    virtual std::string GetName(GetNameArgs) override;
};

class NameSetterProxy : public NameSetterBase
{
public:
    NameSetterProxy() = default;
    virtual ~NameSetterProxy() = default;
    virtual int SetName(SetNameArgs) override;
};

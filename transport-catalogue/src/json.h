#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {

class Node;

using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node {
public:

    using Value = std::variant<std::nullptr_t,Array,Dict,bool,int,double, std::string>;
    
    template<typename T>
    Node(T value):value_(value){}

    Node() = default;
    const Value& GetValue() const;
    Value& GetValue();
    
    bool IsInt() const;
    bool IsDouble() const;
    bool IsPureDouble() const;
    bool IsBool() const;
    bool IsString() const;
    bool IsNull() const;
    bool IsArray() const;
    bool IsMap() const;
    
    
    int AsInt() const;
    bool AsBool() const;
    double AsDouble() const;
    const std::string& AsString() const;
    const Array& AsArray() const;
    const Dict& AsMap() const;
    
    

    
    bool operator==(const Node& r) const{
        return value_ == r.GetValue();
    }
    bool operator!=(const Node& r) const{
        return !(*this == r);
    }
private:
    Value value_;
};

class Document {
public:
    explicit Document(Node root);

    const Node& GetRoot() const;
    bool operator==(const Document& other){
        return root_==other.GetRoot();
    }
    bool operator!=(const Document& other){
        return !(*this==other);
    }
private:
    Node root_;
};

Document Load(std::istream& input);

void Print(const Document& doc, std::ostream& output);

}  // namespace json

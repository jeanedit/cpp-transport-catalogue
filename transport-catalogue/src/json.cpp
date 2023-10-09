#include "json.h"

using namespace std;

namespace json {

namespace {

Node LoadNode(istream& input);
    
Node LoadNull(std::istream& input){
    std::string null;
    for (char c; input >> c;) {
        if(!isalpha(c)){
            input.putback(c);
            break;
        }
        null.push_back(c);
    }
    if(null != "null"s){
        throw ParsingError("Null parsing error");
    }
    return Node{nullptr};
}
    
Node LoadBool(std::istream& input){
    std::string logic;
    for (char c; input >> c;) {
            if(!isalpha(c)){
                input.putback(c);
                break;
            }
            logic.push_back(c);
    }
    if(logic == "true"s){
        return Node{true};
    }
    if(logic == "false"s){
        return Node{false};
    }
      throw ParsingError("Bool parsing error");
}
    
Node LoadNumber(std::istream& input) {
    using namespace std::literals;
    
    std::string parsed_num;

    // Read symbol to parsed_num from input
    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

    // Read one or more digits to parsed_num from input
    auto read_digits = [&input, read_char] {
        if (!std::isdigit(input.peek())) {
            throw ParsingError("A digit is expected"s);
        }
        while (std::isdigit(input.peek())) {
            read_char();
        }
    };
    
    if (input.peek() == '-') {
        read_char();
    }
    // Parse integer part of a number
    if (input.peek() == '0') {
        read_char();
        // In JSON, digits cannot follow a leading zero.
    } else {
        read_digits();
    }

    bool is_int = true;
    // Parsing the fractional part of the number.
    if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
    }

    // Parsing the exponential part of the number.
    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-') {
            read_char();
        }
        read_digits();
        is_int = false;
    }

    try {
        if (is_int) {
            // First, we try to convert the string to an integer.
            try {
                return Node{std::stoi(parsed_num)};
            } catch (...) {
                // In case of failure, for example, due to overflow, 
                // the code below will attempt to convert the string to a double.
            }
        }
        return Node{std::stod(parsed_num)};
    } catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

// Reads the contents of a JSON document's string literal. 
// This function should be used after reading the opening double quotation mark ":
Node LoadString(std::istream& input) {
    using namespace std::literals;
    
    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    std::string s;
    while (true) {
        if (it == end) {
            // The stream ended before encountering the closing double quotation mark?
            throw ParsingError("String parsing error");
        }
        const char ch = *it;
        if (ch == '"') {
            // We encountered the closing double quotation mark.
            ++it;
            break;
        } else if (ch == '\\') {
            // We encountered the beginning of an escape sequence.
            ++it;
            if (it == end) {
                // The stream ended immediately after the backslash character.
                throw ParsingError("String parsing error");
            }
            const char escaped_char = *(it);
            // We are processing one of the sequences: \, \n, \t, \r, ".
            switch (escaped_char) {
                case 'n':
                    s.push_back('\n');
                    break;
                case 't':
                    s.push_back('\t');
                    break;
                case 'r':
                    s.push_back('\r');
                    break;
                case '"':
                    s.push_back('"');
                    break;
                case '\\':
                    s.push_back('\\');
                    break;
                default:
                    // We encountered an unknown escape sequence.
                    throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
            }
        } else if (ch == '\n' || ch == '\r') {
            // A string literal inside JSON cannot be interrupted by the characters \r or \n.
            throw ParsingError("Unexpected end of line"s);
        } else {
            // Simply read the next character and append it to the resulting string.
            s.push_back(ch);
        }
        ++it;
    }

    return Node{std::move(s)};
}
Node LoadArray(istream& input) {
    Array result;
    char c;
    for (;input>>c && c!=']';) {
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }
    
    if((c!=']' && result.empty())){
        throw ParsingError("Array parsing "s+c+" error"s);
    }
    return Node(move(result));
}


Node LoadDict(istream& input) {
    Dict result;
    char c;
    for (; input >> c && c != '}';) {
        if (c == ',') {
            input >> c;
        }
        string key = LoadString(input).AsString();
        input >> c;
        if(c!=':'){
            throw ParsingError("Dict parsing error: "+c+" absence of : between key and value"s);
        }
        result.insert({move(key), LoadNode(input)});
    }
    if(c!='}'){
        throw ParsingError("Dict parsing error: absence of }"s);
    }
    return Node(move(result));
}

Node LoadNode(istream& input) {
    char c;
    input >> c;

    if(c == 'n'){
        input.putback(c);
        return LoadNull(input);
    }
    else if(c == 't' || c == 'f'){
        input.putback(c);
        return LoadBool(input);
    }
    else if (c == '[') {
        return LoadArray(input);
    } else if (c == '{') {
        return LoadDict(input);
    } else if (c == '"') {
        return LoadString(input);
    } else {
        input.putback(c);
        return LoadNumber(input);
    }
}

}  // namespace

const Node::Value& Node::GetValue() const{
    return value_;
}   
    
Node::Value& Node::GetValue(){
    return value_;
}

bool Node::IsInt() const{
    return std::holds_alternative<int>(value_);
}
bool Node::IsDouble() const{
    return std::holds_alternative<double>(value_) ||    std::holds_alternative<int>(value_);
}
bool Node::IsPureDouble() const{
    return std::holds_alternative<double>(value_);
}
bool Node::IsBool() const{
    return std::holds_alternative<bool>(value_);
}
    
bool Node::IsString() const{
    return std::holds_alternative<std::string>(value_);
}
bool Node::IsNull() const{
    return std::holds_alternative<std::nullptr_t>(value_);
}
bool Node::IsArray() const{
    return std::holds_alternative<Array>(value_);
}
bool Node::IsMap() const{
    return std::holds_alternative<Dict>(value_);
}


int Node::AsInt() const{
    if (const int* val = get_if<int>(&value_)) {
        int result = *val;
        return result;
    }
    throw std::logic_error("wrong type");
}
bool Node::AsBool() const{
    if (const bool* val = get_if<bool>(&value_)) {
        bool result = *val;
        return result;
    } 
    throw std::logic_error("wrong type");
}
double Node::AsDouble() const{
    if (const int* val = get_if<int>(&value_)) {
       double result = *val;
       return result;
    }
   if (const double* val = get_if<double>(&value_)) {
       double result = *val;
       return result;
    }
    throw std::logic_error("wrong type");
}
const std::string& Node::AsString() const{
    if (const std::string* val = get_if<std::string>(&value_)) {
       return *val;
    }
    throw std::logic_error("wrong type");
}
const Array& Node::AsArray() const{
    if (const auto* val = get_if<Array>(&value_)) {
       return *val;
    }
    throw std::logic_error("wrong type");
}
const Dict& Node::AsMap() const{
    if (const auto* val = get_if<Dict>(&value_)) {
       return *val;
    }
    throw std::logic_error("wrong type");
} 

    
Document::Document(Node root)
    : root_(move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

    
struct PrintContext {
    std::ostream& out;
    int indent_step = 4;
    int indent = 0;

    void PrintIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    PrintContext Indented() const {
        return {out, indent_step, indent_step + indent};
    }
};

void PrintNode(const Node& value, const PrintContext& ctx);

template <typename Value>
void PrintValue(const Value& value, const PrintContext& ctx) {
    ctx.out << value;
}

void PrintString(const std::string& value, std::ostream& out) {
    out.put('"');
    for (const char c : value) {
        switch (c) {
            case '\r':
                out << "\\r"sv;
                break;
            case '\n':
                out << "\\n"sv;
                break;
            case '"':
                // Символы " и \ выводятся как \" или \\, соответственно
                [[fallthrough]];
            case '\\':
                out.put('\\');
                [[fallthrough]];
            default:
                out.put(c);
                break;
        }
    }
    out.put('"');
}

template <>
void PrintValue<std::string>(const std::string& value, const PrintContext& ctx) {
    PrintString(value, ctx.out);
}

template <>
void PrintValue<std::nullptr_t>(const std::nullptr_t&, const PrintContext& ctx) {
    ctx.out << "null"sv;
}

// In the template specialization for the type bool, 
// the parameter value is passed by constant reference, just like in the main template. 
// Alternatively, you can use an overload: 
// void PrintValue(bool value, const PrintContext& ctx);.
template <>
void PrintValue<bool>(const bool& value, const PrintContext& ctx) {
    ctx.out << (value ? "true"sv : "false"sv);
}

template <>
void PrintValue<Array>(const Array& nodes, const PrintContext& ctx) {
    std::ostream& out = ctx.out;
    out << "[\n"sv;
    bool first = true;
    auto inner_ctx = ctx.Indented();
    for (const Node& node : nodes) {
        if (first) {
            first = false;
        } else {
            out << ",\n"sv;
        }
        inner_ctx.PrintIndent();
        PrintNode(node, inner_ctx);
    }
    out.put('\n');
    ctx.PrintIndent();
    out.put(']');
}

template <>
void PrintValue<Dict>(const Dict& nodes, const PrintContext& ctx) {
    std::ostream& out = ctx.out;
    out << "{\n"sv;
    bool first = true;
    auto inner_ctx = ctx.Indented();
    for (const auto& [key, node] : nodes) {
        if (first) {
            first = false;
        } else {
            out << ",\n"sv;
        }
        inner_ctx.PrintIndent();
        PrintString(key, ctx.out);
        out << ": "sv;
        PrintNode(node, inner_ctx);
    }
    out.put('\n');
    ctx.PrintIndent();
    out.put('}');
}

void PrintNode(const Node& node, const PrintContext& ctx) {
    std::visit(
        [&ctx](const auto& value) {
            PrintValue(value, ctx);
        },
        node.GetValue());
}

Document Load(std::istream& input) {
    return Document{LoadNode(input)};
}

void Print(const Document& doc, std::ostream& output) {
    PrintNode(doc.GetRoot(), PrintContext{output});
}

}  // namespace json

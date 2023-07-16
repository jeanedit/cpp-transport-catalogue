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

    // Считывает в parsed_num очередной символ из input
    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

    // Считывает одну или более цифр в parsed_num из input
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
    // Парсим целую часть числа
    if (input.peek() == '0') {
        read_char();
        // После 0 в JSON не могут идти другие цифры
    } else {
        read_digits();
    }

    bool is_int = true;
    // Парсим дробную часть числа
    if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
    }

    // Парсим экспоненциальную часть числа
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
            // Сначала пробуем преобразовать строку в int
            try {
                return Node{std::stoi(parsed_num)};
            } catch (...) {
                // В случае неудачи, например, при переполнении,
                // код ниже попробует преобразовать строку в double
            }
        }
        return Node{std::stod(parsed_num)};
    } catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

// Считывает содержимое строкового литерала JSON-документа
// Функцию следует использовать после считывания открывающего символа ":
Node LoadString(std::istream& input) {
    using namespace std::literals;
    
    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    std::string s;
    while (true) {
        if (it == end) {
            // Поток закончился до того, как встретили закрывающую кавычку?
            throw ParsingError("String parsing error");
        }
        const char ch = *it;
        if (ch == '"') {
            // Встретили закрывающую кавычку
            ++it;
            break;
        } else if (ch == '\\') {
            // Встретили начало escape-последовательности
            ++it;
            if (it == end) {
                // Поток завершился сразу после символа обратной косой черты
                throw ParsingError("String parsing error");
            }
            const char escaped_char = *(it);
            // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
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
                    // Встретили неизвестную escape-последовательность
                    throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
            }
        } else if (ch == '\n' || ch == '\r') {
            // Строковый литерал внутри- JSON не может прерываться символами \r или \n
            throw ParsingError("Unexpected end of line"s);
        } else {
            // Просто считываем очередной символ и помещаем его в результирующую строку
            s.push_back(ch);
        }
        ++it;
    }

    return Node{std::move(s)}
    ;
}
Node LoadArray(istream& input) {
    Array result;
    char c;
    for (;input>>c && c!=']';) {
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    //if(input >> c){
    //    if(c == ']'){
    //        return Node(move(result));
    //    }
    //    if(c != ','){
    //        throw ParsingError("absence of comma"s);
    //    }
    //}else{
    //    throw ParsingError("Array parsing error");
    //}
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

Document Load(istream& input) {
    return Document{LoadNode(input)};
}
    
// Шаблон, подходящий для вывода double и int
template <typename Value>
void PrintValue(const Value& value, std::ostream& out) {
    out << value;
}

// Перегрузка функции PrintValue для вывода значений null
void PrintValue(std::nullptr_t, std::ostream& out) {
    out << "null"sv;
}

void PrintValue(bool val, std::ostream& out) {
    val ? out << "true"sv : out << "false"sv;
}

void PrintValue(const std::string& str, std::ostream& out) {
    out<<'"';
    for(const char c: str){
        switch (c) {
        case '\n':
            out<<"\\n"s;
            break;
        case '\t':
            out<<'\t';
            break;
        case '\r':
           out<<"\\r"s;
            break;
        case '\\':
            out<<"\\\\"s;
            break;
        case '"':
            out<<"\\\""s;
            break;
        default:
            out<<c;
            break;
         }
    }
    out<<'"';
}
void PrintNode(const Node& node, std::ostream& out);
    
// Другие перегрузки функции PrintValue пишутся аналогично
    
void PrintValue(const Array& arr,std::ostream& out){
    int count = 0;
    out<<'[';
    for(const auto& a:arr){
        if(count++ != 0){
             out<<',';
        }
        
       PrintNode(a,out);  
    }
    out<<']';
}
    
void PrintValue(const Dict& dict,std::ostream& out){
    out<<'{';
    size_t count = 0;
    for(const auto& [key,val]:dict){
        PrintNode(key,out);
        out<<':';
        PrintNode(val,out);
        ++count;
        if(count != dict.size()){
            out<<',';  
        }

    }
    out<<'}';
}
void PrintNode(const Node& node, std::ostream& out) {
    std::visit(
        [&out](const auto& value){ PrintValue(value, out); },
        node.GetValue());
} 
    

void Print(const Document& doc, std::ostream& output) {
    PrintNode(doc.GetRoot(),output);

}

}  // namespace json
#include "json_builder.h"

namespace json {

    Builder::KeyItemContext Builder::BaseContext::Key(std::string key) {
        return builder_.Key(std::move(key));
    }
    Builder::DictItemContext Builder::BaseContext::StartDict() {
        return builder_.StartDict();
    }
    Builder::ArrayItemContext Builder::BaseContext::StartArray() {
        return builder_.StartArray();
    }
    Builder& Builder::BaseContext::EndDict() {
        return builder_.EndDict();
    }

    Builder& Builder::BaseContext::EndArray() {
        return builder_.EndArray();
    }
    Builder& Builder::BaseContext::Value(Node::Value val) {
        return builder_.Value(std::move(val));
    }
    Builder::DictItemContext Builder::KeyItemContext::Value(Node::Value val) {
        return BaseContext::Value(std::move(val));
    }
    Builder::ArrayItemContext Builder::ArrayItemContext::Value(Node::Value val) {
        return BaseContext::Value(std::move(val));
    }

    Builder::KeyItemContext Builder::Key(std::string key) {
        if (nodes_stack_.empty() || !nodes_stack_.back()->IsMap()) {
            throw std::logic_error("Calling method Key() with non-dictionary type");
        }
        auto [it, flag] = std::get<Dict>(nodes_stack_.back()->GetValue()).emplace(std::move(key), Node(nullptr));
        nodes_stack_.push_back(&it->second);
        return Builder::KeyItemContext(*this);
    }

    Builder& Builder::Value(Node::Value val) {
        if (root_.IsNull()) {
            root_.GetValue() = std::move(val);
        }
        else if (!nodes_stack_.empty() && nodes_stack_.back()->IsNull()) {
            nodes_stack_.back()->GetValue() = std::move(val);
            Node* val_ptr = nodes_stack_.back();
            nodes_stack_.pop_back();
            AddNode(val_ptr);
        }
        else if (!nodes_stack_.empty() && nodes_stack_.back()->IsArray()) {
            std::visit([this](auto&& value) {std::get<Array>(nodes_stack_.back()->GetValue()).emplace_back(Node(value)); }, val);

            if (!std::holds_alternative<Array>(val) && !std::holds_alternative<Dict>(val)) {
                AddNode(&std::get<Array>(nodes_stack_.back()->GetValue()).back());
            }
        }
        else {
            throw std::logic_error("Calling method Value() after inappropriate node");
        }
        return *this;
    }

    Builder::DictItemContext Builder::StartDict() {
        if (StartContainer(Dict())) {
            return DictItemContext(*this);
        }
        else {
            throw std::logic_error("Calling method StartDict() after inappropriate type");
        }
    }

    Builder::ArrayItemContext Builder::StartArray() {
        if (StartContainer(Array())) {
            return ArrayItemContext(*this);
        }
        else {
            throw std::logic_error("Calling method StartArray() after inappropriate type");
        }

        return ArrayItemContext(*this);
    }

    Builder& Builder::EndDict() {
        if (!nodes_stack_.empty() && nodes_stack_.back()->IsMap()) {
            nodes_stack_.pop_back();
        }
        else {
            throw std::logic_error("Calling method EndDict() after non-dictionary type");
        }
        return *this;
    }

    Builder& Builder::EndArray() {
        if (!nodes_stack_.empty() && nodes_stack_.back()->IsArray()) {
            nodes_stack_.pop_back();
        }
        else {
            throw std::logic_error("Calling method EndArray() after non-dictionary type");
        }
        return *this;
    }

    Node Builder::Build() {
        if (!nodes_stack_.empty() || root_.IsNull()) {
            throw std::logic_error("Calling method Build() before JSON completeness");
        }
        return root_;
    }

    void Builder::AddNode(Node* node) {
        if (node->IsArray() || node->IsMap()) {
            nodes_stack_.push_back(node);
        }
    }
}
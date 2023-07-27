#include "json.h"

namespace json{
    class BaseContext;
    class KeyItemContext;
    class ValueKeyContext;
    class DictItemContext;
    class ArrayItemContext;

    class Builder{
        public:
          Builder():root_(nullptr){}
        
          KeyItemContext Key(std::string key);
          Builder& Value(Node::Value val);
          DictItemContext StartDict();
          ArrayItemContext StartArray();
          Builder& EndDict();
          Builder& EndArray();
        
          Node Build();
        private:    
            template<typename Container> //Container: Dict or Array
            bool StartContainer(Container cont){
              if(nodes_stack_.empty()){
                  root_ = cont;
                  nodes_stack_.push_back(&root_);
              }
              else if (nodes_stack_.back()->IsArray()){
                std::get<Array>(nodes_stack_.back()->GetValue()).emplace_back(Node(cont));
                nodes_stack_.push_back(&std::get<Array>(nodes_stack_.back()->GetValue()).back());
              } 
              else if(nodes_stack_.back()->IsNull()){
                    *nodes_stack_.back() = Node(cont);
                    Node* val_ptr = nodes_stack_.back();
                    nodes_stack_.pop_back();
                    AddNode(val_ptr);
              } 
              else{
                  return false;
              }
              return true;
            }
            void AddNode(Node* node);    

            Node root_;
            std::vector<Node*> nodes_stack_;
    };
    
    class BaseContext{
       public:
          BaseContext(Builder& builder):builder_(builder){}
        
          KeyItemContext Key(std::string key);
          Builder& Value(Node::Value val);
          DictItemContext StartDict();
          ArrayItemContext StartArray();
          Builder& EndDict();
          Builder& EndArray();

        private:                
            Builder& builder_;
    };
      

    class KeyItemContext:public BaseContext{
        public:
          KeyItemContext(Builder& builder):BaseContext(builder){}
        
          KeyItemContext Key(std::string key) = delete;
          ValueKeyContext Value(Node::Value val);
          Builder& EndDict() = delete;
          Builder& EndArray() = delete;
        
    };
    
    class ValueKeyContext:public BaseContext{
        public:
          ValueKeyContext(Builder& builder):BaseContext(builder){}
        
          Builder& Value(Node::Value val) = delete;
          ArrayItemContext StartArray() = delete;
          DictItemContext StartDict() = delete;
          Builder& EndArray() = delete;
        
    };
    
    
    class DictItemContext:public BaseContext{
        public:
          DictItemContext(Builder& builder):BaseContext(builder){}
        
          Builder& Value(Node::Value val) = delete;
          DictItemContext StartDict() = delete;
          ArrayItemContext StartArray() = delete;
          Builder& EndArray() = delete;
        
    };
    
  
    class ArrayItemContext:public BaseContext{
        public:
          ArrayItemContext(Builder& builder):BaseContext(builder){}
        
          KeyItemContext Key(std::string key) = delete;
          ArrayItemContext Value(Node::Value val);
          Builder& EndDict() = delete;
        
    };

}
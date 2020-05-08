//
//  Type.hpp
//  DulCompiler
//
//  Created by Дмитрий Маслюков on 18.04.2020.
//  Copyright © 2020 Дмитрий Маслюков. All rights reserved.
//

#ifndef Type_hpp
#define Type_hpp

#include <stdio.h>
#include <map>
#include <string.h>
#include <stdlib.h>
#include <vector>

class Type{
public:
    char * name;
    size_t size;
    bool isValueType;
    virtual bool operator==(Type&rhs){
        bool is_eq = !strcmp(name, rhs.name);
        return is_eq;
    }
    
    Type(const char * _name, size_t _size = 0){
        name = strdup(_name);
        size = _size;
    }
    
    bool isNumeric()const{
        return strcmp(name, "Int") == 0 || strcmp(name, "Float") == 0;
    }
    
    
    ~Type(){
        free(name);
    }
    virtual Type* operator[](int){return nullptr;}
};


class IterableType: public Type{
public:
    IterableType(const char * name, size_t s=0):Type(name, s){};
    bool operator==(IterableType& rhs)  {
        int idx = 0;
        while( operator[](idx) && rhs[idx] ){
            idx ++;
        }
        if( operator[](idx) || rhs[idx] ){
            return false;
        } else return true;
    }
};

class LayoutType: public IterableType{
    struct entry{
        const char * name;
        Type * t;
    };
    std::vector<entry> layout;
public:
    
    
    LayoutType(const char * name, size_t s = 0):IterableType(name, s){};
    Type* operator[](const char *name){
        for(const auto& e: layout){
            if(strcmp(name, e.name) == 0){
                return e.t;
            }
        }
        return nullptr;
    }
    Type* operator[](int idx) override{
        if(idx >= layout.size() or idx < 0){
            return nullptr;
        }
        return layout[idx].t;
    }
    
    
    
    virtual void addType(const char * n, Type * t){
        if(t){
            const char * name_copy = strdup(n);
            layout.push_back(entry{name_copy, t});
            size += t->size;
        }
    }
    static LayoutType * createNamespace();
    static LayoutType * createMap(Type * key, Type * val);
    static LayoutType * createOption(Type * optional);
    static LayoutType * createFunctionalType(Type * ret, Type * args);
    static LayoutType * createTuple(std::vector<Type*>);
};

class TupleType: public IterableType{
    std::vector<Type*> layout;
public:
    Type* operator[](int idx) override {
        if(idx >= layout.size() or idx < 0){
            return nullptr;
        }
        return layout[idx];
    }
    
    void addType(Type * t){
        if(t){
            layout.push_back(t);
            size += t->size;
        }
    }
    
    
    
};

extern Type IntType;
extern Type FloatType;
extern Type BoolType;
extern Type VoidType;
extern Type StringType;

bool isPrimitive(Type*);

#endif /* Type_hpp */

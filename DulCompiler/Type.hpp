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

extern Type * undefined;

class LayoutType: public IterableType{
public:
    struct entry{
        const char * name;
        Type * t;
    };
private:
    std::vector<entry> layout;
public:
    
    int length()const{
        return (int)layout.size();
    }
    
    LayoutType(const char * name, size_t s = 0):IterableType(name, s){};
    Type*& operator[](const char *name){
        for(const auto& e: layout){
            if(strcmp(name, e.name) == 0){
                return const_cast<Type*&>(e.t);
            }
        }
        return undefined;
    }
    Type* operator[](int idx) override{
        if(idx >= layout.size() or idx < 0){
            return nullptr;
        }
        return layout[idx].t;
    }
    
    entry getEntry(int idx){
        return layout[idx];
    }
    
    int indexOf(const char * name){
        int curridx = 0;
        for(const auto& e: layout){
            if(strcmp(name, e.name) == 0){
                return curridx;
            }
            curridx += e.t->size;
        }
        return -1;
    }
    
    LayoutType(const LayoutType & rhs):IterableType(rhs.name, rhs.size){
        
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
extern Type ObjectType;

bool isPrimitive(Type*);

#endif /* Type_hpp */

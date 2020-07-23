//
//  Type.cpp
//  DulCompiler
//
//  Created by Дмитрий Маслюков on 18.04.2020.
//  Copyright © 2020 Дмитрий Маслюков. All rights reserved.
//

#include "Type.hpp"
#include <string>
Type IntType("Int", 8);
Type FloatType("Float", 8);
Type BoolType("Bool", 8);
Type VoidType("Void", 8);
Type StringType("String", 8);

LayoutType * LayoutType::createNamespace(){
    LayoutType * layout = new LayoutType("namespace");
    for(auto basic_type: {&IntType, &FloatType, &BoolType, &StringType, &VoidType}){
        LayoutType * constructor = new LayoutType((std::string("fun()->")+basic_type->name).c_str());
        constructor->addType("return", basic_type);
        constructor->addType("args", &VoidType);
        layout->addType(basic_type->name, constructor);
    }
    return layout;
}

LayoutType * LayoutType::createMap(Type *key, Type *val){
    std::string typerepr = "map ";
    typerepr += key->name;
    typerepr += "->";
    typerepr += val->name;
    LayoutType * layout = new LayoutType(typerepr.c_str());
    layout->addType("key", key);
    /*if(!isPrimitive(val))
        layout->addType("value", createOption(val));
    else*/
        layout->addType("value", (val));
    return layout;
}

bool isPrimitive(Type * t){
    return t == &FloatType || t == &IntType || t == &BoolType || t == &VoidType;
}

LayoutType * LayoutType::createOption(Type *optional){
    std::string typerepr = optional->name;
    typerepr += "?";
    LayoutType * layout = new LayoutType(typerepr.c_str());
    layout->addType("val", optional);
    return layout;
}
LayoutType * LayoutType::createTuple(std::vector<Type*> v){
    
    std::string typerepr = "Tuple<";
    LayoutType * tuple = new LayoutType("Tuple<");
    for(int i = 0; i < v.size(); i++){
        Type * t = v[i];
        typerepr += t->name;
        tuple->addType(std::to_string(i).c_str(), t);
        typerepr += ",";
    }
    typerepr[typerepr.size() - 1] = '>';
    tuple->name = strdup(typerepr.c_str());
   
    return tuple;
}

LayoutType* LayoutType::createFunctionalType(Type *ret, Type *args){
    LayoutType * func = new LayoutType(("fun(" + std::string(args->name) + ")->" + ret->name).c_str());
    func->addType("return", ret);
    func->addType("args", args);
    func->size = 8;
    return func;
}

Type ObjectType("object", 8);
Type * undefined = 0;

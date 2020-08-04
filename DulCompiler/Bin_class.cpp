//
//  Bin_class.cpp
//  DulCompiler
//
//  Created by Дмитрий Маслюков on 28.07.2020.
//  Copyright © 2020 Дмитрий Маслюков. All rights reserved.
//

#include "Bin_class.hpp"

namespace bin_classes{
void init_bin_classes(){
}

Type * StringType;
vt_object str_vt;

struct __bin_str{
    vt_object vt;
    char * mem;
    int memsize, memcap;
    
    __bin_str(){
        vt = str_vt;
        mem = (char*)malloc(1024);
        mem[0] = 0;
        memsize = 0;
        memcap = 1024;
    }
    
};

void* create_str(void*){
    return new __bin_str;
}

int64_t getStrSize(__bin_str * s){
    return s->memsize;
}

void init_string_type(){
    StringType = new LayoutType("String");
    vtable_init_list str_methods = {
        bin_f_decl{
            "length",
            LayoutType::createFunctionalType(&IntType, &VoidType),
            (something_callable)&getStrSize
        }
    };
    //str_vt = create_VT(str_methods);
    
}
};



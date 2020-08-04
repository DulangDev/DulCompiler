//
//  Bin_class.hpp
//  DulCompiler
//
//  Created by Дмитрий Маслюков on 28.07.2020.
//  Copyright © 2020 Дмитрий Маслюков. All rights reserved.
//

#ifndef Bin_class_hpp
#define Bin_class_hpp

#include <stdio.h>
#include "Type.hpp"

void init_bin_classes(void);
#warning TODO: Correct further typechecks
namespace bin_classes{
extern Type * StringType;
Type * construct_map_type(Type*, Type*);
typedef void*(*something_callable)(void*);
struct bin_f_decl{
    const char * name;
    LayoutType * func_type;
    something_callable f;
};
using vtable_init_list = std::initializer_list<bin_f_decl>;
struct vt_entry {
    // encapsulates method that will be called and some environment features
    // if some data is required more space is allocated and passed to method somehow (via supercaller ) but the ptr is to be typeof vt_entry
    something_callable method;
};

typedef vt_entry** vt_object;

vt_object create_VT(vtable_init_list);
void init_string_type(void);
};


// etc definitions

#endif /* Bin_class_hpp */

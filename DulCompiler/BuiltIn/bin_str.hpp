//
//  bin_str.hpp
//  DulCompiler
//
//  Created by Дмитрий Маслюков on 20.05.2020.
//  Copyright © 2020 Дмитрий Маслюков. All rights reserved.
//

#ifndef bin_str_hpp
#define bin_str_hpp

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct b_str{
    int refcnt;
    int len;
    int cap;
    char * data;
    
    static b_str * alloc_string(const char * data = ""){
        b_str * s = new b_str();
        s->cap = (int)strlen(data) + 1;
        s->data = strdup(data);
        s->len = (int)strlen(data);
        return s;
    }
    
};

#endif /* bin_str_hpp */

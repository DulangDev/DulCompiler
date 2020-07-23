//
//  object.hpp
//  DulCompiler
//
//  Created by Дмитрий Маслюков on 04.07.2020.
//  Copyright © 2020 Дмитрий Маслюков. All rights reserved.
//

#ifndef object_hpp
#define object_hpp

#include <stdio.h>
#include <stdlib.h>
#include "../Type.hpp"
class object{
protected:
    Type * type;
    int refcount;

    
};

class number: public object{
    double val;
public:
    number * add(number * rhs){
        return new number(val + rhs->val);
    }
    number(double v){
        type = &FloatType;
        refcount = 0;
        this->val = v;
    }
    number(int64_t val){
        type = &FloatType;
        refcount = 0;
        this->val = val;
    }
    
};


#endif /* object_hpp */

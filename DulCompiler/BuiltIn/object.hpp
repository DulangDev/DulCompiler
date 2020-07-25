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




#endif /* object_hpp */

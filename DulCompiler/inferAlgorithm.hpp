//
//  inferAlgorithm.hpp
//  DulCompiler
//
//  Created by Дмитрий Маслюков on 29.07.2020.
//  Copyright © 2020 Дмитрий Маслюков. All rights reserved.
//

#ifndef inferAlgorithm_hpp
#define inferAlgorithm_hpp

#include <stdio.h>
#include "AST.hpp"
#include "Type.hpp"

void smartTypeInfer(AstNode * root);
Type * closureSearchAdd(AstNode * function_not_found, const char * name);

#endif /* inferAlgorithm_hpp */

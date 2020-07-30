//
//  inferAlgorithm.cpp
//  DulCompiler
//
//  Created by Дмитрий Маслюков on 29.07.2020.
//  Copyright © 2020 Дмитрий Маслюков. All rights reserved.
//

#include "inferAlgorithm.hpp"
#include <vector>
void smartTypeInfer(AstNode * root){
    
}

Type * closureSearchAdd(AstNode * function_not_found, const char * name){
    std::vector<AstNode*> funcdef_stack;
    AstNode * treewalker = function_not_found;
    Type* found = nullptr;
    while(not found && treewalker){
        funcdef_stack.push_back(treewalker);
        treewalker = treewalker->getFunctionalParent();
        found = (*(*treewalker).namescope)[name];
    }
    if(not treewalker or not found){
        return NULL;
    }
    // found a type and treewalker is valid
    for(auto func_clos_to_add:funcdef_stack){
        
    }
    return NULL;
}

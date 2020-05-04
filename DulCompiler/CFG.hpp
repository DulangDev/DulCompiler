//
//  CFG.hpp
//  DulCompiler
//
//  Created by Дмитрий Маслюков on 19.04.2020.
//  Copyright © 2020 Дмитрий Маслюков. All rights reserved.
//

#ifndef CFG_hpp
#define CFG_hpp

#include <stdio.h>
#include "AST.hpp"
#include "Type.hpp"


class DataFlowNode{
    Type * type;
    AstNode::type dataT; //either name or literal
    Lexem::val_t value;
    int lineno, linepos;
    DataFlowNode * next;
};

class ControlFlowNode{
    int lineno, linepos;
    std::vector<DataFlowNode> leafs;
    AstNode::type op_type;
    Type * val_type;
    ControlFlowNode * preferrable;
    ControlFlowNode * if_false;
};

class ContextedAST{
    AstNode * root;
    LayoutType * variables;
public:
    ControlFlowNode * convertToCFG();
};

#endif /* CFG_hpp */

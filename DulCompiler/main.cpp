//
//  main.cpp
//  DulCompiler
//
//  Created by Дмитрий Маслюков on 18.04.2020.
//  Copyright © 2020 Дмитрий Маслюков. All rights reserved.
//

#include <iostream>
#include <fstream>
#include "AST.hpp"
#include "IR.hpp"
#include "ASM.hpp"

int main(int argc, const char * argv[]) {
    // insert code here...
    AstNode * p = AstNode::parseFile("example.dul");
    LayoutType * namescope =LayoutType::createNamespace();
    p->setNameScope(namescope, namescope);
    setFunctionParenthesisDownwalk(p->children[0]);
    p->inferTypes();
    //p->removeRedundant();
    std::ofstream a("main.ast");
    p->print(0, a);
    IRFunction f(p->children[0]->children[3]);
    f.print("main.ir");
    ASMWriter writer;
    writer.writeIRFunction(&f);
    auto compiled = writer.generate();
  
   
    
    void ** stack = (void**)malloc(1024);
    compiled(stack, (void**)f.vals.data());
    
    return 0;
}

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


void processGlobal(AstNode * root){
    // in this node only funcdefs and classdefs are allowed (interfaces ?)
    // empty btw will also appear, ignore them
    LayoutType * global = LayoutType::createNamespace();
    for(auto c:root->children){
        c->setNameScope(global, global);
    }
    root->inferTypes();
}

int main(int argc, const char * argv[]) {
    // insert code here...
    AstNode * p = AstNode::parseFile("example.dul");
    LayoutType * global = LayoutType::createNamespace();
    p->children[0]->setNameScope(global, global);
    
   
    setFunctionParenthesisDownwalk(p);
    p->children[0]->inferTypes();
    //p->removeRedundant();
    std::ofstream a("main.ast");
    p->print(0, a);
    IRFunction f(p->children[0]->children[3]);
    f.print("main.ir");
    ASMWriter writer;
    writer.writeIRFunction(&f);
    auto compiled = writer.generate();
  
   
    
    void ** stack = (void**)malloc(1024);
    compiled(stack, (void**)f.vals.data(), 0);
    
    return 0;
}
